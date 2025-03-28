#include "color_detector.H"
#include <Arduino.H>
#include <ESP_Color.H>

// Default thresholds (calibrate these)
ColorDetector::ColorDetector()
{
}

void ColorDetector::initialize(){};

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
                          size_t width,
                          size_t height)
{
    const uint16_t *pixels = reinterpret_cast<const uint16_t *>(frame);
    size_t red_count = 0, green_count = 0, blue_count = 0;

    // Calculate ROI boundaries
    const size_t x_start = width * ROI_X_START;
    const size_t x_end = width * ROI_X_END;
    const size_t y_start = height * ROI_Y_START;
    const size_t y_end = height * ROI_Y_END;

    // Iterate through the ROI
    for (size_t y = y_start; y < y_end; y++)
    {
        for (size_t x = x_start; x < x_end; x++)
        {
            uint16_t pixel = pixels[y * width + x];
            RGB rgb = getRGB565Components(pixel);
            ESP_Color::Color color(rgb.r, rgb.g, rgb.b);
            auto hsv = color.ToHsv();
            hsv.H = hsv.H * 360; // Convert to 0-360 range
            if (hsv.H < 30 | hsv.H > 330)
                red_count++;
            if (hsv.H > 120 & hsv.H < 190)
                green_count++;
            if (hsv.H > 200 & hsv.H < 260)
                blue_count++;
        }
    }

    // Check counts against thresholds
    if (red_count > 2000)
        return DetectedColor::RED;
    else if (blue_count > 2000)
        return DetectedColor::BLUE;
    else if (green_count > 2000)
        return DetectedColor::GREEN;
    return DetectedColor::NONE;

}