#pragma once

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#include "esphome/components/climate/climate.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#include "esphome/components/spi/spi.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/time.h"

#include "mhi_ac_protocol.h"

namespace esphome {
namespace mhi_ac_ctrl {

constexpr char TAG[] = "mhi_ac_ctrl";

enum SPIClock {
  CLOCK_ASSERTED,
  CLOCK_DEASSERTED,
};

enum SPIClockPhase {
  /// The data is sampled on a leading clock edge. (CPHA=0)
  CLOCK_PHASE_LEADING,
  /// The data is sampled on a trailing clock edge. (CPHA=1)
  CLOCK_PHASE_TRAILING,
};

enum SPIBitOrder {
  /// The least significant bit is transmitted/received first.
  BIT_ORDER_LSB_FIRST,
  /// The most significant bit is transmitted/received first.
  BIT_ORDER_MSB_FIRST,
};

enum SPIClockPolarity {
  /** The clock signal idles on LOW. (CPOL=0)
   *
   * A rising edge means a leading edge for the clock.
   */
  CLOCK_POLARITY_LOW = false,
  /** The clock signal idles on HIGH. (CPOL=1)
   *
   * A falling edge means a trailing edge for the clock.
   */
  CLOCK_POLARITY_HIGH = true,
};

class MHIACActiveMode;
class MHIACDebugMode;
class MHIACFrameSize;
class MHIACFrameErrors;

class MHIACCtrl
    : public esphome::Component,
      public esphome::climate::Climate,
      public esphome::spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_LEADING> {
 public:
  MHIACCtrl(){};

  void setup() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  void dump_config() override;

  void loop() override;

  bool HOT wait_clock_(enum SPIClock clock) {
    uint32_t start_ms = millis();
    bool state = (clock == CLOCK_ASSERTED) ? (this->clock_polarity_ == CLOCK_POLARITY_LOW)
                                           : (this->clock_polarity_ != CLOCK_POLARITY_LOW);

    while (this->pin_a_->digital_read() != state) {
      if ((millis() - start_ms) > this->timeout_ms_)
        return false;
    }
    return true;
  }

  uint16_t mhi_transfer_(uint16_t data, size_t num_bits) {
    uint16_t out_data = 0;

    for (uint8_t i = 0; i != num_bits; i++) {
      uint8_t shift;

      if (this->bit_order_ == BIT_ORDER_MSB_FIRST) {
        shift = num_bits - 1 - i;
      } else {
        shift = i;
      }

      if (this->clock_phase_ == CLOCK_PHASE_LEADING) {
        // sampling on leading edge
        this->pin_c_->digital_write(data & (1 << shift));
        if (!this->wait_clock_(CLOCK_ASSERTED)) {
          ESP_LOGD("spi", "Clock not asserted within %d ms.", this->timeout_ms_);
          break;
        }
        out_data |= uint16_t(this->pin_b_->digital_read()) << shift;
        if (!this->wait_clock_(CLOCK_DEASSERTED)) {
          ESP_LOGD("spi", "Clock not de-asserted within %d ms.", this->timeout_ms_);
          break;
        }
      } else {
        // sampling on trailing edge
        if (!this->wait_clock_(CLOCK_ASSERTED)) {
          ESP_LOGD("spi", "Clock not asserted within %d ms.", this->timeout_ms_);
          break;
        }
        this->pin_c_->digital_write(data & (1 << shift));
        if (!this->wait_clock_(CLOCK_DEASSERTED)) {
          ESP_LOGD("spi", "Clock not de-asserted within %d ms.", this->timeout_ms_);
          break;
        }
        out_data |= uint16_t(this->pin_b_->digital_read()) << shift;
      }
    }

    App.feed_wdt();

    return out_data;
  }

  uint8_t read_byte() { return this->mhi_transfer_(0, 8); }

  void control(const climate::ClimateCall &call) override;

  // bool pin_wait(GPIOPin *pin, bool state, uint32_t stable_state_ms, uint32_t timeout_ms);
  //   CallbackManager<void(const char *, size_t)> status_message_callback_{};

  void set_active_mode(bool mode) { this->active_mode_ = mode; }
  bool get_active_mode() { return this->active_mode_; }

  void set_debug_mode(bool mode) { this->debug_mode_ = mode; }
  bool get_debug_mode() { return this->debug_mode_; }

  const char *get_frame_size() { return frame_size_str(this->frame_status_); }
  uint8_t get_frame_errors() { return this->frame_errors_; }
  void frame_errors_reset() { this->frame_errors_ = 0; }

  void set_pin_cs_sync(GPIOPin *pin) { this->pin_cs_sync_ = pin; }
  void set_pin_clk_mon(GPIOPin *pin) { this->pin_clk_mon_ = pin; }

  void set_pin_a(GPIOPin *pin) { this->pin_a_ = pin; }
  void set_pin_b(GPIOPin *pin) { this->pin_b_ = pin; }
  void set_pin_c(GPIOPin *pin) { this->pin_c_ = pin; }
  void set_pin_d(GPIOPin *pin) { this->pin_d_ = pin; }

 protected:
  GPIOPin *pin_clk_mon_ = nullptr;
  GPIOPin *pin_cs_sync_ = nullptr;

  GPIOPin *pin_a_ = nullptr;
  GPIOPin *pin_b_ = nullptr;
  GPIOPin *pin_c_ = nullptr;
  GPIOPin *pin_d_ = nullptr;

  enum spi::SPIClockPolarity clock_polarity_{spi::CLOCK_POLARITY_HIGH};
  enum spi::SPIClockPhase clock_phase_{spi::CLOCK_PHASE_TRAILING};

  uint32_t timeout_ms_{300};

  float minimum_temperature_{18.0f};
  float maximum_temperature_{30.0f};
  float temperature_step_{0.5f};

  struct MHI_climate_settings climate_settings_;

  climate::ClimateTraits traits() override;

  bool active_mode_{false};  // true
  bool debug_mode_{false};
  enum MHI_frame_status frame_status_;
  uint32_t frame_errors_{0};
};

#ifdef USE_SWITCH
class MHIACActiveMode : public esphome::Component, public esphome::switch_::Switch, public Parented<MHIACCtrl> {
 public:
  MHIACActiveMode() {}

  void setup() { this->publish_state(this->parent_->get_active_mode()); }

  void dump_config() { LOG_SWITCH("  ", "Active Mode Switch", this); }

 protected:
  void write_state(bool state) {
    this->parent_->set_active_mode(state);
    this->publish_state(state);
  }
};

class MHIACDebugMode : public esphome::Component, public esphome::switch_::Switch, public Parented<MHIACCtrl> {
 public:
  MHIACDebugMode() {}

  void setup() { this->publish_state(this->parent_->get_debug_mode()); }

  void dump_config() { LOG_SWITCH("  ", "Debug Mode Switch", this); }

 protected:
  void write_state(bool state) {
    this->parent_->set_debug_mode(state);
    this->publish_state(state);
  }
};
#endif

#ifdef USE_TEXT_SENSOR
class MHIACFrameSize : public esphome::Component, public esphome::text_sensor::TextSensor, public Parented<MHIACCtrl> {
 public:
  MHIACFrameSize() {}

  void setup() { this->publish_state(this->parent_->get_frame_size()); }

  void dump_config() { LOG_TEXT_SENSOR("  ", "Frame Size Sensor", this); }

  void loop() {
    std::string frame_format;
    frame_format = this->parent_->get_frame_size();
    if (frame_format != this->get_raw_state()) {
      this->publish_state(frame_format);
    }
  }
};
#endif

#ifdef USE_SENSOR
class MHIACFrameErrors : public esphome::Component, public esphome::sensor::Sensor, public Parented<MHIACCtrl> {
 public:
  MHIACFrameErrors() {}

  void setup() { this->publish_state(this->parent_->get_frame_errors()); }

  void dump_config() { LOG_SENSOR("  ", "Frame Errors Sensor", this); }

  void loop() {
    if (this->parent_->get_frame_errors() != this->get_raw_state()) {
      this->publish_state(this->parent_->get_frame_errors());
    }
  }
};
#endif

}  // namespace mhi_ac_ctrl
}  // namespace esphome
