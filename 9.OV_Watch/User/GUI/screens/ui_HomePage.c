#include "../ui.h"
#include "../assets/watch_anime_assets.h"
#include "lcd_init.h"

lv_obj_t * ui_HomePage = NULL;

static lv_obj_t * ui_AppsPage = NULL;
static lv_obj_t * ui_AIPage = NULL;
static lv_obj_t * ui_NoticePage = NULL;
static lv_obj_t * ui_SettingsPage = NULL;

static lv_obj_t * ui_connection_label = NULL;
static lv_obj_t * ui_status_label = NULL;
static lv_obj_t * ui_detail_label = NULL;
static lv_obj_t * ui_temperature_label = NULL;
static lv_obj_t * ui_humidity_label = NULL;
static lv_obj_t * ui_steps_label = NULL;
static lv_obj_t * ui_app_status_label = NULL;
static lv_obj_t * ui_brightness_slider = NULL;
static lv_obj_t * ui_brightness_value = NULL;
static lv_obj_t * ui_quick_panel = NULL;
static lv_obj_t * ui_quick_brightness_slider = NULL;
static lv_obj_t * ui_quick_status_label = NULL;
static bool ui_quick_panel_visible = false;

static void ui_brightness_event(lv_event_t * event);

static lv_obj_t * ui_create_label(lv_obj_t * parent, const char * text,
                                  lv_color_t color, const lv_font_t * font)
{
    lv_obj_t * label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    return label;
}

static void ui_style_screen(lv_obj_t * screen)
{
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1428), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
}

static void ui_create_anime_background(lv_obj_t * parent, const lv_img_dsc_t * source)
{
    lv_obj_t * image = lv_img_create(parent);
    lv_obj_t * shade = lv_obj_create(parent);

    lv_img_set_src(image, source);
    lv_obj_set_size(image, 144, 168);
    lv_obj_set_pos(image, 48, 56);
    lv_img_set_pivot(image, 72, 84);
    lv_img_set_zoom(image, 427);
    lv_img_set_antialias(image, true);
    lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_pos(shade, 0, 0);
    lv_obj_set_size(shade, 240, 280);
    lv_obj_clear_flag(shade, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(shade, lv_color_hex(0x071329), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(shade, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_width(shade, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(shade, 0, LV_PART_MAIN);
}

static lv_obj_t * ui_create_card(lv_obj_t * parent, lv_coord_t x, lv_coord_t y,
                                 lv_coord_t width, lv_coord_t height)
{
    lv_obj_t * card = lv_obj_create(parent);

    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x111E3A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(0x8F7BCA), LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN);
    return card;
}

static void ui_style_brightness_slider(lv_obj_t * slider, lv_coord_t x, lv_coord_t y,
                                        lv_coord_t width, lv_coord_t height)
{
    lv_obj_set_pos(slider, x, y);
    lv_obj_set_size(slider, width, height);
    lv_slider_set_range(slider, 5, 100);
    lv_slider_set_value(slider, LCD_Get_Light(), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x10213E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(slider, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(slider, lv_color_hex(0x5F7FA7), LV_PART_MAIN);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x62D7E9), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_width(slider, 1, LV_PART_KNOB);
    lv_obj_set_style_height(slider, 1, LV_PART_KNOB);
    lv_obj_add_event_cb(slider, ui_brightness_event, LV_EVENT_VALUE_CHANGED, NULL);
}

static lv_obj_t * ui_create_action_button(lv_obj_t * parent, const char * text,
                                          lv_coord_t width, lv_coord_t height,
                                          lv_event_cb_t event_cb)
{
    lv_obj_t * button = lv_btn_create(parent);
    lv_obj_t * label = lv_label_create(button);

    lv_obj_set_size(button, width, height);
    lv_obj_set_style_radius(button, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x14264A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x245070), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(0xD38EBF), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(button, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, NULL);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFF6EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(label);
    return button;
}

static void ui_add_sun_icon(lv_obj_t * parent, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t * icon = lv_label_create(parent);

    lv_label_set_text(icon, "*");
    lv_obj_set_pos(icon, x, y);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFD27A), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_18, LV_PART_MAIN);
}

static void ui_apps_button_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_AppsPage);
}

static void ui_ai_button_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_AIPage);
}

static void ui_apps_back_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_HomePage);
}

static void ui_ai_back_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_HomePage);
}

static void ui_environment_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_label_set_text(ui_app_status_label, "ENV / AHT21 PLACEHOLDER");
}

static void ui_ai_app_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_AIPage);
}

static void ui_rtc_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_label_set_text(ui_app_status_label, "RTC / SETUP MODULE NEXT");
}

static void ui_settings_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_SettingsPage);
}

static void ui_settings_back_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_AppsPage);
}

static void ui_brightness_event(lv_event_t * event)
{
    lv_obj_t * slider;
    int32_t value;

    if(lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;
    slider = lv_event_get_target(event);
    value = lv_slider_get_value(slider);
    LCD_Set_Light((uint8_t)value);
    if(ui_brightness_slider && slider != ui_brightness_slider) {
        lv_slider_set_value(ui_brightness_slider, value, LV_ANIM_OFF);
    }
    if(ui_quick_brightness_slider && slider != ui_quick_brightness_slider) {
        lv_slider_set_value(ui_quick_brightness_slider, value, LV_ANIM_OFF);
    }
    if(ui_brightness_value) lv_label_set_text_fmt(ui_brightness_value, "%ld%%", (long)value);
    if(ui_quick_status_label) lv_label_set_text_fmt(ui_quick_status_label, "%ld%%", (long)value);
}

static void ui_quick_panel_anim(int32_t end_y)
{
    lv_anim_t animation;

    if(!ui_quick_panel) return;
    lv_anim_del(ui_quick_panel, NULL);
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, ui_quick_panel);
    lv_anim_set_values(&animation, lv_obj_get_y(ui_quick_panel), end_y);
    lv_anim_set_time(&animation, 180);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&animation, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&animation);
}

static void ui_quick_close_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_quick_panel_visible = false;
    ui_quick_panel_anim(-170);
}

static void ui_quick_night_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_set_brightness(15);
    if(ui_quick_status_label) lv_label_set_text(ui_quick_status_label, "15%");
}

static void ui_quick_day_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_set_brightness(50);
    if(ui_quick_status_label) lv_label_set_text(ui_quick_status_label, "50%");
}

static void ui_quick_lock_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    if(ui_quick_status_label) lv_label_set_text(ui_quick_status_label, "LOCK");
}

void ui_handle_swipe(lv_dir_t direction)
{
    if(lv_scr_act() != ui_HomePage || !ui_quick_panel) return;

    if(direction == LV_DIR_BOTTOM && !ui_quick_panel_visible) {
        ui_quick_panel_visible = true;
        ui_quick_panel_anim(0);
    } else if(direction == LV_DIR_TOP && ui_quick_panel_visible) {
        ui_quick_panel_visible = false;
        ui_quick_panel_anim(-170);
    }
}

void ui_handle_touch(lv_coord_t x, lv_coord_t y)
{
    if(lv_scr_act() == ui_HomePage && ui_status_label) {
        lv_label_set_text_fmt(ui_status_label, "TOUCH / %d,%d", (int)x, (int)y);
    }
}

void ui_HomePage_screen_init(void)
{
    lv_obj_t * title;
    lv_obj_t * divider;
    lv_obj_t * section;
    lv_obj_t * clock;
    lv_obj_t * date;
    lv_obj_t * steps_card;
    lv_obj_t * env_card;
    lv_obj_t * label;
    lv_obj_t * progress;
    lv_obj_t * apps_button;
    lv_obj_t * ai_button;

    ui_HomePage = lv_obj_create(NULL);
    ui_style_screen(ui_HomePage);
    ui_create_anime_background(ui_HomePage, &watch_home);

    title = ui_create_label(ui_HomePage, "OV // WATCH", lv_color_hex(0xFFF6EF), &lv_font_montserrat_14);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 14);
    ui_connection_label = ui_create_label(ui_HomePage, "LIVE", lv_color_hex(0x76E8D0), &lv_font_montserrat_14);
    lv_obj_align(ui_connection_label, LV_ALIGN_TOP_RIGHT, -16, 14);

    divider = lv_obj_create(ui_HomePage);
    lv_obj_set_pos(divider, 16, 40);
    lv_obj_set_size(divider, 208, 1);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0xD68DBA), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(divider, 0, LV_PART_MAIN);

    section = ui_create_label(ui_HomePage, "WATCH STATUS / DEMO", lv_color_hex(0x8CECF5), &lv_font_montserrat_14);
    lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 50);
    clock = ui_create_label(ui_HomePage, "09:41", lv_color_hex(0xFFF6EF), &lv_font_montserrat_32);
    lv_obj_align(clock, LV_ALIGN_TOP_MID, 0, 68);
    date = ui_create_label(ui_HomePage, "FRI 21 AUG / 24 C", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
    lv_obj_align(date, LV_ALIGN_TOP_MID, 0, 111);

    steps_card = ui_create_card(ui_HomePage, 14, 143, 102, 68);
    label = ui_create_label(steps_card, "STEP QUEST", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 9, 8);
    ui_steps_label = ui_create_label(steps_card, "4.2K", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
    lv_obj_align(ui_steps_label, LV_ALIGN_TOP_LEFT, 9, 29);
    progress = lv_bar_create(steps_card);
    lv_obj_set_pos(progress, 9, 55);
    lv_obj_set_size(progress, 84, 5);
    lv_bar_set_range(progress, 0, 100);
    lv_bar_set_value(progress, 76, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x17284A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x77E8EF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progress, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_radius(progress, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    env_card = ui_create_card(ui_HomePage, 124, 143, 102, 68);
    label = ui_create_label(env_card, "AHT21", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 9, 8);
    ui_temperature_label = ui_create_label(env_card, "24 C", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
    lv_obj_align(ui_temperature_label, LV_ALIGN_TOP_LEFT, 9, 29);
    ui_humidity_label = ui_create_label(env_card, "48 %", lv_color_hex(0xFFD27A), &lv_font_montserrat_14);
    lv_obj_align(ui_humidity_label, LV_ALIGN_TOP_LEFT, 9, 51);

    ui_status_label = ui_create_label(ui_HomePage, "READY / TOUCH", lv_color_hex(0xFFF6EF), &lv_font_montserrat_14);
    lv_obj_align(ui_status_label, LV_ALIGN_TOP_MID, 0, 216);
    ui_detail_label = ui_create_label(ui_HomePage, "DEMO DATA / PLACEHOLDER", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
    lv_obj_align(ui_detail_label, LV_ALIGN_TOP_MID, 0, 234);

    apps_button = ui_create_action_button(ui_HomePage, "APPS", 96, 28, ui_apps_button_event);
    lv_obj_align(apps_button, LV_ALIGN_TOP_LEFT, 14, 250);
    ai_button = ui_create_action_button(ui_HomePage, "AI", 96, 28, ui_ai_button_event);
    lv_obj_align(ai_button, LV_ALIGN_TOP_RIGHT, -14, 250);

    ui_AppsPage = lv_obj_create(NULL);
    ui_style_screen(ui_AppsPage);
    ui_create_anime_background(ui_AppsPage, &watch_apps);
    {
        lv_obj_t * back = ui_create_action_button(ui_AppsPage, "<", 34, 28, ui_apps_back_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_AppsPage, "APPS", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        section = ui_create_label(ui_AppsPage, "WATCH MODULES / 04", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 48);
        {
            lv_obj_t * env = ui_create_action_button(ui_AppsPage, "ENV", 96, 42, ui_environment_event);
            lv_obj_t * ai = ui_create_action_button(ui_AppsPage, "AI", 96, 42, ui_ai_app_event);
            lv_obj_t * rtc = ui_create_action_button(ui_AppsPage, "RTC", 96, 42, ui_rtc_event);
            lv_obj_t * settings = ui_create_action_button(ui_AppsPage, "SET", 96, 42, ui_settings_event);
            lv_obj_set_pos(env, 14, 78);
            lv_obj_set_pos(ai, 130, 78);
            lv_obj_set_pos(rtc, 14, 130);
            lv_obj_set_pos(settings, 130, 130);
        }
        ui_app_status_label = ui_create_label(ui_AppsPage, "CHOOSE A MODULE", lv_color_hex(0xFFF6EF), &lv_font_montserrat_14);
        lv_obj_align(ui_app_status_label, LV_ALIGN_TOP_MID, 0, 200);
        label = ui_create_label(ui_AppsPage, "MODULES ARE PLACEHOLDERS", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 224);
    }

    ui_AIPage = lv_obj_create(NULL);
    ui_style_screen(ui_AIPage);
    ui_create_anime_background(ui_AIPage, &watch_ai);
    {
        lv_obj_t * back = ui_create_action_button(ui_AIPage, "<", 34, 28, ui_ai_back_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_AIPage, "AI LINK", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        section = ui_create_label(ui_AIPage, "LISTENING / ESP32", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 52);
        label = ui_create_label(ui_AIPage, "I'M HERE", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 78);
        label = ui_create_card(ui_AIPage, 24, 116, 192, 54);
        lv_obj_set_style_bg_opa(label, LV_OPA_80, LV_PART_MAIN);
        {
            lv_obj_t * copy = ui_create_label(label, "VOICE CHANNEL READY", lv_color_hex(0xFFF6EF), &lv_font_montserrat_14);
            lv_obj_align(copy, LV_ALIGN_CENTER, 0, 0);
        }
        label = ui_create_label(ui_AIPage, "DEMO / ESP32 NOT CONNECTED", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 204);
    }

    ui_NoticePage = lv_obj_create(NULL);
    ui_style_screen(ui_NoticePage);
    ui_create_anime_background(ui_NoticePage, &watch_notice);
    {
        lv_obj_t * back = ui_create_action_button(ui_NoticePage, "<", 34, 28, ui_apps_back_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_NoticePage, "NOTICE", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        section = ui_create_label(ui_NoticePage, "MOOD / ALERT", lv_color_hex(0xFFD27A), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 54);
        label = ui_create_label(ui_NoticePage, "HEY, LOOK HERE", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 84);
        label = ui_create_label(ui_NoticePage, "NEW MESSAGE PLACEHOLDER", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 126);
        label = ui_create_label(ui_NoticePage, "DEMO / NOTIFICATION", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 204);
    }

    ui_SettingsPage = lv_obj_create(NULL);
    ui_style_screen(ui_SettingsPage);
    {
        lv_obj_t * back = ui_create_action_button(ui_SettingsPage, "<", 34, 28, ui_settings_back_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_SettingsPage, "SETTINGS", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        section = ui_create_label(ui_SettingsPage, "BRIGHTNESS", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 52);
        ui_brightness_slider = lv_slider_create(ui_SettingsPage);
        ui_style_brightness_slider(ui_brightness_slider, 96, 75, 48, 96);
        ui_add_sun_icon(ui_SettingsPage, 108, 140);
        ui_brightness_value = ui_create_label(ui_SettingsPage, "20%", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(ui_brightness_value, LV_ALIGN_TOP_MID, 0, 182);
        lv_label_set_text_fmt(ui_brightness_value, "%d%%", (int)LCD_Get_Light());
        label = ui_create_label(ui_SettingsPage, "DISPLAY BRIGHTNESS", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 204);
        label = ui_create_label(ui_SettingsPage, "PWM / PA15 / TIM2 CH1", lv_color_hex(0x6D87A7), &lv_font_montserrat_14);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 230);
    }

    ui_quick_panel = lv_obj_create(ui_HomePage);
    lv_obj_set_pos(ui_quick_panel, 0, -170);
    lv_obj_set_size(ui_quick_panel, 240, 170);
    lv_obj_clear_flag(ui_quick_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_quick_panel, lv_color_hex(0x0D1832), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_quick_panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_quick_panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_quick_panel, lv_color_hex(0xD38EBF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_quick_panel, 0, LV_PART_MAIN);
    {
        lv_obj_t * panel_title = ui_create_label(ui_quick_panel, "QUICK PANEL", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_set_pos(panel_title, 15, 12);
        lv_obj_t * close = ui_create_action_button(ui_quick_panel, "^", 34, 26, ui_quick_close_event);
        lv_obj_set_pos(close, 190, 10);
        section = ui_create_label(ui_quick_panel, "BRIGHTNESS", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_set_pos(section, 84, 43);
        ui_quick_brightness_slider = lv_slider_create(ui_quick_panel);
        ui_style_brightness_slider(ui_quick_brightness_slider, 18, 40, 48, 96);
        ui_add_sun_icon(ui_quick_panel, 30, 106);
        ui_quick_status_label = ui_create_label(ui_quick_panel, "20%", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_set_pos(ui_quick_status_label, 84, 66);
        lv_label_set_text_fmt(ui_quick_status_label, "%d%%", (int)LCD_Get_Light());
        {
            lv_obj_t * night = ui_create_action_button(ui_quick_panel, "NIGHT", 46, 28, ui_quick_night_event);
            lv_obj_t * day = ui_create_action_button(ui_quick_panel, "DAY", 46, 28, ui_quick_day_event);
            lv_obj_t * lock = ui_create_action_button(ui_quick_panel, "LOCK", 46, 28, ui_quick_lock_event);
            lv_obj_set_pos(night, 82, 108);
            lv_obj_set_pos(day, 132, 108);
            lv_obj_set_pos(lock, 182, 108);
        }
    }
}

void ui_set_sensor_values(const char * temperature, const char * humidity)
{
    if(ui_temperature_label && temperature) lv_label_set_text(ui_temperature_label, temperature);
    if(ui_humidity_label && humidity) lv_label_set_text(ui_humidity_label, humidity);
}

void ui_set_steps(uint32_t steps)
{
    if(ui_steps_label) lv_label_set_text_fmt(ui_steps_label, "%lu", (unsigned long)steps);
}

void ui_set_connection_state(const char * state)
{
    if(ui_connection_label && state) lv_label_set_text(ui_connection_label, state);
}

void ui_set_brightness(uint8_t percent)
{
    if(percent > 100U) percent = 100U;
    LCD_Set_Light(percent);
    if(ui_brightness_slider) lv_slider_set_value(ui_brightness_slider, percent, LV_ANIM_OFF);
    if(ui_quick_brightness_slider) lv_slider_set_value(ui_quick_brightness_slider, percent, LV_ANIM_OFF);
    if(ui_brightness_value) lv_label_set_text_fmt(ui_brightness_value, "%d%%", (int)percent);
    if(ui_quick_status_label) lv_label_set_text_fmt(ui_quick_status_label, "%d%%", (int)percent);
}

void ui_HomePage_screen_destroy(void)
{
    if(ui_SettingsPage) lv_obj_del(ui_SettingsPage);
    if(ui_NoticePage) lv_obj_del(ui_NoticePage);
    if(ui_AIPage) lv_obj_del(ui_AIPage);
    if(ui_AppsPage) lv_obj_del(ui_AppsPage);
    if(ui_HomePage) lv_obj_del(ui_HomePage);

    ui_HomePage = NULL;
    ui_AppsPage = NULL;
    ui_AIPage = NULL;
    ui_NoticePage = NULL;
    ui_SettingsPage = NULL;
    ui_connection_label = NULL;
    ui_status_label = NULL;
    ui_detail_label = NULL;
    ui_temperature_label = NULL;
    ui_humidity_label = NULL;
    ui_steps_label = NULL;
    ui_app_status_label = NULL;
    ui_brightness_slider = NULL;
    ui_brightness_value = NULL;
    ui_quick_panel = NULL;
    ui_quick_brightness_slider = NULL;
    ui_quick_status_label = NULL;
    ui_quick_panel_visible = false;
}
