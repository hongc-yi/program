
#include "ui.h"

///////////////////// VARIABLES ////////////////////

// IMAGES AND IMAGE SETS

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

static lv_obj_t * ui_BootPage = NULL;
static lv_timer_t * ui_boot_timer = NULL;

static void ui_boot_timer_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    /* Switch directly so the boot screen cannot retain input or remain visible. */
    lv_scr_load(ui_HomePage);
    if(ui_BootPage) {
        lv_obj_del(ui_BootPage);
        ui_BootPage = NULL;
    }
    ui_boot_timer = NULL;
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_obj_t * title;
    lv_obj_t * subtitle;

    lv_disp_set_theme(dispp, theme);
    ui_HomePage_screen_init();

    ui_BootPage = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_BootPage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_BootPage, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_BootPage, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_BootPage, 0, LV_PART_MAIN);

    title = lv_label_create(ui_BootPage);
    lv_label_set_text(title, "OV-WATCH");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFF6EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -14);

    subtitle = lv_label_create(ui_BootPage);
    lv_label_set_text(subtitle, "WELCOME / SYSTEM READY");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x77E8EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 20);

    lv_disp_load_scr(ui_BootPage);
    ui_boot_timer = lv_timer_create(ui_boot_timer_cb, 2500, NULL);
    lv_timer_set_repeat_count(ui_boot_timer, 1);
}

void ui_destroy(void)
{
    if(ui_boot_timer) {
        lv_timer_del(ui_boot_timer);
        ui_boot_timer = NULL;
    }
    if(ui_BootPage) {
        lv_obj_del(ui_BootPage);
        ui_BootPage = NULL;
    }
    ui_HomePage_screen_destroy();
}
