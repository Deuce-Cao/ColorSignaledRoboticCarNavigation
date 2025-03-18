#include <stdint.h>
#include <stddef.h>

enum class DetectedColor
{
    RED,
    GREEN,
    BLUE,
    NONE
};

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct HSV
{
    float h; // [0, 360) degrees
    float s; // [0, 1]
    float v; // [0, 1]
};

class ColorDetector
{
public:
    ColorDetector();
    // Main detection function
    DetectedColor detect(const uint8_t *frame,
                         size_t width,
                         size_t height);


private:

    // RGB565 conversion utilities
    RGB getRGB565Components(uint16_t rgb565) const;

    int checkThreshold(const HSV &hsv) const;

    HSV rgbToHsv(const RGB &rgb) const;

    // Analysis region (center 40% of image)
    static constexpr float ROI_X_START = 0.3f;
    static constexpr float ROI_X_END = 0.7f;
    static constexpr float ROI_Y_START = 0.3f;
    static constexpr float ROI_Y_END = 0.7f;
};