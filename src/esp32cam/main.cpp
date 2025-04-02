#include <Arduino.h>
#include "camera_module.h"
#include "color_detector.h"

#define TX_PIN 3
#define RX_PIN 40

ColorDetector detector;

void sendImageToPC(camera_fb_t *fb)
{
    // Send metadata
    Serial.println("START");
    Serial.println(fb->width);  // Image width
    Serial.println(fb->height); // Image height
    Serial.println(fb->len);    // Image length in bytes

    // Send raw RGB565 data
    Serial.write(fb->buf, fb->len);

    // End marker
    Serial.println("END");
}

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
    config.frame_size = FRAMESIZE_QQVGA;
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
    s->set_brightness(s, 0);                 // -2 to 2
    s->set_contrast(s, 0);                   // -2 to 2
    s->set_saturation(s, 1);                 // -2 to 2
    s->set_special_effect(s, 0);             // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 0);                   // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
    s->set_wb_mode(s, 3);                    // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 0);              // 0 = disable , 1 = enable
    s->set_aec2(s, 1);                       // 0 = disable , 1 = enable
    s->set_ae_level(s, -2);                  // -2 to 2
    s->set_aec_value(s, 120);                // 0 to 1200
    s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
    s->set_agc_gain(s, 5);                   // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)2); // 0 to 6
    s->set_bpc(s, 1);                        // 0 = disable , 1 = enable
    s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
    s->set_lenc(s, 0);                       // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);                    // 0 = disable , 1 = enable
    s->set_vflip(s, 0);                      // 0 = disable , 1 = enable
    s->set_dcw(s, 1);                        // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);                   // 0 = disable , 1 = enable
    Serial.printf("Sensor PID: 0x%x\n", s->id.PID);

    // Set color thresholds (calibrate these)
    detector.initialize();
}
void loop()
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Failed to capture frame");
        return;
    }
    //sendImageToPC(fb);

    DetectedColor color = detector.detect(fb->buf, fb->len, fb->width, fb->height);

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
        Serial2.flush(); // Ensure the data is sent immediately
        Serial.printf("Detected color: %s\n", color_str);
    }

        esp_camera_fb_return(fb);
    delay(1000); // 3s
}