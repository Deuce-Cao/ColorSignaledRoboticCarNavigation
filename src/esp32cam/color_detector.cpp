#include "color_detector.H"
#include <Arduino.H>
#include <ESP_Color.H>

struct Thresholds
{
    // HSV thresholds
    float saturation = 0.3f; // Saturation threshold
    float value = 0.3f;      // Value threshold

    // Hue ranges for specific colors
    int redMin = 330;    // Minimum hue for red
    int redMax = 20;     // Maximum hue for red
    int greenMin = 90;   // Minimum hue for green
    int greenMax = 190;  // Maximum hue for green
    int blueMin = 220;   // Minimum hue for blue
    int blueMax = 260;   // Maximum hue for blue

    // Pixel count thresholds
    int validPixelCount = 2000; // Minimum valid pixels in ROI
    int redPixelCount = 3000;   // Minimum red pixels for detection
    int greenPixelCount = 2000; // Minimum green pixels for detection
    int bluePixelCount = 3000;  // Minimum blue pixels for detection
};

// Create a global instance of the struct
Thresholds thresholds;

// Default thresholds (calibrate these)
ColorDetector::ColorDetector()
{
}

void ColorDetector::initialize() {};

RGB ColorDetector::getRGB565Components(uint16_t rgb565) const
{
    RGB rgb;
    // Extract 5-6-5 components
    rgb.r = ((rgb565 >> 11) & 0x1F) * 255 / 31; // 5 bits
    rgb.g = ((rgb565 >> 5) & 0x3F) * 255 / 63;  // 6 bits
    rgb.b = (rgb565 & 0x1F) * 255 / 31;         // 5 bits

    return rgb;
}

// detect function, check rgb channels separately, for each channel, if the (value > 200) count is more than 2000, then return this color, if not return NONE
DetectedColor
ColorDetector::detect(const uint8_t *frame,
                      size_t length,
                      size_t width,
                      size_t height)
{
    const uint16_t *pixels = reinterpret_cast<const uint16_t *>(frame);
    // Check the total length of the frame matches the expected size
    if (width * height * 2 != length)
    {
        Serial.println("Frame size mismatch!");
        Serial.printf("Expected: %zu, Actual: %zu\n", width * height * 2, length);
        return DetectedColor::NONE;
    }
    //  create a list of 50 pixels of RGB565 format data to verify the data
    // uint16_t pixels[49] = {
    //     0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000,
    //     0xF81F, 0x07FF, 0xFFE0, 0xF800, 0x07E0,
    //     0x001F, 0xFFFF, 0x0000, 0xF81F, 0x07FF,
    //     0xFFE0, 0xF800, 0x07E0, 0x001F, 0xFFFF,
    //     0x0000, 0xF81F, 0x07FF, 0xFFE0, 0xF800,
    //     0x07E0, 0x001F, 0xFFFF, 0x0000, 0xF81F,
    //     0x07FF, 0xFFE0, 0xF800, 0x07E0, 0x001F,
    //     0xFFFF, 0x0000, 0xF81F, 0x07FF, 0xFFE0,
    //     0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000,
    //     0xF81F, 0x07FF, 0xFFE0, 0xF800
    // };
    // // expected rgb output:
    // // 0xF800 = 255, 0, 0
    // // 0x07E0 = 0, 255, 0
    // // 0x001F = 0, 0, 255
    // // 0xFFFF = 255, 255, 255
    // // 0x0000 = 0, 0, 0
    // // 0xF81F = 255, 0, 255
    // // 0x07FF = 0, 255, 255
    // // 0xFFE0 = 255, 255, 0
    size_t red_count = 0, green_count = 0, blue_count = 0;

    int valid_count = 0; // Count of valid pixels in the ROI
    int total_H = 0;     // Total hue value

    // Serial.println("RAW_START");
    // for (size_t i = 0; i < length; i++)
    // {
    //     Serial.printf("%02X", frame[i]);
    //     if ((i + 1) % 16 == 0)
    //         Serial.println(); // Print 16 bytes per line
    // }
    // Serial.println();
    // Serial.println("RAW_END");

    // size_t pixel_count = 0;

    // Iterate through the ROI
    // Serial.println("RGB_START");
    for (size_t y = ROI_Y_START * height; y < ROI_Y_END * height; y++)
    {
        for (size_t x = ROI_X_START * width; x < ROI_X_END * width; x++)
        {
            // Extract pixel data from the frame buffer
            uint16_t pixel = (frame[(y * width + x) * 2] << 8) | frame[(y * width + x) * 2 + 1]; // RGB565 format
            // uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
            // uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
            // uint8_t b = (pixel & 0x1F) * 255 / 31;

            // Serial.printf("%d, %d, %d\n", r, g, b);
            ESP_Color::Color color(pixel);
            // RGB rgb = {color.R_Byte(), color.G_Byte(), color.B_Byte()};
            // Serial.printf("%d, %d, %d\n", rgb.r, rgb.g, rgb.b);
            // pixel_count++;
            auto hsv = color.ToHsv();
            hsv.H = hsv.H * 360;              // Convert to 0-360 range
            if (hsv.S > thresholds.saturation && hsv.V > thresholds.value) // Check saturation and value
            {
                valid_count++;
                total_H += hsv.H; // Accumulate hue value
                // Check color ranges
                if (hsv.H < thresholds.redMax || hsv.H > thresholds.redMin) // Red range
                    red_count++;
                if (hsv.H > thresholds.greenMin && hsv.H < thresholds.greenMax) // Green range
                    green_count++;
                if (hsv.H > thresholds.blueMin && hsv.H < thresholds.blueMax) // Blue range
                    blue_count++;
            }
        }
    }
    // Serial.println("RGB_END");
    // Serial.printf("Total Pixels Sent: %zu\n", pixel_count);
    // Serial.println("");
    // Calculate average hue
    if (valid_count > 0)
    {
        int avg_H = total_H / valid_count;
        Serial.printf("Average Hue: %d\n", avg_H);
    }
    Serial.printf("Valid pixels in ROI: %d\n", valid_count);
    Serial.printf("Red: %zu, Green: %zu, Blue: %zu\n", red_count, green_count, blue_count);
    // Check counts against thresholds
    if (valid_count < thresholds.validPixelCount)
        return DetectedColor::NONE; // No valid pixels in ROI
    if (red_count > thresholds.redPixelCount)
        return DetectedColor::RED;
    else if (blue_count > thresholds.bluePixelCount)
        return DetectedColor::BLUE;
    else if (green_count > thresholds.greenPixelCount)
        return DetectedColor::GREEN;
    return DetectedColor::NONE;
}