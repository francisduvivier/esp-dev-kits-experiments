#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"

static lv_style_t style_label;

static void add_style(lv_obj_t *label, lv_obj_t *button)
{
    lv_style_init(&style_label);
    lv_style_set_bg_color(&style_label, lv_color_hex(0x000000));
    lv_style_set_text_color(&style_label, lv_color_hex(0xff0000));
    lv_obj_add_style(label, &style_label, LV_PART_MAIN);
    lv_obj_add_style(button, &style_label, LV_PART_MAIN);
}

// Initialize the application list UI
static void init_app_list_ui(void)
{
    /*Create a container with ROW flex direction*/
    lv_obj_t *cont_col = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont_col, LV_PCT(80), LV_PCT(100));
    lv_obj_align(cont_col, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);

    uint32_t i;
    for (i = 0; i < 10; i++)
    {

        lv_obj_t *app_card = lv_obj_create(cont_col);
        ;
        lv_obj_set_size(app_card, LV_PCT(100), 80);
        lv_obj_set_flex_flow(app_card, LV_FLEX_FLOW_ROW); // Set row-based flex layout
        lv_obj_clear_flag(app_card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *app_name_label = lv_label_create(app_card);
        lv_obj_t *install_button = lv_button_create(app_card);
        lv_obj_t *install_label = lv_label_create(install_button);

        /*Add items to the row*/
        lv_obj_set_size(app_name_label, LV_PCT(50), LV_PCT(100));
        lv_label_set_text_fmt(app_name_label, "App: %" LV_PRId32 "", i);

        lv_obj_set_size(install_button, LV_PCT(50), LV_PCT(100));
        lv_label_set_text_fmt(install_label, "install: %" LV_PRId32 "", i);
        add_style(install_label, install_button);
        lv_obj_center(install_label);
    }
}

static void show_app_list(void)
{
    bsp_display_lock(0);
    init_app_list_ui();
    bsp_display_unlock();
}

static void show_loading_screen(void)
{
    bsp_display_lock(0);
    lv_obj_t *loading_label = lv_label_create(lv_screen_active());
    lv_label_set_text(loading_label, "loading app data...");
    lv_obj_center(loading_label);
    bsp_display_unlock();
}

static void update_app_list()
{
    vTaskDelay(pdMS_TO_TICKS(1 * 1000));
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        }};
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();
    bsp_display_brightness_set(5);
    show_loading_screen();
    update_app_list();
    show_app_list();
}
