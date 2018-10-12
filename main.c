
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED        /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/keyboard.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/test_stubs.h"

/*********************
 *      DEFINES
 *********************/

/*On OSX SDL needs different handling*/
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
# if __APPLE__ && TARGET_OS_MAC
#define SDL_APPLE
# endif
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void * data);
static void memory_monitor(void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_indev_t *emulated_kp_indev;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char ** argv)
{
    (void) argc;    /*Unused*/
    (void) argv;    /*Unused*/

    /*Initialize LittlevGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LittlevGL*/
    hal_init();

    /* Initialize Hardware Monitors */
    hardware_monitors_init();

    /* Start the PC_SIMULATOR equivalent hardware monitor task */
    lv_task_create(&hardware_monitors_update, 1200, LV_TASK_PRIO_LOW, NULL);

    /*Load Jolt GUI*/
    jolt_gui_create(emulated_kp_indev);

    /*Try the benchmark to see how fast your GUI is*/
//    benchmark_create();

    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();

        usleep(10000);       /*Just to let the system breath*/

        #ifdef SDL_APPLE
            SDL_Event event;
            
            while(SDL_PollEvent(&event)) {
                #if USE_MOUSE != 0
                    mouse_handler(&event);
                #endif

                #if USE_KEYBOARD
                    keyboard_handler(&event);
                #endif

                #if USE_ENCODER != 0
                    encoder_handler(&event);
                #endif
            }
        #endif

    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
static void hal_init(void)
{
    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = monitor_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    lv_disp_drv_register(&disp_drv);

    /* Add the keyboard */
    keyboard_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read = keyboard_read;
    emulated_kp_indev = lv_indev_drv_register(&indev_drv);

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /* Optional:
     * Create a memory monitor task which prints the memory usage in every second.*/
    lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_LOWEST, NULL);
}

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void * data)
{
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(void * param)
{
    (void) param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size);
}
