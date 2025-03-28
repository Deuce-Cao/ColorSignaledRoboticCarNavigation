#include <stdint.h>
#include <stddef.h>

enum class DetectedColor
{
    NONE,
    RED,
    GREEN,
    BLUE
};

struct ColorThreshold
{
    uint16_t min_red; // 0-31
    uint16_t max_red;
    uint16_t min_green; // 0-63
    uint16_t max_green;
    uint16_t min_blue; // 0-31
    uint16_t max_blue;
    uint16_t confidence; // Min pixels
};

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class ColorDetector
{
public:
    ColorDetector();
    void setThresholds(const ColorThreshold &red,
                       const ColorThreshold &green,
                       const ColorThreshold &blue);
    // Main detection function
    DetectedColor detect(const uint8_t *frame,
                         size_t width,
                         size_t height);
    // Function to get RGB values of the center pixel
    RGB getCenterPixelRGB(const uint8_t *frame,
                          size_t width,
                          size_t height) const;
    // Function to get average RGB values of the image
    RGB getAverageRGB(const uint8_t *frame, size_t width, size_t height) const;

private:
    ColorThreshold red_thresh;
    ColorThreshold green_thresh;
    ColorThreshold blue_thresh;

    // RGB565 conversion utilities
    RGB getRGB565Components(uint16_t rgb565) const;

    bool checkThreshold(uint8_t r,
                        uint8_t g,
                        uint8_t b,
                        const ColorThreshold &thresh) const;

    // Analysis region (center 50% of image)
    static constexpr float ROI_X_START = 0.4f;
    static constexpr float ROI_X_END = 0.6f;
    static constexpr float ROI_Y_START = 0.4f;
    static constexpr float ROI_Y_END = 0.6f;
};