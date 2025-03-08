#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "bsp.h"
#include "bsp_color.h"
#include "lvgl.h"
#include "bsp_device.h"
#include <stddef.h>
#include "esp_lvgl_port.h"
#include "bsp/disp_mipi_dsi.h"

#include <esp_heap_caps.h>
#include <string.h>

static void show_loading_screen(void)
{
    lvgl_port_lock(0);
    lv_obj_t *loading_label = lv_label_create(lv_screen_active());
    lv_label_set_text(loading_label, "HELLO LVGL!");
    lv_obj_center(loading_label);
    lvgl_port_unlock();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second
        lvgl_port_lock(0);
        lv_label_set_text(loading_label, "LOADING!");
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second
        lvgl_port_lock(0);
        lv_label_set_text(loading_label, "HELLO LVGL!");
        lvgl_port_unlock();
    }
}

// Helper function for 16-bit memset
static inline void *memset16(void *dst, uint16_t val, size_t count)
{
    uint16_t *tmp = (uint16_t *)dst;
    for (size_t i = 0; i < count; i++)
    {
        tmp[i] = val;
    }
    return dst;
}
static const uint32_t dev_id = 1;
static void show_colors()
{
    // Allocate buffer in PSRAM
    uint16_t *red_screen = heap_caps_malloc(800 * 480 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (red_screen == NULL)
    {
        ESP_LOGE("display_init", "Failed to allocate memory for red_screen");
        return;
    }

    // Initialize buffer to 0x0 (black)
    // memset(red_screen, 0b00111000, 800 * 480 * sizeof(uint16_t)); // RIGHT BLUE
    // memset(red_screen, 0b11000100, 800 * 240 * sizeof(uint16_t)); // LEFT YELLOW (0b11000111) gives greenish yellow

    // In your display code:
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xffffff), 800 * 480); // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xffff00), 600 * 480); // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xff0000), 400 * 480); // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0x00ff00), 200 * 480); // Green 0-10
    // memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0x0000ff), 200 * 480);   // Green 0-10
    // Update display with the buffer
    bsp_disp_update(1, 0, red_screen);

    // Free the allocated memory
    free(red_screen);
}

static void setup_lvgl()
{

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t err = lvgl_port_init(&lvgl_cfg);

    const int DISP_WIDTH = 800;            // TODO no hardcoding
    const int DISP_HEIGHT = 480;           // TODO no hardcoding
    const int BUFF_SIZE = DISP_WIDTH * 50; // TODO no hardcoding
    ptrdiff_t idx = bsp_find_device(dev_id);
    bsp_device_t *dev = devices[idx];
    bsp_disp_dsi_t *disp = dev->disp_aux[0];

    /* Add LCD screen */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = disp->io_handle,
        .panel_handle = disp->disp_handle,
        .buffer_size = BUFF_SIZE,
        .double_buffer = false,
        .hres = DISP_HEIGHT,
        .vres = DISP_WIDTH,
        .monochrome = false,
        // .mipi_dsi = true, // Does not exist anymore, it's implied by using lvgl_port_add_disp_dsi function
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = false,
            .buff_spiram = false,
            .sw_rotate = true,
        }};
    const lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags = {
            .avoid_tearing = false,
        }};
    // Add error checking
    lv_disp_t *disp_handle = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);
    if (disp_handle == NULL)
    {
        ESP_LOGE("TAG", "Failed to add display to LVGL");
    }
    else
    {
        ESP_LOGI("TAG", "Display initialized");
    }
}

static void display_init_task(void *pvParameters)
{
    bsp_disp_backlight(dev_id, 0, 2);
    setup_lvgl();
    ESP_LOGI("TAG", "setup_lvgl  DONE");
    show_loading_screen();
    // show_colors();
    ESP_LOGI("TAG", "TASK delete START");
    vTaskDelete(NULL); // Delete the task after completion
    ESP_LOGI("TAG", "TASK deleted");
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

    bsp_init();

    xTaskCreate(display_init_task, "display_init", 4096, NULL, 5, NULL);
}
