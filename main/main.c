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

#include <esp_heap_caps.h>
#include <string.h>

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

static void display_init_task(void *pvParameters)
{
    bsp_disp_backlight(1, 0, 2);

    // Allocate buffer in PSRAM
    uint16_t *red_screen = heap_caps_malloc(800 * 480 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (red_screen == NULL)
    {
        ESP_LOGE("display_init", "Failed to allocate memory for red_screen");
        vTaskDelete(NULL);
        return;
    }

    // Initialize buffer to 0x0 (black)
    // memset(red_screen, 0b00111000, 800 * 480 * sizeof(uint16_t)); // RIGHT BLUE
    // memset(red_screen, 0b11000100, 800 * 240 * sizeof(uint16_t)); // LEFT YELLOW (0b11000111) gives greenish yellow

    // In your display code:
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xffffff), 800 * 480);   // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xffff00), 600 * 480);   // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0xff0000), 400 * 480);   // Green 0-10
    memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0x00ff00), 200 * 480);   // Green 0-10
    // memset16(red_screen, bsp_rgb_to_col(BSP_PIXFMT_16_565RGB, 0x0000ff), 200 * 480);   // Green 0-10
    // Update display with the buffer
    bsp_disp_update(1, 0, red_screen);

    // Free the allocated memory
    free(red_screen);

    vTaskDelete(NULL); // Delete the task after completion
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
