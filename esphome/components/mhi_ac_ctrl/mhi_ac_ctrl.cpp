#include "esphome/components/climate/climate.h"
#include "esphome/components/spi/spi.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "mhi_ac_ctrl.h"
#include "mhi_ac_protocol.h"

namespace esphome {
namespace mhi_ac_ctrl {

static void print_frame(uint8_t *frame, enum MHI_frame_status frame_status) {
  ESP_LOGD(TAG,
           "Standard Frame: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X "
           "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
           frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9],
           frame[10], frame[11], frame[12], frame[13], frame[14], frame[15], frame[16], frame[17], frame[18], frame[19],
           frame[20]);
  if (frame_status == MHI_FRAME_STATUS_SIZE_EXTENDED) {
    ESP_LOGD(TAG,
             "Extended Frame:      0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X "
             "0x%02X 0x%02X",
             frame[21], frame[22], frame[23], frame[24], frame[25], frame[26], frame[27], frame[28], frame[29],
             frame[30], frame[31], frame[32], frame[33]);
  }
}

static bool pin_wait(GPIOPin *pin, bool state, uint32_t stable_state_ms, uint32_t timeout_ms) {
  uint32_t start_ms = millis();
  uint32_t state_ms = 0;

  if (pin == nullptr)
    return true;

  do {
    if ((millis() - start_ms) > timeout_ms) {
      return false;
    }

    if (pin->digital_read() == state)
      state_ms = millis();

  } while ((millis() - state_ms) < stable_state_ms);

  return true;
}

climate::ClimateTraits MHIACCtrl::traits() {
  esphome::climate::ClimateTraits traits = climate::ClimateTraits();

  traits.set_visual_min_temperature(this->minimum_temperature_);
  traits.set_visual_max_temperature(this->maximum_temperature_);
  traits.set_visual_temperature_step(this->temperature_step_);

  return traits;
}

void MHIACCtrl::setup() {
  ESP_LOGI(TAG, "Initializing ...");

  if (this->pin_cs_sync_ != nullptr) {
    this->pin_cs_sync_->setup();
    this->pin_cs_sync_->digital_write(true);
  }

  if (this->pin_clk_mon_ != nullptr)
    this->pin_clk_mon_->setup();

  this->clock_phase_ = spi::Utility::get_phase(this->mode_);
  this->clock_polarity_ = spi::Utility::get_polarity(this->mode_);

  if (this->pin_a_ != nullptr) {
    this->pin_a_->setup();
    this->pin_a_->digital_write(false);
  }

  if (this->pin_b_ != nullptr) {
    this->pin_b_->setup();
    this->pin_b_->digital_write(false);
  }

  if (this->pin_c_ != nullptr) {
    this->pin_c_->setup();
    this->pin_c_->digital_write(false);
  }

  if (this->pin_d_ != nullptr) {
    this->pin_d_->setup();
    this->pin_d_->digital_write(false);
  }
}

void MHIACCtrl::dump_config() {
  ESP_LOGCONFIG("", "MHI AC Controller", this);
  LOG_PIN("  Clock pin: ", this->pin_clk_mon_);
  LOG_PIN("  Sync pin: ", this->pin_cs_sync_);
  ESP_LOGCONFIG(TAG, "  Minimum Temperature: %.1f°C", this->minimum_temperature_);
  ESP_LOGCONFIG(TAG, "  Maximum Temperature: %.1f°C", this->maximum_temperature_);
  ESP_LOGCONFIG(TAG, "  Temperature step: %.1f°C", this->temperature_step_);

  if (this->pin_a_ != nullptr)
    LOG_PIN("  Pin A: ", this->pin_a_);
  if (this->pin_b_ != nullptr)
    LOG_PIN("  Pin B: ", this->pin_b_);
  if (this->pin_c_ != nullptr)
    LOG_PIN("  Pin C: ", this->pin_c_);
  if (this->pin_d_ != nullptr)
    LOG_PIN("  Pin D: ", this->pin_d_);
}

void MHIACCtrl::loop() {
  enum MHI_frame_status frame_status;

  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(true);

  if (this->pin_a_ != nullptr)
    this->pin_a_->digital_write(false);
  if (this->pin_b_ != nullptr)
    this->pin_b_->digital_write(false);
  if (this->pin_c_ != nullptr)
    this->pin_c_->digital_write(false);
  if (this->pin_d_ != nullptr)
    this->pin_d_->digital_write(false);

  if (!pin_wait(this->pin_clk_mon_, this->clock_phase_, MHI_FRAME_SYNC_MS, this->timeout_ms_)) {
    ESP_LOGW(TAG, "Timeout reached while waiting for clock pin.");
    return;
  }

  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(false);

  if (this->frame_status_ <= MHI_FRAME_STATUS_DUPLICATE) {
    frame_status = frame_size_get(this->read_byte());
    if (frame_size_valid(frame_status) && (this->frame_status_ != frame_status)) {
      this->frame_status_ = frame_status;
      ESP_LOGD(TAG, "Initial '%s' frame format", frame_size_str(this->frame_status_));
      pin_wait(this->pin_clk_mon_, this->clock_phase_, MHI_FRAME_SYNC_MS, this->timeout_ms_);
    }
  } else {
    uint8_t frame[MHI_FRAME_STATUS_SIZE_MAX];

    mhi_ac_ctrl_frame_prepare(frame, &this->climate_settings_, this->frame_status_, this->active_mode_);

    //    this->transfer_array(frame, 4 /* this->frame_status_ */);

    frame[0] = mhi_transfer_(0, 8);
    frame[1] = mhi_transfer_(0, 8);
    frame[2] = mhi_transfer_(0, 8);
    frame[3] = mhi_transfer_(0, 8);
    pin_wait(this->pin_clk_mon_, this->clock_phase_, 2, this->timeout_ms_);

    frame_status = mhi_ac_ctrl_frame_process(frame, &this->climate_settings_);
    if (frame_status == MHI_FRAME_STATUS_DUPLICATE)
      goto exit;

    if (this->debug_mode_)
      print_frame(frame, frame_status);

    if (frame_status < MHI_FRAME_STATUS_DUPLICATE) {
      this->frame_errors_++;
      goto exit;
    }

    if (this->frame_status_ != frame_status) {
      this->frame_status_ = frame_status;
      ESP_LOGD(TAG, "Changed to '%s' frame format", frame_size_str(this->frame_status_));
    }
  }

exit:
  if (this->pin_cs_sync_ != nullptr)
    this->pin_cs_sync_->digital_write(true);
}

void MHIACCtrl::control(const esphome::climate::ClimateCall &call) { ESP_LOGD("Control", "Control call"); }

}  // namespace mhi_ac_ctrl
}  // namespace esphome
