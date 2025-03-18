#include "color_detector.h"
#include <Arduino.h>
// Default thresholds (calibrate these)
ColorDetector::ColorDetector()
{
}

RGB ColorDetector::getRGB565Components(uint16_t rgb565) const
{
    RGB rgb;
    // Extract 5-6-5 components
    rgb.r = ((rgb565 >> 11) & 0x1F) * 255 / 31; // 5 bits
    rgb.g = ((rgb565 >> 5) & 0x3F) * 255 / 63;  // 6 bits
    rgb.b = (rgb565 & 0x1F) * 255 / 31;         // 5 bits

    return rgb;
}

int ColorDetector::checkThreshold(const HSV &hsv) const
{
    const float h = hsv.h;
    const float s = hsv.s;
    const float v = hsv.v;

    // Check for black/white/gray
    if (s <= 0.3 || v <= 0.3)
    {
        // Red
        if (h >= 0 && h <= 40 || h >= 320 && h <= 360)
            return 1;
        // Green
        else if (h >= 50 && h <= 150)
            return 2;
        // Blue
        else if (h >= 160 && h <= 300)
            return 3;
    }
    return 0;
}

// Helper function to convert RGB to HSV
HSV ColorDetector::rgbToHsv(const RGB &rgb) const
{
    // Normalize RGB values to [0, 1]
    const float r = rgb.r / 255.0f;
    const float g = rgb.g / 255.0f;
    const float b = rgb.b / 255.0f;

    // Find value and chroma
    const float max = fmaxf(fmaxf(r, g), b);
    const float min = fminf(fminf(r, g), b);
    const float chroma = max - min;

    HSV hsv;
    hsv.v = max;

    // Handle achromatic case (gray)
    if (chroma < 1e-5f)
    {
        hsv.h = 0.0f;
        hsv.s = 0.0f;
        return hsv;
    }

    // Calculate saturation
    hsv.s = chroma / max;

    // Calculate hue
    if (max == r)
    {
        hsv.h = 60.0f * fmodf((g - b) / chroma + 6.0f, 6.0f);
    }
    else if (max == g)
    {
        hsv.h = 60.0f * ((b - r) / chroma + 2.0f);
    }
    else
    { // max == b
        hsv.h = 60.0f * ((r - g) / chroma + 4.0f);
    }

    // Ensure hue is positive
    if (hsv.h < 0.0f)
    {
        hsv.h += 360.0f;
    }
    Serial.printf("H: %.2f\n", hsv.h);
    return hsv;
}

DetectedColor ColorDetector::detect(const uint8_t *frame,
                                    size_t width,
                                    size_t height)
{
    const uint16_t *pixels = reinterpret_cast<const uint16_t *>(frame);
    uint32_t red_count = 0, green_count = 0, blue_count = 0;

    uint32_t total_pixels = 0;

    // Calculate ROI boundaries
    const size_t x_start = width * ROI_X_START;
    const size_t x_end = width * ROI_X_END;
    const size_t y_start = height * ROI_Y_START;
    const size_t y_end = height * ROI_Y_END;

    // Analyze region of interest
    for (size_t y = y_start; y < y_end; y++)
    {
        for (size_t x = x_start; x < x_end; x++)
        {
            uint16_t pixel = pixels[y * width + x];
            RGB rgb = getRGB565Components(pixel);

            // Convert RGB to HSV
            HSV hsv = rgbToHsv(rgb);

            // Check thresholds for each color
            if (checkThreshold(hsv) == 1)
                red_count++;
            else if (checkThreshold(hsv) == 2)
                green_count++;
            else if (checkThreshold(hsv) == 3)
                blue_count++;

            total_pixels++;
        }
    }

    // Calculate percentages
    const float red_percent = (red_count * 100.0f) / total_pixels;
    const float green_percent = (green_count * 100.0f) / total_pixels;
    const float blue_percent = (blue_count * 100.0f) / total_pixels;

    Serial.printf("Red: %.2f%%, Green: %.2f%%, Blue: %.2f%%\n", red_percent, green_percent, blue_percent);

    // Determine dominant color
    if (red_percent > 50 && red_percent > green_percent && red_percent > blue_percent)
        return DetectedColor::RED;
    else if (green_percent > 50 && green_percent > red_percent && green_percent > blue_percent)
        return DetectedColor::GREEN;
    else if (blue_percent > 50 && blue_percent > red_percent && blue_percent > green_percent)
        return DetectedColor::BLUE;

    return DetectedColor::NONE;
}