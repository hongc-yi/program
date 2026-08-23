/**
 * @file lv_port_indev_templ.c
 *
 */

 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "lvgl.h"
#include "ui.h"
#include "CST816.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);
static uint8_t touchpad_finger_count = 0;

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_touchpad;
extern CST816_Info	CST816_Instance;

static bool touch_active = false;
static bool touch_gesture_sent = false;
static bool touch_feedback_sent = false;
static lv_coord_t touch_start_y = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    /*Your code comes here*/
    CST816_GPIO_Init();
    CST816_RESET();
}

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*Save the pressed coordinates and the state.*/
    if(touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;

        if(!touch_active) {
            touch_active = true;
            touch_gesture_sent = false;
            touch_feedback_sent = false;
            touch_start_y = last_y;
        }
        if(!touch_feedback_sent) {
            ui_handle_touch(last_x, last_y);
            touch_feedback_sent = true;
        }
    } else {
        data->state = LV_INDEV_STATE_REL;

        if(touch_active && !touch_gesture_sent) {
            lv_coord_t delta_y = last_y - touch_start_y;
            if(touch_start_y < 70 && delta_y > 35) {
                ui_handle_swipe(LV_DIR_BOTTOM);
                touch_gesture_sent = true;
            } else if(delta_y < -35) {
                ui_handle_swipe(LV_DIR_TOP);
                touch_gesture_sent = true;
            }
        }

        touch_active = false;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/
	touchpad_finger_count = CST816_Get_FingerNum();
	if(touchpad_finger_count!=0x00 && touchpad_finger_count!=0xFF)
	{return true;}
	else
  {return false;}
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/
		CST816_Get_XY_AXIS();
    (*x) = CST816_Instance.X_Pos;
    (*y) = CST816_Instance.Y_Pos;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
