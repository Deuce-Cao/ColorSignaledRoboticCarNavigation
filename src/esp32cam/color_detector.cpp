#include "color_detector.h"
#include <Arduino.h>
// Default thresholds (calibrate these)
ColorDetector::ColorDetector()
{
    // Red: High R, low G/B
    red_thresh = {150, 255, 0, 100, 0, 100, 50};

    // Green: High G, low R/B
    green_thresh = {0, 100, 150, 255, 0, 100, 50};

    // Blue: High B, low R/G
    blue_thresh = {0, 100, 0, 100, 150, 255, 50};
}

void ColorDetector::setThresholds(const ColorThreshold &red,
                                  const ColorThreshold &green,
                                  const ColorThreshold &blue)
{
    red_thresh = red;
    green_thresh = green;
    blue_thresh = blue;
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

RGB ColorDetector::getCenterPixelRGB(const uint8_t *frame,
                                     size_t width,
                                     size_t height) const
{
    const uint16_t *pixels = reinterpret_cast<const uint16_t *>(frame);
    size_t center_x = width / 2;
    size_t center_y = height / 2;
    uint16_t center_pixel = pixels[center_y * width + center_x];
    return getRGB565Components(center_pixel);
}

bool ColorDetector::checkThreshold(uint8_t r,
                                   uint8_t g,
                                   uint8_t b,
                                   const ColorThreshold &thresh) const
{
    return (r >= thresh.min_red) && (r <= thresh.max_red) &&
           (g >= thresh.min_green) && (g <= thresh.max_green) &&
           (b >= thresh.min_blue) && (b <= thresh.max_blue);
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

            if (checkThreshold(rgb.r, rgb.g, rgb.b, red_thresh))
                red_count++;
            if (checkThreshold(rgb.r, rgb.g, rgb.b, green_thresh))
                green_count++;
            if (checkThreshold(rgb.r, rgb.g, rgb.b, blue_thresh))
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
    if (red_percent >= red_thresh.confidence &&
        red_percent > green_percent &&
        red_percent > blue_percent)
    {
        return DetectedColor::RED;
    }

    if (green_percent >= green_thresh.confidence &&
        green_percent > red_percent &&
        green_percent > blue_percent)
    {
        return DetectedColor::GREEN;
    }

    if (blue_percent >= blue_thresh.confidence &&
        blue_percent > red_percent &&
        blue_percent > green_percent)
    {
        return DetectedColor::BLUE;
    }

    return DetectedColor::NONE;
}