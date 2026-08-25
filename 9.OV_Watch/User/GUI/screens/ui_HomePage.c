#include "../ui.h"
#include "../assets/watch_anime_assets.h"
#include "lcd_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lv_obj_t * ui_HomePage = NULL;

static lv_obj_t * ui_AppsPage = NULL;
static lv_obj_t * ui_AIPage = NULL;
static lv_obj_t * ui_CalcPage = NULL;
static lv_obj_t * ui_StopwatchPage = NULL;
static lv_obj_t * ui_RtcPage = NULL;
static lv_obj_t * ui_NoticePage = NULL;
static lv_obj_t * ui_SettingsPage = NULL;

static lv_obj_t * ui_connection_label = NULL;
static lv_obj_t * ui_status_label = NULL;
static lv_obj_t * ui_detail_label = NULL;
static lv_obj_t * ui_temperature_label = NULL;
static lv_obj_t * ui_humidity_label = NULL;
static lv_obj_t * ui_app_status_label = NULL;
static lv_obj_t * ui_timeout_value = NULL;
static lv_obj_t * ui_brightness_slider = NULL;
static lv_obj_t * ui_brightness_value = NULL;
static lv_obj_t * ui_quick_panel = NULL;
static lv_obj_t * ui_quick_brightness_slider = NULL;
static lv_obj_t * ui_quick_brightness_value = NULL;

static lv_obj_t * ui_calc_display = NULL;
static lv_obj_t * ui_stopwatch_display = NULL;
static lv_obj_t * ui_timer_mode_label = NULL;
static lv_obj_t * ui_timer_status_label = NULL;
static lv_obj_t * ui_home_clock_label = NULL;
static lv_obj_t * ui_home_date_label = NULL;
static lv_obj_t * ui_time_clock_label = NULL;
static lv_obj_t * ui_time_status_label = NULL;
static lv_timer_t * ui_stopwatch_timer = NULL;
static lv_timer_t * ui_time_timer = NULL;
static lv_obj_t * ui_timer_control_button = NULL;
static lv_obj_t * ui_timer_adjust_up_button = NULL;
static lv_obj_t * ui_timer_adjust_down_button = NULL;
static uint32_t ui_screen_timeout_seconds = 30U;
static uint32_t ui_last_activity_tick = 0U;
static uint32_t ui_stopwatch_ticks = 0;
static uint32_t ui_stopwatch_last_tick = 0;
static uint32_t ui_stopwatch_accumulated_ms = 0;
static uint32_t ui_countdown_seconds = 60;
static uint32_t ui_countdown_last_tick = 0;
static uint32_t ui_countdown_accumulated_ms = 0;
static uint32_t ui_alarm_seconds = 7U * 3600U + 30U * 60U;
static uint32_t ui_alarm_last_clock_seconds = 0;
static bool ui_alarm_last_clock_valid = false;
static uint32_t ui_clock_seconds = 9U * 3600U + 41U * 60U;
static uint32_t ui_clock_base_tick = 0;
static uint32_t ui_time_edit_seconds = 9U * 3600U + 41U * 60U;
static uint8_t ui_timer_mode = 0;
static bool ui_stopwatch_running = false;
static bool ui_countdown_running = false;
static bool ui_alarm_enabled = false;
static bool ui_alarm_ringing = false;
static bool ui_time_editing = false;
static bool ui_quick_panel_visible = false;
static bool ui_screen_off = false;

static char ui_calc_value[24] = "0";
static char ui_calc_pending_operator = 0;
static double ui_calc_left = 0.0;
static bool ui_calc_entering_value = false;

static void ui_brightness_event(lv_event_t * event);
static void ui_calc_button_event(lv_event_t * event);
static void ui_calc_back_event(lv_event_t * event);
static void ui_calc_reset(void);
static void ui_stopwatch_timer_cb(lv_timer_t * timer);
static void ui_time_timer_cb(lv_timer_t * timer);
static void ui_timer_mode_event(lv_event_t * event);
static void ui_timer_control_event(lv_event_t * event);
static void ui_timer_reset_event(lv_event_t * event);
static void ui_timer_adjust_event(lv_event_t * event);
static void ui_time_adjust_event(lv_event_t * event);
static void ui_time_apply_event(lv_event_t * event);
static void ui_time_back_event(lv_event_t * event);
static void ui_timeout_adjust_event(lv_event_t * event);
static void ui_timer_control_action(void);
static void ui_time_apply_action(void);
static void ui_screen_timeout_refresh(void);
static void ui_screen_timeout_adjust(int32_t delta);
static void ui_screen_timeout_check(void);
static void ui_alarm_check(uint32_t current);
static void ui_timer_refresh(void);
static void ui_time_refresh(void);

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

static lv_obj_t * ui_create_app_row(lv_obj_t * parent, const char * icon_text,
                                    const lv_font_t * icon_font, lv_color_t icon_color,
                                    const char * title_text, lv_event_cb_t event_cb)
{
    lv_obj_t * row = lv_btn_create(parent);
    lv_obj_t * icon_bg = lv_obj_create(row);
    lv_obj_t * icon = lv_label_create(icon_bg);
    lv_obj_t * title = lv_label_create(row);

    lv_obj_set_size(row, 196, 56);
    lv_obj_set_style_radius(row, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x21466E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(row, LV_OPA_80, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(row, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(row, event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_set_size(icon_bg, 44, 44);
    lv_obj_set_pos(icon_bg, 6, 6);
    lv_obj_set_style_bg_color(icon_bg, icon_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(icon_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(icon_bg, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(icon_bg, 0, LV_PART_MAIN);
    lv_obj_clear_flag(icon_bg, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_label_set_text(icon, icon_text);
    lv_obj_center(icon);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, icon_font, LV_PART_MAIN);

    lv_label_set_text(title, title_text);
    lv_obj_set_pos(title, 62, 16);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFF6EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, LV_PART_MAIN);
    return row;
}

static lv_obj_t * ui_create_quick_toggle(lv_obj_t * parent, const char * icon_text,
                                         const char * caption_text, lv_event_cb_t event_cb)
{
    lv_obj_t * button = lv_btn_create(parent);
    lv_obj_t * icon = lv_label_create(button);
    lv_obj_t * caption = lv_label_create(button);

    lv_obj_set_size(button, 92, 54);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x14264A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x245070), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(0x385B7A), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(button, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, NULL);

    lv_label_set_text(icon, icon_text);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFF6EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_18, LV_PART_MAIN);

    lv_label_set_text(caption, caption_text);
    lv_obj_align(caption, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_text_color(caption, lv_color_hex(0x77E8EF), LV_PART_MAIN);
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, LV_PART_MAIN);
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

void ui_open_apps(void)
{
    if(ui_AppsPage && lv_scr_act() == ui_HomePage) lv_scr_load(ui_AppsPage);
}

void ui_handle_key_confirm(void)
{
    if(ui_screen_off) {
        LCD_Open_Light();
        ui_screen_off = false;
        ui_screen_timeout_refresh();
        return;
    }

    ui_screen_timeout_refresh();
    if(lv_scr_act() == ui_StopwatchPage) {
        ui_timer_control_action();
    } else if(lv_scr_act() == ui_RtcPage) {
        ui_time_apply_action();
    } else if(lv_scr_act() == ui_AppsPage) {
        lv_scr_load(ui_HomePage);
    } else if(lv_scr_act() == ui_HomePage) {
        ui_open_apps();
    }
}

void ui_set_time_from_network(uint8_t hour, uint8_t minute, uint8_t second)
{
    if(hour >= 24U || minute >= 60U || second >= 60U) return;
    ui_clock_seconds = (uint32_t)hour * 3600U + (uint32_t)minute * 60U + second;
    ui_clock_base_tick = HAL_GetTick();
    ui_time_edit_seconds = ui_clock_seconds;
    ui_time_editing = false;
    ui_alarm_last_clock_valid = false;
    ui_alarm_ringing = false;
    ui_time_refresh();
}

static void ui_screen_timeout_refresh(void)
{
    ui_last_activity_tick = HAL_GetTick();
}

static void ui_screen_timeout_adjust(int32_t delta)
{
    int32_t value = (int32_t)ui_screen_timeout_seconds + delta;
    if(value < 0) value = 0;
    if(value > 60) value = 60;
    ui_screen_timeout_seconds = (uint32_t)value;
    ui_screen_timeout_refresh();
    if(ui_timeout_value) {
        if(ui_screen_timeout_seconds == 0U) lv_label_set_text(ui_timeout_value, "OFF");
        else lv_label_set_text_fmt(ui_timeout_value, "%lus", (unsigned long)ui_screen_timeout_seconds);
    }
}

static void ui_screen_timeout_check(void)
{
    if(ui_screen_off || ui_screen_timeout_seconds == 0U) return;
    if((HAL_GetTick() - ui_last_activity_tick) >= ui_screen_timeout_seconds * 1000U) {
        LCD_Close_Light();
        ui_screen_off = true;
    }
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

static void ui_calc_app_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_calc_reset();
    lv_scr_load(ui_CalcPage);
}

static void ui_timeout_adjust_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_screen_timeout_adjust((int32_t)(intptr_t)lv_obj_get_user_data(lv_event_get_target(event)));
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
    if(ui_quick_brightness_value) lv_label_set_text_fmt(ui_quick_brightness_value, "%ld%%", (long)value);

}

static void ui_calc_reset(void)
{
    strcpy(ui_calc_value, "0");
    ui_calc_pending_operator = 0;
    ui_calc_left = 0.0;
    ui_calc_entering_value = false;
    if(ui_calc_display) lv_label_set_text(ui_calc_display, ui_calc_value);
}

static void ui_calc_refresh(void)
{
    if(ui_calc_display) lv_label_set_text(ui_calc_display, ui_calc_value);
}

static void ui_calc_set_error(void)
{
    strcpy(ui_calc_value, "ERROR");
    ui_calc_pending_operator = 0;
    ui_calc_entering_value = false;
    ui_calc_refresh();
}

static void ui_calc_apply_operator(char op)
{
    double right = atof(ui_calc_value);
    double result = ui_calc_left;

    if(op == '+') result += right;
    else if(op == '-') result -= right;
    else if(op == '*') result *= right;
    else if(op == '/') {
        if(right == 0.0) {
            ui_calc_set_error();
            return;
        }
        result /= right;
    }

    if(result > 999999999.0 || result < -999999999.0) {
        ui_calc_set_error();
        return;
    }

    snprintf(ui_calc_value, sizeof(ui_calc_value), "%.8g", result);
}

static void ui_calc_handle_input(const char * input)
{
    char op;

    if(strcmp(input, "C") == 0) {
        ui_calc_reset();
        return;
    }

    if(strcmp(input, "DEL") == 0) {
        size_t length = strlen(ui_calc_value);
        if(!ui_calc_entering_value || length <= 1U || (length == 2U && ui_calc_value[0] == '-')) {
            strcpy(ui_calc_value, "0");
        } else {
            ui_calc_value[length - 1U] = '\0';
        }
        ui_calc_entering_value = true;
        ui_calc_refresh();
        return;
    }

    if(strcmp(input, "+/-") == 0) {
        char signed_value[24];
        if(strcmp(ui_calc_value, "0") != 0 && strcmp(ui_calc_value, "ERROR") != 0) {
            if(ui_calc_value[0] == '-') {
                memmove(ui_calc_value, ui_calc_value + 1, strlen(ui_calc_value));
            } else if(strlen(ui_calc_value) < sizeof(ui_calc_value) - 1U) {
                snprintf(signed_value, sizeof(signed_value), "-%s", ui_calc_value);
                strcpy(ui_calc_value, signed_value);
            }
            ui_calc_entering_value = true;
            ui_calc_refresh();
        }
        return;
    }

    if(strcmp(input, ".") == 0) {
        if(!ui_calc_entering_value) {
            strcpy(ui_calc_value, "0.");
            ui_calc_entering_value = true;
        } else if(strchr(ui_calc_value, '.') == NULL) {
            strncat(ui_calc_value, ".", sizeof(ui_calc_value) - strlen(ui_calc_value) - 1U);
        }
        ui_calc_refresh();
        return;
    }

    if(strlen(input) == 1U && input[0] >= '0' && input[0] <= '9') {
        if(!ui_calc_entering_value || strcmp(ui_calc_value, "0") == 0 || strcmp(ui_calc_value, "ERROR") == 0) {
            strcpy(ui_calc_value, input);
        } else if(strlen(ui_calc_value) < sizeof(ui_calc_value) - 1U) {
            strcat(ui_calc_value, input);
        }
        ui_calc_entering_value = true;
        ui_calc_refresh();
        return;
    }

    if(strlen(input) == 1U && strchr("+-*/", input[0]) != NULL) {
        op = input[0];
        if(ui_calc_pending_operator && ui_calc_entering_value) {
            ui_calc_apply_operator(ui_calc_pending_operator);
            if(strcmp(ui_calc_value, "ERROR") == 0) return;
        } else {
            ui_calc_left = atof(ui_calc_value);
        }
        ui_calc_pending_operator = op;
        ui_calc_entering_value = false;
        ui_calc_refresh();
        return;
    }

    if(strcmp(input, "=") == 0 && ui_calc_pending_operator) {
        ui_calc_apply_operator(ui_calc_pending_operator);
        ui_calc_pending_operator = 0;
        ui_calc_entering_value = false;
        ui_calc_refresh();
    }
}

static void ui_calc_button_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_calc_handle_input((const char *)lv_obj_get_user_data(lv_event_get_target(event)));
}

static void ui_calc_back_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    lv_scr_load(ui_AppsPage);
}

static void ui_button_set_text(lv_obj_t * button, const char * text)
{
    lv_obj_t * label;
    if(!button) return;
    label = lv_obj_get_child(button, 0);
    if(label) lv_label_set_text(label, text);
}

static void ui_time_refresh(void)
{
    uint32_t elapsed = (HAL_GetTick() - ui_clock_base_tick) / 1000U;
    uint32_t seconds = (ui_clock_seconds + elapsed) % 86400U;
    uint32_t display_seconds = ui_time_editing ? ui_time_edit_seconds : seconds;
    uint32_t hours = display_seconds / 3600U;
    uint32_t minutes = (display_seconds / 60U) % 60U;

    if(ui_home_clock_label) {
        lv_label_set_text_fmt(ui_home_clock_label, "%02lu:%02lu", (unsigned long)hours, (unsigned long)minutes);
    }
    if(ui_time_clock_label) {
        lv_label_set_text_fmt(ui_time_clock_label, "%02lu:%02lu:%02lu",
                              (unsigned long)hours, (unsigned long)minutes,
                              (unsigned long)(display_seconds % 60U));
    }
}

static void ui_timer_refresh(void)
{
    if(!ui_stopwatch_display) return;

    if(ui_timer_mode == 0U) {
        lv_label_set_text_fmt(ui_stopwatch_display, "%02lu:%02lu.%01lu",
                              (unsigned long)(ui_stopwatch_ticks / 600U),
                              (unsigned long)((ui_stopwatch_ticks / 10U) % 60U),
                              (unsigned long)(ui_stopwatch_ticks % 10U));
        if(ui_timer_mode_label) lv_label_set_text(ui_timer_mode_label, "STOPWATCH");
        if(ui_timer_status_label) lv_label_set_text(ui_timer_status_label, ui_stopwatch_running ? "RUNNING" : "PAUSED");
        ui_button_set_text(ui_timer_control_button, ui_stopwatch_running ? "PAUSE" : "START");
        if(ui_timer_adjust_up_button) lv_obj_add_flag(ui_timer_adjust_up_button, LV_OBJ_FLAG_HIDDEN);
        if(ui_timer_adjust_down_button) lv_obj_add_flag(ui_timer_adjust_down_button, LV_OBJ_FLAG_HIDDEN);
    } else if(ui_timer_mode == 1U) {
        lv_label_set_text_fmt(ui_stopwatch_display, "%02lu:%02lu",
                              (unsigned long)(ui_countdown_seconds / 60U),
                              (unsigned long)(ui_countdown_seconds % 60U));
        if(ui_timer_mode_label) lv_label_set_text(ui_timer_mode_label, "COUNTDOWN");
        if(ui_timer_status_label) lv_label_set_text(ui_timer_status_label, ui_countdown_running ? "RUNNING" : "PAUSED");
        ui_button_set_text(ui_timer_control_button, ui_countdown_running ? "PAUSE" : "START");
        ui_button_set_text(ui_timer_adjust_up_button, "+1 MIN");
        ui_button_set_text(ui_timer_adjust_down_button, "-10 SEC");
        if(ui_timer_adjust_up_button) lv_obj_clear_flag(ui_timer_adjust_up_button, LV_OBJ_FLAG_HIDDEN);
        if(ui_timer_adjust_down_button) lv_obj_clear_flag(ui_timer_adjust_down_button, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_fmt(ui_stopwatch_display, "%02lu:%02lu",
                              (unsigned long)(ui_alarm_seconds / 3600U),
                              (unsigned long)((ui_alarm_seconds / 60U) % 60U));
        if(ui_timer_mode_label) lv_label_set_text(ui_timer_mode_label, "ALARM");
        if(ui_timer_status_label) {
            if(ui_alarm_ringing) lv_label_set_text(ui_timer_status_label, "ALARM DUE");
            else if(ui_alarm_enabled) lv_label_set_text(ui_timer_status_label, "ARMED");
            else lv_label_set_text(ui_timer_status_label, "DISABLED");
        }
        ui_button_set_text(ui_timer_control_button, ui_alarm_enabled ? "DISARM" : "ARM");
        ui_button_set_text(ui_timer_adjust_up_button, "+1 HOUR");
        ui_button_set_text(ui_timer_adjust_down_button, "-10 MIN");
        if(ui_timer_adjust_up_button) lv_obj_clear_flag(ui_timer_adjust_up_button, LV_OBJ_FLAG_HIDDEN);
        if(ui_timer_adjust_down_button) lv_obj_clear_flag(ui_timer_adjust_down_button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_stopwatch_timer_cb(lv_timer_t * timer)
{
    uint32_t now;
    uint32_t elapsed;
    uint32_t current;
    LV_UNUSED(timer);

    now = HAL_GetTick();
    current = (ui_clock_seconds + ((now - ui_clock_base_tick) / 1000U)) % 86400U;

    if(ui_stopwatch_running) {
        if(ui_stopwatch_last_tick == 0U) ui_stopwatch_last_tick = now;
        elapsed = now - ui_stopwatch_last_tick;
        ui_stopwatch_last_tick = now;
        ui_stopwatch_accumulated_ms += elapsed;
        ui_stopwatch_ticks = ui_stopwatch_accumulated_ms / 100U;
    }

    if(ui_countdown_running) {
        if(ui_countdown_last_tick == 0U) ui_countdown_last_tick = now;
        elapsed = now - ui_countdown_last_tick;
        ui_countdown_last_tick = now;
        ui_countdown_accumulated_ms += elapsed;
        while(ui_countdown_accumulated_ms >= 1000U && ui_countdown_seconds > 0U) {
            ui_countdown_accumulated_ms -= 1000U;
            ui_countdown_seconds--;
        }
        if(ui_countdown_seconds == 0U) {
            ui_countdown_running = false;
            ui_countdown_last_tick = 0U;
            ui_countdown_accumulated_ms = 0U;
        }
    }

    ui_alarm_check(current);
    ui_timer_refresh();
}

static void ui_time_timer_cb(lv_timer_t * timer)
{
    uint32_t current;
    LV_UNUSED(timer);
    ui_time_refresh();
    ui_screen_timeout_check();
    current = (ui_clock_seconds + ((HAL_GetTick() - ui_clock_base_tick) / 1000U)) % 86400U;
    ui_alarm_check(current);
    if(ui_alarm_ringing) ui_timer_refresh();
}

static void ui_stopwatch_app_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_timer_mode = 0U;
    ui_timer_refresh();
    lv_scr_load(ui_StopwatchPage);
}

static void ui_rtc_app_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_time_edit_seconds = (ui_clock_seconds + ((HAL_GetTick() - ui_clock_base_tick) / 1000U)) % 86400U;
    ui_time_editing = true;
    ui_time_refresh();
    lv_scr_load(ui_RtcPage);
}

static void ui_timer_mode_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_timer_mode = (uint8_t)(uintptr_t)lv_obj_get_user_data(lv_event_get_target(event));
    ui_alarm_ringing = false;
    ui_timer_refresh();
}

static void ui_timer_control_action(void)
{
    uint32_t now = HAL_GetTick();

    if(ui_timer_mode == 0U) {
        ui_stopwatch_running = !ui_stopwatch_running;
        ui_stopwatch_last_tick = ui_stopwatch_running ? now : 0U;
    } else if(ui_timer_mode == 1U) {
        if(ui_countdown_seconds == 0U) ui_countdown_seconds = 60U;
        ui_countdown_running = !ui_countdown_running;
        ui_countdown_last_tick = ui_countdown_running ? now : 0U;
    } else {
        ui_alarm_enabled = !ui_alarm_enabled;
        ui_alarm_ringing = false;
        ui_alarm_last_clock_valid = false;
    }
    if(ui_stopwatch_timer) {
        if(ui_stopwatch_running || ui_countdown_running || ui_alarm_enabled) lv_timer_resume(ui_stopwatch_timer);
        else lv_timer_pause(ui_stopwatch_timer);
    }
    ui_timer_refresh();
}

static void ui_timer_control_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_timer_control_action();
}

static void ui_timer_reset_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    if(ui_timer_mode == 0U) {
        ui_stopwatch_ticks = 0;
        ui_stopwatch_accumulated_ms = 0U;
        ui_stopwatch_last_tick = 0U;
        ui_stopwatch_running = false;
    } else if(ui_timer_mode == 1U) {
        ui_countdown_seconds = 60U;
        ui_countdown_accumulated_ms = 0U;
        ui_countdown_last_tick = 0U;
        ui_countdown_running = false;
    } else {
        ui_alarm_enabled = false;
        ui_alarm_ringing = false;
    }
    ui_timer_refresh();
}

static void ui_timer_adjust_event(lv_event_t * event)
{
    int32_t delta;
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    delta = (int32_t)(intptr_t)lv_obj_get_user_data(lv_event_get_target(event));
    if(ui_timer_mode == 1U) {
        if(delta > 0 && ui_countdown_seconds <= 5940U) ui_countdown_seconds += (uint32_t)delta;
        else if(delta < 0 && ui_countdown_seconds >= (uint32_t)(-delta)) ui_countdown_seconds -= (uint32_t)(-delta);
        ui_countdown_accumulated_ms = 0U;
    } else if(ui_timer_mode == 2U) {
        int32_t alarm_seconds = (int32_t)ui_alarm_seconds + delta * 60;
        while(alarm_seconds < 0) alarm_seconds += 86400;
        while(alarm_seconds >= 86400) alarm_seconds -= 86400;
        ui_alarm_seconds = (uint32_t)alarm_seconds;
        ui_alarm_last_clock_valid = false;
        ui_alarm_ringing = false;
    } else return;
    ui_timer_refresh();
}

static void ui_time_adjust_event(lv_event_t * event)
{
    int32_t delta;
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    delta = (int32_t)(intptr_t)lv_obj_get_user_data(lv_event_get_target(event));
    {
        int32_t edit_seconds = (int32_t)ui_time_edit_seconds + delta;
        while(edit_seconds < 0) edit_seconds += 86400;
        while(edit_seconds >= 86400) edit_seconds -= 86400;
        ui_time_edit_seconds = (uint32_t)edit_seconds;
    }
    if(ui_time_status_label) lv_label_set_text_fmt(ui_time_status_label, "EDIT %02lu:%02lu",
        (unsigned long)(ui_time_edit_seconds / 3600U),
        (unsigned long)((ui_time_edit_seconds / 60U) % 60U));
    ui_time_refresh();
}

static void ui_alarm_check(uint32_t current)
{
    uint32_t clock_delta;
    uint32_t alarm_distance;

    if(!ui_alarm_enabled) {
        ui_alarm_last_clock_valid = false;
        return;
    }

    if(!ui_alarm_last_clock_valid) {
        ui_alarm_last_clock_seconds = current;
        ui_alarm_last_clock_valid = true;
        if(current == ui_alarm_seconds) ui_alarm_ringing = true;
        return;
    }

    clock_delta = (current + 86400U - ui_alarm_last_clock_seconds) % 86400U;
    alarm_distance = (ui_alarm_seconds + 86400U - ui_alarm_last_clock_seconds) % 86400U;
    if(current == ui_alarm_seconds ||
       (clock_delta > 0U && alarm_distance > 0U && alarm_distance <= clock_delta)) {
        ui_alarm_ringing = true;
    }
    ui_alarm_last_clock_seconds = current;
}

static void ui_time_apply_action(void)
{
    ui_clock_seconds = ui_time_edit_seconds;
    ui_clock_base_tick = HAL_GetTick();
    ui_time_editing = false;
    ui_alarm_last_clock_valid = false;
    if(ui_time_status_label) lv_label_set_text(ui_time_status_label, "TIME APPLIED");
    ui_time_refresh();
}

static void ui_time_apply_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_time_apply_action();
}

static void ui_time_back_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    ui_time_editing = false;
    ui_time_refresh();
    lv_scr_load(ui_AppsPage);
}

static void ui_quick_panel_set_y(void * var, int32_t value)
{
    lv_obj_set_y((lv_obj_t *)var, (lv_coord_t)value);
}

static void ui_quick_panel_anim(int32_t end_y)
{
    lv_anim_t animation;

    if(!ui_quick_panel) return;
    lv_anim_del(ui_quick_panel, NULL);
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, ui_quick_panel);
    lv_anim_set_values(&animation, lv_obj_get_y(ui_quick_panel), end_y);
    lv_anim_set_time(&animation, 120);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&animation, ui_quick_panel_set_y);
    lv_anim_start(&animation);
}

static void ui_quick_panel_close(void)
{
    if(!ui_quick_panel || !ui_quick_panel_visible) return;
    ui_quick_panel_visible = false;
    ui_quick_panel_anim(-160);
}

static void ui_quick_lock_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    LCD_Close_Light();
    ui_screen_off = true;
    ui_quick_panel_close();
}

static void ui_quick_wifi_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;

}

static void ui_quick_bluetooth_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;

}

static void ui_quick_power_event(lv_event_t * event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;

}

void ui_handle_swipe(lv_dir_t direction)
{
    if(lv_scr_act() != ui_HomePage || !ui_quick_panel) return;

    if(direction == LV_DIR_BOTTOM && !ui_quick_panel_visible) {
        ui_quick_panel_visible = true;
        ui_quick_panel_anim(0);
    } else if(direction == LV_DIR_TOP && ui_quick_panel_visible) {
        ui_quick_panel_close();
    }
}

static bool ui_quick_panel_hit(lv_coord_t x, lv_coord_t y)
{
    lv_coord_t panel_y = lv_obj_get_y(ui_quick_panel);
    lv_coord_t panel_w = lv_obj_get_width(ui_quick_panel);
    lv_coord_t panel_h = lv_obj_get_height(ui_quick_panel);

    if(panel_y >= 0) { /* panel fully open (or animating downward) */
        if(x >= 0 && x < panel_w && y >= panel_y && y < panel_y + panel_h) return true;
    } else {
        /* panel still animating up: its visible bottom edge is panel_y + panel_h */
        if(x >= 0 && x < panel_w && y >= 0 && y < panel_y + panel_h) return true;
    }
    return false;
}

void ui_handle_touch(lv_coord_t x, lv_coord_t y)
{
    if(ui_screen_off) {
        LCD_Open_Light();
        ui_screen_off = false;
        ui_screen_timeout_refresh();
        ui_quick_panel_close();
        return;
    }

    ui_screen_timeout_refresh();
    if(lv_scr_act() == ui_HomePage) {
        /* Only dismiss the panel when the touch is OUTSIDE the panel area,
           so the toggle buttons and the brightness slider keep working. */
        if(ui_quick_panel_visible && !ui_quick_panel_hit(x, y)) ui_quick_panel_close();
        if(ui_status_label) lv_label_set_text_fmt(ui_status_label, "TOUCH / %d,%d", (int)x, (int)y);
    }
}

void ui_HomePage_screen_init(void)
{
    lv_obj_t * title;
    lv_obj_t * divider;
    lv_obj_t * section;
    lv_obj_t * env_card;
    lv_obj_t * label;

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
    ui_home_clock_label = ui_create_label(ui_HomePage, "09:41", lv_color_hex(0xFFF6EF), &lv_font_montserrat_32);
    lv_obj_align(ui_home_clock_label, LV_ALIGN_TOP_MID, 0, 68);
    ui_home_date_label = ui_create_label(ui_HomePage, "MANUAL TIME / 24 C", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
    lv_obj_align(ui_home_date_label, LV_ALIGN_TOP_MID, 0, 111);
    ui_clock_base_tick = HAL_GetTick();
    ui_screen_timeout_refresh();

    env_card = ui_create_card(ui_HomePage, 14, 143, 212, 74);
    label = ui_create_label(env_card, "AHT21 / ENVIRONMENT", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 8);
    ui_temperature_label = ui_create_label(env_card, "--.- C", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
    lv_obj_align(ui_temperature_label, LV_ALIGN_TOP_LEFT, 12, 33);
    ui_humidity_label = ui_create_label(env_card, "--.- %", lv_color_hex(0xFFD27A), &lv_font_montserrat_18);
    lv_obj_align(ui_humidity_label, LV_ALIGN_TOP_RIGHT, -12, 33);

    ui_status_label = ui_create_label(ui_HomePage, "READY / TOUCH", lv_color_hex(0xFFF6EF), &lv_font_montserrat_14);
    lv_obj_align(ui_status_label, LV_ALIGN_TOP_MID, 0, 228);
    ui_detail_label = ui_create_label(ui_HomePage, "ENVIRONMENT / MANUAL TIME", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
    lv_obj_align(ui_detail_label, LV_ALIGN_TOP_MID, 0, 248);

    ui_AppsPage = lv_obj_create(NULL);
    ui_style_screen(ui_AppsPage);
    {
        lv_obj_t * back = ui_create_action_button(ui_AppsPage, "<", 34, 28, ui_apps_back_event);
        lv_obj_t * list;
        lv_obj_t * item;
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_AppsPage, "APPS", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        list = lv_obj_create(ui_AppsPage);
        lv_obj_set_pos(list, 8, 48);
        lv_obj_set_size(list, 224, 222);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_top(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(list, 6, LV_PART_MAIN);
        lv_obj_set_style_pad_right(list, 6, LV_PART_MAIN);
        lv_obj_set_style_pad_row(list, 0, LV_PART_MAIN);

        item = ui_create_app_row(list, "\xEE\x9C\x86", &ui_font_iconfont34, lv_color_hex(0x77E8EF), "ENVIRONMENT", ui_environment_event);
        lv_obj_set_pos(item, 6, 0);
        item = ui_create_app_row(list, "\xEE\x9E\x88", &ui_font_iconfont28, lv_color_hex(0xD38EBF), "AI ASSISTANT", ui_ai_app_event);
        lv_obj_set_pos(item, 6, 56);
        item = ui_create_app_row(list, "\xEE\x98\x81", &ui_font_iconfont30, lv_color_hex(0xFFD27A), "RTC TIME", ui_rtc_app_event);
        lv_obj_set_pos(item, 6, 112);
        item = ui_create_app_row(list, "\xEE\xA2\x9B", &ui_font_iconfont28, lv_color_hex(0x8CECF5), "CALCULATOR", ui_calc_app_event);
        lv_obj_set_pos(item, 6, 168);
        item = ui_create_app_row(list, LV_SYMBOL_LOOP, &lv_font_montserrat_18, lv_color_hex(0x77E8EF), "TIMER", ui_stopwatch_app_event);
        lv_obj_set_pos(item, 6, 224);
        item = ui_create_app_row(list, "\xEE\x98\x80", &ui_font_iconfont30, lv_color_hex(0x8CECF5), "SETTINGS", ui_settings_event);
        lv_obj_set_pos(item, 6, 280);
    }

    ui_CalcPage = lv_obj_create(NULL);
    ui_style_screen(ui_CalcPage);
    {
        lv_obj_t * back = ui_create_action_button(ui_CalcPage, "<", 34, 28, ui_calc_back_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_CalcPage, "CALC", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        section = ui_create_label(ui_CalcPage, "BASIC / FOUR OPERATIONS", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 48);

        ui_calc_display = ui_create_card(ui_CalcPage, 14, 70, 212, 38);
        lv_obj_set_style_bg_color(ui_calc_display, lv_color_hex(0x09162C), LV_PART_MAIN);
        lv_obj_set_style_border_color(ui_calc_display, lv_color_hex(0x5F7FA7), LV_PART_MAIN);
        {
            lv_obj_t * value = ui_create_label(ui_calc_display, "0", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
            lv_obj_align(value, LV_ALIGN_RIGHT_MID, -10, 0);
            ui_calc_display = value;
        }

        {
            static const char * keys[20] = {
                "C", "DEL", "/", "*",
                "7", "8", "9", "-",
                "4", "5", "6", "+",
                "1", "2", "3", "=",
                "+/-", "0", ".", ""
            };
            int row;
            int col;
            for(row = 0; row < 5; row++) {
                for(col = 0; col < 4; col++) {
                    const char * key = keys[row * 4 + col];
                    lv_obj_t * button;
                    if(key[0] == '\0') continue;
                    button = ui_create_action_button(ui_CalcPage, key, 50, 30, ui_calc_button_event);
                    lv_obj_set_pos(button, 12 + col * 55, 116 + row * 32);
                    lv_obj_set_user_data(button, (void *)key);
                    if(strcmp(key, "=") == 0) {
                        lv_obj_set_style_bg_color(button, lv_color_hex(0x1A5A73), LV_PART_MAIN);
                        lv_obj_set_style_border_color(button, lv_color_hex(0x77E8EF), LV_PART_MAIN);
                    } else if(strcmp(key, "C") == 0 || strcmp(key, "DEL") == 0) {
                        lv_obj_set_style_bg_color(button, lv_color_hex(0x3A244A), LV_PART_MAIN);
                    }
                }
            }
        }
    }

    ui_StopwatchPage = lv_obj_create(NULL);
    ui_style_screen(ui_StopwatchPage);
    {
        lv_obj_t * back = ui_create_action_button(ui_StopwatchPage, "<", 34, 28, ui_calc_back_event);
        lv_obj_t * stopwatch = ui_create_action_button(ui_StopwatchPage, "SW", 64, 28, ui_timer_mode_event);
        lv_obj_t * countdown = ui_create_action_button(ui_StopwatchPage, "COUNT", 64, 28, ui_timer_mode_event);
        lv_obj_t * alarm = ui_create_action_button(ui_StopwatchPage, "ALARM", 64, 28, ui_timer_mode_event);
        lv_obj_t * control;
        lv_obj_t * reset;
        lv_obj_t * adjust_up;
        lv_obj_t * adjust_down;
        lv_obj_set_pos(back, 14, 14);
        lv_obj_set_pos(stopwatch, 12, 52);
        lv_obj_set_pos(countdown, 88, 52);
        lv_obj_set_pos(alarm, 164, 52);
        lv_obj_set_user_data(stopwatch, (void *)(uintptr_t)0U);
        lv_obj_set_user_data(countdown, (void *)(uintptr_t)1U);
        lv_obj_set_user_data(alarm, (void *)(uintptr_t)2U);
        title = ui_create_label(ui_StopwatchPage, "TIMER", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        ui_timer_mode_label = ui_create_label(ui_StopwatchPage, "STOPWATCH", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(ui_timer_mode_label, LV_ALIGN_TOP_MID, 0, 88);
        ui_stopwatch_display = ui_create_label(ui_StopwatchPage, "00:00.0", lv_color_hex(0xFFF6EF), &lv_font_montserrat_32);
        lv_obj_align(ui_stopwatch_display, LV_ALIGN_TOP_MID, 0, 104);
        ui_timer_status_label = ui_create_label(ui_StopwatchPage, "PAUSED", lv_color_hex(0xE1D4E8), &lv_font_montserrat_14);
        lv_obj_align(ui_timer_status_label, LV_ALIGN_TOP_MID, 0, 148);
        control = ui_create_action_button(ui_StopwatchPage, "START", 92, 34, ui_timer_control_event);
        reset = ui_create_action_button(ui_StopwatchPage, "RESET", 72, 34, ui_timer_reset_event);
        adjust_up = ui_create_action_button(ui_StopwatchPage, "+1 MIN", 72, 30, ui_timer_adjust_event);
        adjust_down = ui_create_action_button(ui_StopwatchPage, "-10 SEC", 72, 30, ui_timer_adjust_event);
        lv_obj_set_pos(control, 24, 178);
        lv_obj_set_pos(reset, 144, 178);
        lv_obj_set_pos(adjust_up, 30, 220);
        lv_obj_set_pos(adjust_down, 138, 220);
        lv_obj_set_user_data(adjust_up, (void *)(intptr_t)60);
        lv_obj_set_user_data(adjust_down, (void *)(intptr_t)-10);
        ui_timer_control_button = control;
        ui_timer_adjust_up_button = adjust_up;
        ui_timer_adjust_down_button = adjust_down;
        ui_stopwatch_timer = lv_timer_create(ui_stopwatch_timer_cb, 100, NULL);
        lv_timer_pause(ui_stopwatch_timer);
        ui_timer_refresh();
    }

    ui_RtcPage = lv_obj_create(NULL);
    ui_style_screen(ui_RtcPage);
    {
        lv_obj_t * back = ui_create_action_button(ui_RtcPage, "<", 34, 28, ui_time_back_event);
        lv_obj_t * hour_up = ui_create_action_button(ui_RtcPage, "+ HOUR", 82, 32, ui_time_adjust_event);
        lv_obj_t * hour_down = ui_create_action_button(ui_RtcPage, "- HOUR", 82, 32, ui_time_adjust_event);
        lv_obj_t * min_up = ui_create_action_button(ui_RtcPage, "+ MIN", 82, 32, ui_time_adjust_event);
        lv_obj_t * min_down = ui_create_action_button(ui_RtcPage, "- MIN", 82, 32, ui_time_adjust_event);
        lv_obj_t * apply = ui_create_action_button(ui_RtcPage, "APPLY", 92, 34, ui_time_apply_event);
        lv_obj_set_pos(back, 14, 14);
        title = ui_create_label(ui_RtcPage, "TIME SET", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
        ui_time_clock_label = ui_create_label(ui_RtcPage, "09:41:00", lv_color_hex(0xFFF6EF), &lv_font_montserrat_32);
        lv_obj_align(ui_time_clock_label, LV_ALIGN_TOP_MID, 0, 74);
        ui_time_status_label = ui_create_label(ui_RtcPage, "MANUAL CLOCK / RAM ONLY", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(ui_time_status_label, LV_ALIGN_TOP_MID, 0, 124);
        lv_obj_set_pos(hour_up, 16, 158);
        lv_obj_set_pos(hour_down, 16, 198);
        lv_obj_set_pos(min_up, 142, 158);
        lv_obj_set_pos(min_down, 142, 198);
        lv_obj_set_user_data(hour_up, (void *)(intptr_t)3600);
        lv_obj_set_user_data(hour_down, (void *)(intptr_t)-3600);
        lv_obj_set_user_data(min_up, (void *)(intptr_t)60);
        lv_obj_set_user_data(min_down, (void *)(intptr_t)-60);
        lv_obj_set_pos(apply, 74, 238);
        ui_time_timer = lv_timer_create(ui_time_timer_cb, 1000, NULL);
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
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 218);

        section = ui_create_label(ui_SettingsPage, "SCREEN TIMEOUT", lv_color_hex(0x77E8EF), &lv_font_montserrat_14);
        lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 236);
        {
            lv_obj_t * timeout_down = ui_create_action_button(ui_SettingsPage, "-15S", 56, 26, ui_timeout_adjust_event);
            lv_obj_t * timeout_up = ui_create_action_button(ui_SettingsPage, "+15S", 56, 26, ui_timeout_adjust_event);
            lv_obj_set_pos(timeout_down, 24, 250);
            lv_obj_set_pos(timeout_up, 160, 250);
            lv_obj_set_user_data(timeout_down, (void *)(intptr_t)-15);
            lv_obj_set_user_data(timeout_up, (void *)(intptr_t)15);
            ui_timeout_value = ui_create_label(ui_SettingsPage, "30s", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
            lv_obj_align(ui_timeout_value, LV_ALIGN_TOP_MID, 0, 250);
        }
    }

    ui_quick_panel = lv_obj_create(ui_HomePage);
    lv_obj_set_pos(ui_quick_panel, 0, -160);
    lv_obj_set_size(ui_quick_panel, 240, 160);
    lv_obj_clear_flag(ui_quick_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_quick_panel, lv_color_hex(0x0D1832), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_quick_panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_quick_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_quick_panel, 0, LV_PART_MAIN);
    {
        lv_obj_t * wifi = ui_create_quick_toggle(ui_quick_panel, LV_SYMBOL_WIFI, "WIFI", ui_quick_wifi_event);
        lv_obj_t * bluetooth = ui_create_quick_toggle(ui_quick_panel, LV_SYMBOL_BLUETOOTH, "BLE", ui_quick_bluetooth_event);
        lv_obj_t * screen = ui_create_quick_toggle(ui_quick_panel, LV_SYMBOL_EYE_CLOSE, "OFF", ui_quick_lock_event);
        lv_obj_t * power = ui_create_quick_toggle(ui_quick_panel, LV_SYMBOL_POWER, "PWR", ui_quick_power_event);
        lv_obj_set_pos(wifi, 12, 12);
        lv_obj_set_pos(bluetooth, 136, 12);
        lv_obj_set_pos(screen, 12, 74);
        lv_obj_set_pos(power, 136, 74);
        ui_quick_brightness_slider = lv_slider_create(ui_quick_panel);
        ui_style_brightness_slider(ui_quick_brightness_slider, 64, 138, 138, 18);
        {
            lv_obj_t * sun = lv_label_create(ui_quick_panel);
            lv_label_set_text(sun, "*");
            lv_obj_align_to(sun, ui_quick_brightness_slider, LV_ALIGN_OUT_RIGHT_MID, 6, -3);
            lv_obj_set_style_text_color(sun, lv_color_hex(0xFFD27A), LV_PART_MAIN);
            lv_obj_set_style_text_font(sun, &lv_font_montserrat_24, LV_PART_MAIN);
        }
        ui_quick_brightness_value = ui_create_label(ui_quick_panel, "20%", lv_color_hex(0xFFF6EF), &lv_font_montserrat_18);
        lv_obj_set_pos(ui_quick_brightness_value, 12, 136);
        lv_label_set_text_fmt(ui_quick_brightness_value, "%d%%", (int)LCD_Get_Light());
    }
}

void ui_set_sensor_values(const char * temperature, const char * humidity)
{
    if(ui_temperature_label && temperature) lv_label_set_text(ui_temperature_label, temperature);
    if(ui_humidity_label && humidity) lv_label_set_text(ui_humidity_label, humidity);
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
    if(ui_quick_brightness_value) lv_label_set_text_fmt(ui_quick_brightness_value, "%d%%", (int)percent);

}

void ui_HomePage_screen_destroy(void)
{
    if(ui_SettingsPage) lv_obj_del(ui_SettingsPage);
    if(ui_stopwatch_timer) lv_timer_del(ui_stopwatch_timer);
    if(ui_time_timer) lv_timer_del(ui_time_timer);
    if(ui_CalcPage) lv_obj_del(ui_CalcPage);
    if(ui_NoticePage) lv_obj_del(ui_NoticePage);
    if(ui_AIPage) lv_obj_del(ui_AIPage);
    if(ui_AppsPage) lv_obj_del(ui_AppsPage);
    if(ui_HomePage) lv_obj_del(ui_HomePage);

    ui_HomePage = NULL;
    ui_AppsPage = NULL;
    ui_AIPage = NULL;
    ui_CalcPage = NULL;
    ui_StopwatchPage = NULL;
    ui_RtcPage = NULL;
    ui_stopwatch_timer = NULL;
    ui_time_timer = NULL;
    ui_NoticePage = NULL;
    ui_SettingsPage = NULL;
    ui_connection_label = NULL;
    ui_status_label = NULL;
    ui_detail_label = NULL;
    ui_temperature_label = NULL;
    ui_humidity_label = NULL;
    ui_app_status_label = NULL;
    ui_timeout_value = NULL;
    ui_brightness_slider = NULL;
    ui_brightness_value = NULL;
    ui_quick_panel = NULL;
    ui_quick_brightness_slider = NULL;
    ui_quick_brightness_value = NULL;
    ui_quick_panel_visible = false;
    ui_screen_off = false;
}
