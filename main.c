
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
#include <emscripten.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/sdl/sdl.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"

#include "src/ui/ui.h"

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
static void memory_monitor(lv_timer_t * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_t  * disp1;

int monitor_hor_res, monitor_ver_res;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void do_loop(void *arg);

int main(int argc, char ** argv)
{
    monitor_hor_res = atoi(argv[1]);
    monitor_ver_res = atoi(argv[2]);
    printf("Starting with screen resolution of %dx%d px\n", monitor_hor_res, monitor_ver_res);

    /*Initialize LittlevGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LittlevGL*/
    hal_init();

    /*Init*/
    ui_init();

    emscripten_set_main_loop_arg(do_loop, NULL, -1, true);
}

void do_loop(void *arg)
{
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_task_handler();

    ui_tick();

    SDL_Event event;
    
    while(SDL_PollEvent(&event)) {
        #if USE_MOUSE != 0
            mouse_handler(&event);
        #endif

        #if USE_KEYBOARD
            keyboard_handler(&event);
        #endif

        #if USE_MOUSEWHEEL != 0
            mousewheel_handler(&event);
        #endif
    }

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
static void hal_init(void)
{
   /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    sdl_init();

    /*Create a display buffer*/
    static lv_disp_draw_buf_t disp_buf1;
    lv_color_t * buf1_1 = malloc(sizeof(lv_color_t) * monitor_hor_res * monitor_ver_res);
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, NULL, monitor_hor_res * monitor_ver_res);

    /*Create a display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = sdl_display_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.hor_res = monitor_hor_res;
    disp_drv.ver_res = monitor_ver_res;
    disp1 = lv_disp_drv_register(&disp_drv);

    //lv_group_t * g = lv_group_create();
    //lv_group_set_default(g);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = sdl_mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = sdl_keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    //lv_indev_set_group(kb_indev, g);
    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = sdl_mousewheel_read;

    lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
    //lv_indev_set_group(enc_indev, g);

    /* Optional:
     * Create a memory monitor task which prints the memory usage in periodically.*/
    lv_timer_create(memory_monitor, 3000, NULL);
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_timer_t * param)
{
    (void) param; /*Unused*/
}
