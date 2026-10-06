#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
namespace ss2vr {
struct OpaqueDimming {
    std::array<uint8_t, 256> channel{};
    bool passthrough = true, black = false;
};
inline OpaqueDimming makeOpaqueDimming(float visibility) {
    const float factor = std::isfinite(visibility) ? std::clamp(visibility, 0.f, 1.f) : 1.f;
    OpaqueDimming dimming;
    dimming.passthrough = factor == 1;
    dimming.black = factor == 0;
    if (!dimming.passthrough && !dimming.black)
        for (unsigned value = 0; value < dimming.channel.size(); ++value)
            dimming.channel[value] = uint8_t(std::lround(float(value) * factor));
    return dimming;
}
inline void dimOpaquePixels(uint8_t *pixels, size_t count, const OpaqueDimming &dimming) {
    if (dimming.black)
        std::fill(pixels, pixels + count * 4, uint8_t(0));
    for (size_t i = 0; i < count; ++i) {
        if (!dimming.passthrough && !dimming.black)
            for (unsigned channel = 0; channel < 3; ++channel)
                pixels[4 * i + channel] = dimming.channel[pixels[4 * i + channel]];
        pixels[4 * i + 3] = 255;
    }
}
// Pre-UI transfer preserves the native destination alpha for subsequent blends.
inline void dimRgbPixels(uint8_t *pixels, size_t count, const OpaqueDimming &dimming) {
    if (dimming.passthrough) return;
    for (size_t i = 0; i < count; ++i)
        for (unsigned channel = 0; channel < 3; ++channel)
            pixels[4*i+channel] = dimming.black ? 0 : dimming.channel[pixels[4*i+channel]];
}
inline void bgraToRgba(uint8_t *destination, const uint8_t *source, size_t pixels) {
    for (size_t i = 0; i < pixels; i++) {
        auto b = source[4 * i], g = source[4 * i + 1], r = source[4 * i + 2], a = source[4 * i + 3];
        destination[4 * i] = r;
        destination[4 * i + 1] = g;
        destination[4 * i + 2] = b;
        destination[4 * i + 3] = a;
    }
}
} // namespace ss2vr
