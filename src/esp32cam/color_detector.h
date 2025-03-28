#include <stdint.h>
#include <stddef.h>

enum class DetectedColor
{
    NONE,
    RED,
    GREEN,
    BLUE
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
    void initialize();
    // Main detection function
    DetectedColor detect(const uint8_t *frame,
                         size_t width,
                         size_t height);


private:
    // RGB565 conversion utilities
    RGB getRGB565Components(uint16_t rgb565) const;

    // Analysis region (center 50% of image)
    static constexpr float ROI_X_START = 0.25f;
    static constexpr float ROI_X_END = 0.75f;
    static constexpr float ROI_Y_START = 0.25f;
    static constexpr float ROI_Y_END = 0.75f;
};