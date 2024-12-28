#ifdef ESP32

#include "enabled.h"

#if FASTLED_RMT5

#include "rmt_strip.h"
#include "esp_log.h"
#include "configure_led.h"
#include "construct.h"
#include "esp_check.h"

#include "rmt_strip_group.h"
#include "fl/warn.h"
#include "fl/assert.h"

namespace fastled_rmt51_strip {

#define TAG "rtm_strip_no_recycle.cpp"

#define RMT_ASSERT(x)                  \
    {                                  \
        if (!(x)) {                    \
            ESP_ERROR_CHECK(ESP_FAIL); \
        }                              \
    }

#define RMT_ASSERT_LT(x, y)            \
    {                                  \
        if (!((x) < (y))) {            \
            ESP_ERROR_CHECK(ESP_FAIL); \
        }                              \
    }


// new experiment - try disabling recycling of RMT channels to try and fix
// first led always on bug.

class RmtLedStripNoRecycle : public IRmtLedStrip {
public:
    RmtLedStripNoRecycle(uint16_t T0H, uint16_t T0L, uint16_t T1H, uint16_t T1L, uint32_t TRESET,
                int pin, uint32_t max_leds, bool is_rgbw)
        : mT0H(T0H),
          mT0L(T0L),
          mT1H(T1H),
          mT1L(T1L),
          mTRESET(TRESET),
          mIsRgbw(is_rgbw),
          mPin(pin),
          mMaxLeds(max_leds) {
        const uint8_t bytes_per_pixel = is_rgbw ? 4 : 3;
        mBuffer = static_cast<uint8_t*>(calloc(max_leds, bytes_per_pixel));
    }

    void acquire_rmt_if_necessary() {
        if (mAquired) {
            return;
        }
        do {
            esp_err_t err = construct_led_strip(
                mT0H, mT0L, mT1H, mT1L, mTRESET,
                mPin, mMaxLeds, mIsRgbw, mBuffer,
                &mLedStrip);

            if (err == ESP_OK) {
                RmtActiveStripGroup::instance().add(this);
                break;
            }

            if (err == ESP_ERR_NOT_FOUND) {  // No free RMT channels yet.
                int active_strips = RmtActiveStripGroup::instance().count_active();
                if (active_strips == 0) {
                    // If there are no active strips and we don't have any resources then
                    // this means RMT is not supported on this platform so we just abort.
                    ESP_ERROR_CHECK(err);
                }
                // Update the total number of active strips allowed.
                RmtActiveStripGroup::instance().set_total_allowed(active_strips);
                // FASTLED_WARN("All available RMT channels are in use, and no more can be allocated.");
                FASTLED_ASSERT(false, "All available RMT channels are in use, and no more can be allocated.");
                return;
            }
            // Some other error that we can't handle.

            ESP_LOGE(TAG, "construct_led_strip failed because of unexpected error, is DMA not supported on this device?: %s", esp_err_to_name(err));
            ESP_ERROR_CHECK(err);
        } while (true);
        mAquired = true;
    }


    void release_rmt() {
        if (!mAquired) {
            FASTLED_WARN("release_rmt called but mAquired is false");
            return;
        }
        led_strip_wait_refresh_done(mLedStrip, -1, true);
        mAquired = false;
    }


    virtual ~RmtLedStripNoRecycle() override {
        if (mLedStrip) {
            led_strip_del(mLedStrip, false);
            mLedStrip = nullptr;
        }

        RmtActiveStripGroup::instance().remove(this);
        free(mBuffer);
    }

    virtual void set_pixel(uint32_t i, uint8_t r, uint8_t g, uint8_t b) override {
        RMT_ASSERT_LT(i, mMaxLeds);
        RMT_ASSERT(!mIsRgbw);
        mBuffer[i * 3 + 0] = r;
        mBuffer[i * 3 + 1] = g;
        mBuffer[i * 3 + 2] = b;
    }

    virtual void set_pixel_rgbw(uint32_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t w) override {
        RMT_ASSERT_LT(i, mMaxLeds);
        RMT_ASSERT(mIsRgbw);
        mBuffer[i * 4 + 0] = r;
        mBuffer[i * 4 + 1] = g;
        mBuffer[i * 4 + 2] = b;
        mBuffer[i * 4 + 3] = w;
    }

    void draw_and_wait_for_completion() {
        acquire_rmt_if_necessary();
        draw_async();
        wait_for_draw_complete();
    }

    void draw_async() {
        ESP_ERROR_CHECK(led_strip_refresh_async(mLedStrip));
    }

    virtual void draw() override {
        FASTLED_ASSERT(!mDrawing, "draw called while already drawing");
        acquire_rmt_if_necessary();
        draw_async();
        mDrawing = true;
    }

    virtual void wait_for_draw_complete() override {
        if (!mDrawing) {
            return;
        }
        led_strip_wait_refresh_done(mLedStrip, -1, true);
        mDrawing = false;
    }

    virtual uint32_t num_pixels() const {
        return mMaxLeds;
    }

private:
    int mPin = -1;
    led_strip_handle_t mLedStrip = nullptr;
    bool mIsRgbw = false;
    uint32_t mMaxLeds = 0;
    uint8_t* mBuffer = nullptr;
    bool mAquired = false;
    uint16_t mT0H = 0;
    uint16_t mT0L = 0;
    uint16_t mT1H = 0;
    uint16_t mT1L = 0;
    uint32_t mTRESET = 0;
    bool mDrawing = false;
};


IRmtLedStrip* create_rmt_led_strip_no_recycle(
        uint16_t T0H, uint16_t T0L, uint16_t T1H, uint16_t T1L, uint32_t TRESET, // Timing is in nanoseconds
        int pin, uint32_t max_leds, bool is_rgbw){
    return new RmtLedStripNoRecycle(T0H, T0L, T1H, T1L, TRESET, pin, max_leds, is_rgbw);
}

}  // namespace fastled_rmt51_strip

#endif  // FASTLED_RMT5

#endif  // ESP32