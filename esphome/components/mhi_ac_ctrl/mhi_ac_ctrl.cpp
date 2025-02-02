#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "esphome/components/spi/spi.h"
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "mhi_ac_ctrl.h"
#include "mhi_protocol.h"

namespace esphome {
namespace mhi_ac_ctrl {

static void debug_frame_print(const uint8_t *frame, enum MHI_frame_size frame_size) {
  char *buf_std;
  char *buf_ext;

  asprintf(&buf_std,
           "Frame data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X "
           "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
           frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9],
           frame[10], frame[11], frame[12], frame[13], frame[14], frame[15], frame[16], frame[17], frame[18],
           frame[19]);
  if (frame_size == MHI_FRAME_SIZE_EXTENDED) {
    asprintf(&buf_ext,
             "%s | 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X "
             "0x%02X",
             buf_std, frame[20], frame[21], frame[22], frame[23], frame[24], frame[25], frame[26], frame[27], frame[28],
             frame[29], frame[30], frame[31], frame[32]);
    ESP_LOGD(TAG, buf_ext);
    free(buf_ext);
  } else {
    ESP_LOGD(TAG, buf_std);
  }
  free(buf_std);
}

static void debug_climate_print(const uint8_t *frame, const struct MHI_climate_settings *climate) {
  if (climate == nullptr)
    return;

  ESP_LOGD(TAG, "Error: 0x%02x", climate->last_error);

  ESP_LOGD(TAG, "Troom: %02.2f", climate->temperature_room);
  ESP_LOGD(TAG, "Tsetp: %02.2f", climate->temperature_setpoint);
  ESP_LOGD(TAG, "internal: %s", YESNO(climate->temperature_room_use_internal));

  ESP_LOGD(TAG, "power: %s [%d]", mhi_power_str(climate->power), climate->power);
  ESP_LOGD(TAG, "mode: %s [%d]", mhi_mode_str(climate->mode), climate->mode);
  ESP_LOGD(TAG, "fan: %s [%d]", mhi_fan_str(climate->fan), climate->fan);
  ESP_LOGD(TAG, "vanes_ud: %s [%d]", mhi_vanes_ud_str(climate->vanes_ud), climate->vanes_ud);
  ESP_LOGD(TAG, "vanes lr: %s [%d]", mhi_vanes_lr_str(climate->vanes_lr), climate->vanes_lr);
  ESP_LOGD(TAG, "vanes 3d: %s [%d]", mhi_vanes_3d_auto_str(climate->vanes_3d_auto), climate->vanes_3d_auto);

  ESP_LOGD(TAG, "compressor power: %s [%d]", mhi_compressor_power_str(climate->compressor_power),
           climate->compressor_power);
  ESP_LOGD(TAG, "compressor mode: %s [%d]", mhi_compressor_mode_str(climate->compressor_mode),
           climate->compressor_mode);
  ESP_LOGD(TAG, "compressor state: %s [%d]", mhi_compressor_state_str(climate->compressor_state));
}

/*
 * Sync on the end of each clock pulse, this does mean we can only process
 * every other frame, but that is still more reliable then getting in the frame
 * pause that could be just before a clock transition.
 */
static bool HOT frame_sync(GPIOPin *pin, bool state, uint32_t stable_state_ms, uint32_t timeout_ms) {
  uint32_t state_change_ms = millis();
  uint32_t timeout_bailout_ms = state_change_ms + timeout_ms;

  if ((pin == nullptr) || (!stable_state_ms) || (!timeout_ms))
    return false;

  while (millis() <= timeout_bailout_ms) {
    if (pin->digital_read() != state)
      state_change_ms = millis();

    if ((millis() - state_change_ms) > stable_state_ms)
      return true;
  }

  return false;
}

void MHIACCtrl::setup() {
  ESP_LOGI(TAG, "Initializing ...");

  this->climate_target_buffer_swap();

#ifdef USE_CLIMATE
  this->climate_reset_state_();
#endif

  if (this->pin_cs_sync_ != nullptr) {
    this->pin_cs_sync_->setup();
    this->pin_cs_sync_->digital_write(true);
  }

  if (this->pin_clk_mon_ != nullptr)
    this->pin_clk_mon_->setup();

  this->spi_setup();

  this->clock_phase_ = spi::Utility::get_phase(this->mode_);
  this->clock_polarity_ = spi::Utility::get_polarity(this->mode_);
}

void MHIACCtrl::dump_config() {
  ESP_LOGCONFIG("", "MHI AC Controller", this);
#ifdef USE_CLIMATE
  LOG_CLIMATE("", "MHI AC Climate", this);
#endif
  LOG_PIN("  Clock pin: ", this->pin_clk_mon_);
  LOG_PIN("  Sync pin: ", this->pin_cs_sync_);
}

void MHIACCtrl::climate_target_buffer_swap() {
  static struct MHI_climate_settings climate_target[2] = {0};
  static int climate_target_index = 0;

  ESP_LOGD(TAG, "swapping target climate buffers");
  this->climate_target_read_ = &climate_target[climate_target_index];
  this->climate_target_write_ = &climate_target[1 - climate_target_index];
  climate_target_index = 1 - climate_target_index;
}

#ifdef USE_CLIMATE
void MHIACCtrl::control(const climate::ClimateCall &call) {
  if (this->climate_target_write_ == nullptr)
    return;

  // if (this->use_internal_temperature_sensor_) {
  //  this->climate_target_write_->temperature_room_use_internal = false;
  //  if (this->internal_temperature_sensor_.has_value())
  //    this->climate_target_write_->temperature_room = this->internal_temperature_sensor_.get_value();
  // } else {
    this->climate_target_write_->temperature_room_use_internal = true;
  // }

  if (call.get_target_temperature().has_value()) {
    this->climate_target_write_->temperature_setpoint = *call.get_target_temperature();
    this->climate_target_write_->temperature_setpoint_change_request = true;
  }

  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = *call.get_mode();

    ESP_LOGD(TAG, "changing mode: %d", mode);
    if (mode == climate::CLIMATE_MODE_OFF)
      this->climate_target_write_->power = MHI_POWER_OFF;
    else
      this->climate_target_write_->power = MHI_POWER_ON;
    this->climate_target_write_->power_change_request = true;

    switch (mode) {
      case climate::CLIMATE_MODE_COOL:
        this->climate_target_write_->mode = MHI_MODE_COOL;
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->climate_target_write_->mode = MHI_MODE_HEAT;
        break;
      case climate::CLIMATE_MODE_DRY:
        this->climate_target_write_->mode = MHI_MODE_DRY;
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        this->climate_target_write_->mode = MHI_MODE_FAN;
        break;
      case climate::CLIMATE_MODE_HEAT_COOL:
        this->climate_target_write_->mode = MHI_MODE_AUTO;
        break;
      default:
        break;
    }

    this->climate_target_write_->mode_change_request = true;
  }

  if (call.get_fan_mode().has_value()) {
    climate::ClimateFanMode fan_mode = *call.get_fan_mode();

    switch (fan_mode) {
      case climate::CLIMATE_FAN_OFF:
        /* fallthrough; We can only report the off state, not actually set it */
      case climate::CLIMATE_FAN_QUIET:
        this->climate_target_write_->fan = MHI_FAN_QUIET;
        break;
      case climate::CLIMATE_FAN_LOW:
        this->climate_target_write_->fan = MHI_FAN_LOW;
        break;
      case climate::CLIMATE_FAN_MEDIUM:
        this->climate_target_write_->fan = MHI_FAN_MEDIUM;
        break;
      case climate::CLIMATE_FAN_HIGH:
        this->climate_target_write_->fan = MHI_FAN_HIGH;
        break;
      case climate::CLIMATE_FAN_AUTO:
        this->climate_target_write_->fan = MHI_FAN_AUTO;
        break;
      default:
        break;
    }

    this->climate_target_write_->fan_change_request = true;
  }

  if (call.get_swing_mode().has_value()) {
    climate::ClimateSwingMode swing_mode = *call.get_swing_mode();

    switch (swing_mode) {
      case climate::CLIMATE_SWING_OFF:
        this->climate_target_write_->vanes_ud = MHI_VANES_UD_AUTO;
        this->climate_target_write_->vanes_lr = MHI_VANES_LR_AUTO;
        break;
      case climate::CLIMATE_SWING_BOTH:
        this->climate_target_write_->vanes_ud = MHI_VANES_UD_SWING;
        this->climate_target_write_->vanes_lr = MHI_VANES_LR_SWING;
      case climate::CLIMATE_SWING_VERTICAL:
        this->climate_target_write_->vanes_ud = MHI_VANES_UD_SWING;
        break;
      case climate::CLIMATE_SWING_HORIZONTAL:
        this->climate_target_write_->vanes_lr = MHI_VANES_LR_SWING;
        break;
      default:
        break;
    }

    this->climate_target_write_->vanes_ud_change_request = true;
    this->climate_target_write_->vanes_lr_change_request = true;
  }

  this->climate_target_buffer_swap();
}

void MHIACCtrl::climate_settings_process() {
  static MHI_climate_settings climate_previous = {0};
  static uint32_t last_publish_time_ms = 0;
  uint32_t publish_time_ms = millis();
  bool force_update = false;

  if (publish_time_ms < last_publish_time_ms)
    last_publish_time_ms = 0;

  this->action = climate::CLIMATE_ACTION_OFF;
  this->fan_mode = climate::CLIMATE_FAN_OFF;
  this->mode = climate::CLIMATE_MODE_OFF;
  this->swing_mode = climate::CLIMATE_SWING_OFF;

  this->current_temperature = this->climate_current_.temperature_room;
  this->target_temperature = this->climate_current_.temperature_setpoint;

  if (this->climate_current_.power == MHI_POWER_ON) {
    if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_IDLE)
      this->action = climate::CLIMATE_ACTION_IDLE;

    switch (this->climate_current_.mode) {
      case MHI_MODE_HEAT:
        this->mode = climate::CLIMATE_MODE_HEAT;
        if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_RUNNING)
          this->action = climate::CLIMATE_ACTION_HEATING;
        break;
      case MHI_MODE_COOL:
        this->mode = climate::CLIMATE_MODE_COOL;
        if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_RUNNING)
          this->action = climate::CLIMATE_ACTION_COOLING;
        break;
      case MHI_MODE_AUTO:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_RUNNING) {
          if (this->climate_current_.compressor_mode == MHI_COMPRESSOR_MODE_HEAT)
            this->action = climate::CLIMATE_ACTION_HEATING;
          else
            this->action = climate::CLIMATE_ACTION_COOLING;
        }
        break;
      case MHI_MODE_DRY:
        this->mode = climate::CLIMATE_MODE_DRY;
        if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_RUNNING)
          this->action = climate::CLIMATE_ACTION_DRYING;
        break;
      case MHI_MODE_FAN:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        if (this->climate_current_.compressor_state == MHI_COMPRESSOR_STATE_RUNNING)
          this->action = climate::CLIMATE_ACTION_FAN;
        break;
    }

    switch (this->climate_current_.fan) {
      case MHI_FAN_QUIET:
        this->fan_mode = climate::CLIMATE_FAN_QUIET;
        break;
      case MHI_FAN_LOW:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case MHI_FAN_MEDIUM:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case MHI_FAN_HIGH:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      case MHI_FAN_AUTO:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
    }

    if ((this->climate_current_.vanes_ud == MHI_VANES_UD_SWING) &&
        (this->climate_current_.vanes_lr == MHI_VANES_LR_SWING))
      this->swing_mode = climate::CLIMATE_SWING_BOTH;
    else if (this->climate_current_.vanes_ud == MHI_VANES_UD_SWING)
      this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
    else if (this->climate_current_.vanes_lr == MHI_VANES_LR_SWING)
      this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
    else
      this->swing_mode = climate::CLIMATE_SWING_OFF;
  }

  if ((publish_time_ms - last_publish_time_ms) >= this->forced_update_ms_) {
    last_publish_time_ms = publish_time_ms;
    force_update = true;
    ESP_LOGD(TAG, "Forcefully publishing state due to update time '%d'", this->forced_update_ms_);
  }

  if ((force_update) || (memcmp(&climate_previous, &this->climate_current_, sizeof(struct MHI_climate_settings)))) {
    memcpy(&climate_previous, &this->climate_current_, sizeof(struct MHI_climate_settings));
    this->publish_state();
  }
}
#endif

void MHIACCtrl::loop() {
  uint8_t frame[MHI_FRAME_SIZE_MAX] = {0x00};
  enum MHI_frame_status frame_status;
  enum MHI_frame_size frame_size;
  size_t frame_counter;

  if (!this->loop_enable_)
    return;

  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(true);

  if (!frame_sync(this->pin_clk_mon_, this->clock_phase_, MHI_FRAME_SYNC_MS, this->timeout_ms_)) {
    ESP_LOGW(TAG, "Timeout reached while trying to sync to frame.");
    return;
  }

  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(false);

  if (this->frame_size_ == MHI_FRAME_SIZE_UNKNOWN)
    frame_size = MHI_FRAME_SIZE_STANDARD;
  else
    frame_size = this->frame_size_;

  frame_counter = mhi_frame_prepare(frame, this->climate_target_read_, &this->climate_current_, frame_size);
  if (this->debug_mode_)
    debug_frame_print(frame, frame_size);
  this->transfer_array(frame, frame_size);
  frame_status = mhi_frame_process(frame, &this->climate_current_, &this->opdata_);
  if (frame_status == MHI_FRAME_DUPLICATE)
    goto exit;

  if (frame_status != MHI_FRAME_OK) {
    ESP_LOGV(TAG, "Framing error: %s", mhi_strerror(frame_status));
    debug_frame_print(frame, frame_size);
    this->frame_errors_++;
    goto exit;
  }

#ifdef USE_CLIMATE
  climate_settings_process();
  if (this->debug_mode_) {
    //      debug_climate_print(frame, &this->climate_current_);
    debug_frame_print(frame, frame_size);
  }
#endif

  frame_size = mhi_frame_size(frame);
  if (this->frame_size_ != frame_size) {
    ESP_LOGD(TAG, "Changed to '%s' frame size %d bytes", mhi_frame_size_str(frame_size), frame_size);
    this->frame_size_ = frame_size;
  }

exit:
  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(true);
}

}  // namespace mhi_ac_ctrl
}  // namespace esphome
