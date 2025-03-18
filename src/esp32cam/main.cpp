#include <Arduino.h>
#include "camera_module.h"
#include "color_detector.h"

#define TX_PIN 3
#define RX_PIN 40

ColorDetector detector;

void setup()
{
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 10000000;
    config.frame_size = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_RGB565; 
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        while (1)
            delay(1000);
    }

    sensor_t *s = esp_camera_sensor_get();
    Serial.printf("Sensor PID: 0x%x\n", s->id.PID);

    // Set color thresholds (calibrate these)
    detector.setThresholds(
        {150, 255, 0, 100, 0, 100, 50},
        {0, 100, 150, 255, 0, 100, 50},
        {0, 100, 0, 100, 150, 255, 50});
}
void loop()
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Failed to capture frame");
        return;
    }

    DetectedColor color = detector.detect(fb->buf, fb->width, fb->height);

    RGB center_rgb = detector.getCenterPixelRGB(fb->buf, fb->width, fb->height);
    Serial.printf("Center Pixel RGB: R=%d, G=%d, B=%d\n", center_rgb.r, center_rgb.g, center_rgb.b);

    if (color != DetectedColor::NONE)
    {
        const char *color_str = "";
        switch (color)
        {
        case DetectedColor::RED:
            color_str = "RED";
            break;
        case DetectedColor::GREEN:
            color_str = "GREEN";
            break;
        case DetectedColor::BLUE:
            color_str = "BLUE";
            break;
        }
        Serial2.println(color_str);
        Serial.printf("Detected: %s\n", color_str);
    }

    esp_camera_fb_return(fb);
    delay(3000); // 3s
}