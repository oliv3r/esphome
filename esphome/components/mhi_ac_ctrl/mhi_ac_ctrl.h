#pragma once

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
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

#include "mhi_protocol.h"

namespace esphome {
namespace mhi_ac_ctrl {

constexpr char TAG[] = "mhi_ac_ctrl";

class MHIACActiveMode;
class MHIACClimate;
class MHIACDebugMode;
class MHIACFrameSize;
class MHIACFrameErrors;

class MHIACCtrl
    : public esphome::Component,
#ifdef USE_CLIMATE
      public esphome::climate::Climate,
#endif
      public esphome::spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_LEADING> {
 public:
  MHIACCtrl(){};

#ifdef USE_CLIMATE
  void climate_reset_state_() {
    this->mode = climate::CLIMATE_MODE_OFF;
    this->action = climate::CLIMATE_ACTION_OFF;
    this->fan_mode = climate::CLIMATE_FAN_OFF;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
    this->preset = climate::CLIMATE_PRESET_NONE;
    this->target_temperature = NAN;
    this->current_temperature = NAN;
    this->target_humidity = NAN;

    this->climate_target_buffer_swap();
    mhi_climate_defaults(this->climate_target_read_);
    mhi_climate_defaults(this->climate_target_write_);
    mhi_climate_defaults(&this->climate_current_);

    mhi_opdata_defaults(&this->opdata_);

    this->publish_state();
  }

  void climate_settings_process();

  void control(const climate::ClimateCall &call);

  climate::ClimateTraits traits() {
    climate::ClimateTraits traits = climate::ClimateTraits();

    traits.set_supports_action(true);
    traits.set_supports_current_temperature(true);
    traits.set_supports_current_humidity(false);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_supports_target_humidity(false);
    traits.set_supported_fan_modes({
        //        climate::CLIMATE_FAN_ON,
        climate::CLIMATE_FAN_OFF,
        climate::CLIMATE_FAN_QUIET,
        climate::CLIMATE_FAN_LOW,
        climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH,
        climate::CLIMATE_FAN_AUTO,
        //        climate::CLIMATE_FAN_FOCUS,
        //        climate::CLIMATE_FAN_DIFFUSE,
    });
    traits.set_supported_modes({
        climate::CLIMATE_MODE_OFF,
        climate::CLIMATE_MODE_COOL,
        climate::CLIMATE_MODE_HEAT,
        climate::CLIMATE_MODE_HEAT_COOL,
        climate::CLIMATE_MODE_DRY,
        climate::CLIMATE_MODE_FAN_ONLY,
    });
    //    traits.set_supported_presets({
    //        climate::CLIMATE_PRESET_NONE,
    //      climate::CLIMATE_PRESET_HOME,
    //      climate::CLIMATE_PRESET_AWAY,
    //      climate::CLIMATE_PRESET_BOOST,
    //      climate::CLIMATE_PRESET_COMFORT,
    //      climate::CLIMATE_PRESET_ECO,
    //      climate::CLIMATE_PRESET_SLEEP,
    //      climate::CLIMATE_PRESET_ACTIVITY,
    //    });
    traits.set_supported_swing_modes({
        climate::CLIMATE_SWING_OFF,
        climate::CLIMATE_SWING_BOTH,
        climate::CLIMATE_SWING_VERTICAL,
        climate::CLIMATE_SWING_HORIZONTAL,
    });
    traits.set_visual_min_temperature(this->minimum_temperature_);
    traits.set_visual_max_temperature(this->maximum_temperature_);
    traits.set_visual_temperature_step(this->temperature_step_);
    traits.set_visual_current_temperature_step(0.1);
    traits.set_visual_min_humidity(this->minimum_humidity_);
    traits.set_visual_max_humidity(this->maximum_humidity_);

    return traits;
  }
#endif

  void setup() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  void dump_config() override;

  void loop() override;

  // bool pin_wait(GPIOPin *pin, bool state, uint32_t stable_state_ms, uint32_t timeout_ms);

  void set_debug_mode(bool mode) { this->debug_mode_ = mode; }
  bool get_debug_mode() { return this->debug_mode_; }

  void set_loop_enable(bool enable) { this->loop_enable_ = enable; }
  bool get_loop_enable() { return this->loop_enable_; }

  void set_internal_temperature_sensor(bool enable) { this->internal_temperature_sensor_ = enable; }
  bool get_internal_temperature_sensor() { return this->internal_temperature_sensor_; }

  enum MHI_frame_size get_frame_size() { return this->frame_size_; }

  size_t get_frame_errors() { return this->frame_errors_; }
  void frame_errors_reset() { this->frame_errors_ = 0; }

  void set_pin_cs_sync(GPIOPin *pin) { this->pin_cs_sync_ = pin; }
  void set_pin_clk_mon(GPIOPin *pin) { this->pin_clk_mon_ = pin; }

 protected:
  GPIOPin *pin_clk_mon_ = nullptr;
  GPIOPin *pin_cs_sync_ = nullptr;

  spi::SPIClockPolarity clock_polarity_;
  spi::SPIClockPhase clock_phase_;

  uint32_t timeout_ms_{300};
  uint32_t forced_update_ms_{30000};

  bool internal_temperature_sensor_{true};
  bool debug_mode_{false};
  bool loop_enable_{true};
  enum MHI_frame_size frame_size_ { MHI_FRAME_SIZE_UNKNOWN };
  size_t frame_errors_{0};
  size_t duplicate_frames_{0};

  struct MHI_operational_data opdata_;

  struct MHI_climate_settings climate_current_;
  struct MHI_climate_settings *climate_target_read_;
  struct MHI_climate_settings *climate_target_write_;

  void climate_target_buffer_swap();

  const float minimum_temperature_{18.0f};
  const float maximum_temperature_{30.0f};
  const float temperature_step_{0.5f};
  const float minimum_humidity_{30.0f};
  const float maximum_humidity_{99.0f};
};

#ifdef USE_BUTTON
class MHIACFrameErrorsReset : public esphome::Component, public esphome::button::Button, public Parented<MHIACCtrl> {
 public:
  MHIACFrameErrorsReset() {}

  void press_action() { this->parent_->frame_errors_reset(); }

  void dump_config() { LOG_BUTTON("  ", "Reset button", this); }
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

#ifdef USE_SWITCH
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

class MHIACLoopEnable : public esphome::Component, public esphome::switch_::Switch, public Parented<MHIACCtrl> {
 public:
  MHIACLoopEnable() {}

  void setup() { this->publish_state(this->parent_->get_loop_enable()); }

  void dump_config() { LOG_SWITCH("  ", "Loop Enable", this); }

 protected:
  void write_state(bool state) {
    this->parent_->set_loop_enable(state);
    this->publish_state(state);
  }
};

class MHIACInternalTempSensor : public esphome::Component, public esphome::switch_::Switch, public Parented<MHIACCtrl> {
 public:
  MHIACInternalTempSensor() {}

  void setup() { this->publish_state(this->parent_->get_internal_temperature_sensor()); }

  void dump_config() { LOG_SWITCH("  ", "Internal Temperature Sensor Switch", this); }

 protected:
  void write_state(bool state) {
    this->parent_->set_internal_temperature_sensor(state);
    this->publish_state(state);
  }
};
#endif

#ifdef USE_TEXT_SENSOR
class MHIACFrameSize : public esphome::Component, public esphome::text_sensor::TextSensor, public Parented<MHIACCtrl> {
 public:
  MHIACFrameSize() {}

  void setup() { this->publish_state(mhi_frame_size_str(this->parent_->get_frame_size())); }

  void dump_config() { LOG_TEXT_SENSOR("  ", "Frame Size Sensor", this); }

  void loop() {
    enum MHI_frame_size frame_size = this->parent_->get_frame_size();
    if (this->frame_size_ != frame_size) {
      this->frame_size_ = frame_size;
      this->publish_state(mhi_frame_size_str(frame_size));
    }
  }

 protected:
  enum MHI_frame_size frame_size_ { MHI_FRAME_SIZE_UNKNOWN };
};
#endif

}  // namespace mhi_ac_ctrl
}  // namespace esphome
