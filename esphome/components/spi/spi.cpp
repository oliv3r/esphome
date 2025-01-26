#include "spi.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace spi {

const char *const TAG = "spi";

SPIDelegate *const SPIDelegate::NULL_DELEGATE =  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    new SPIDelegateDummy();
// https://bugs.llvm.org/show_bug.cgi?id=48040

bool SPIDelegate::is_ready() { return true; }

GPIOPin *const NullPin::NULL_PIN = new NullPin();  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

SPIDelegate *SPIComponent::register_device(SPIClient *device, SPIMode mode, SPIRole role, SPIBitOrder bit_order,
                                           uint32_t data_rate, GPIOPin *cs_pin) {
  if (this->devices_.count(device) != 0) {
    ESP_LOGE(TAG, "SPI device already registered");
    return this->devices_[device];
  }
  SPIDelegate *delegate = this->spi_bus_->get_delegate(data_rate, bit_order, mode, role, cs_pin);  // NOLINT
  this->devices_[device] = delegate;
  return delegate;
}

void SPIComponent::unregister_device(SPIClient *device) {
  if (this->devices_.count(device) == 0) {
    esph_log_e(TAG, "SPI device not registered");
    return;
  }
  delete this->devices_[device];  // NOLINT
  this->devices_.erase(device);
}

void SPIComponent::setup() {
  ESP_LOGD(TAG, "Setting up SPI bus...");

  if (this->mosi_pin_ == nullptr)
    this->mosi_pin_ = NullPin::NULL_PIN;
  if (this->miso_pin_ == nullptr)
    this->miso_pin_ = NullPin::NULL_PIN;
  if (this->clk_pin_ == nullptr) {
    ESP_LOGE(TAG, "No clock pin for SPI");
    this->mark_failed();
    return;
  }

  if (this->using_hw_) {
    this->spi_bus_ =
        SPIComponent::get_bus(this->interface_, this->clk_pin_, this->mosi_pin_, this->miso_pin_, this->data_pins_);
    if (this->spi_bus_ == nullptr) {
      ESP_LOGE(TAG, "Unable to allocate SPI interface");
      this->mark_failed();
    }
  } else {
    this->spi_bus_ = new SPIBus(this->clk_pin_, this->mosi_pin_, this->miso_pin_);  // NOLINT
    this->clk_pin_->setup();
    this->clk_pin_->digital_write(true);
    this->mosi_pin_->setup();
    this->miso_pin_->setup();
  }
}

void SPIComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "SPI bus:");
  ESP_LOGCONFIG(TAG, "  Role: %s", this->role_ == ROLE_MASTER ? "master" : "slave");
  LOG_PIN("  CLK Pin: ", this->clk_pin_)
  LOG_PIN("  MISO Pin: ", this->miso_pin_)
  LOG_PIN("  MOSI Pin: ", this->mosi_pin_)
  for (size_t i = 0; i != this->data_pins_.size(); i++) {
    ESP_LOGCONFIG(TAG, "  Data pin %u: GPIO%d", i, this->data_pins_[i]);
  }
  if (this->spi_bus_->is_hw()) {
    ESP_LOGCONFIG(TAG, "  Using HW SPI: %s", this->interface_name_);
  } else {
    ESP_LOGCONFIG(TAG, "  Using software SPI");
  }
}

void SPIDelegateDummy::begin_transaction() { ESP_LOGE(TAG, "SPIDevice not initialised - did you call spi_setup()?"); }

uint8_t SPIDelegateBitBash::transfer(uint8_t data) { return this->transfer_(data, 8); }

void SPIDelegateBitBash::write(uint16_t data, size_t num_bits) { this->transfer_(data, num_bits); }

uint16_t SPIDelegateBitBash::slave_transfer_(uint16_t data, size_t num_bits) {
  uint16_t out_data = 0;

  for (uint8_t i = 0; i < num_bits; i++) {
    uint8_t shift;

    if (this->bit_order_ == BIT_ORDER_MSB_FIRST) {
      shift = num_bits - 1 - i;
    } else {
      shift = i;
    }

    if (this->clock_phase_ == CLOCK_PHASE_LEADING) {
      this->miso_pin_->digital_write(data & (1 << shift));
      if (!this->wait_clock_(CLOCK_ASSERTED)) {
        ESP_LOGD(TAG, "Leading phase clock not asserted within %d ms.", this->timeout_ms_);
        break;
      }
      out_data |= uint16_t(this->mosi_pin_->digital_read()) << shift;
      if (!this->wait_clock_(CLOCK_DEASSERTED)) {
        ESP_LOGD(TAG, "Leading phase clock not de-asserted within %d ms.", this->timeout_ms_);
        break;
      }
    } else {  // CLOCK_PHASE_TRAILING
      if (!this->wait_clock_(CLOCK_ASSERTED)) {
        ESP_LOGD(TAG, "Trailing phase Clock not asserted within %d ms.", this->timeout_ms_);
        break;
      }
      this->miso_pin_->digital_write(data & (1 << shift));
      if (!this->wait_clock_(CLOCK_DEASSERTED)) {
        ESP_LOGD(TAG, "Trailing phase clock not de-asserted within %d ms.", this->timeout_ms_);
        break;
      }
      out_data |= uint16_t(this->mosi_pin_->digital_read()) << shift;
    }
  }

  App.feed_wdt();

  return out_data;
}

uint16_t SPIDelegateBitBash::master_transfer_(uint16_t data, size_t num_bits) {
  uint16_t out_data = 0;

  // Clock starts out at idle level
  this->clk_pin_->digital_write(this->clock_polarity_);

  for (uint8_t i = 0; i < num_bits; i++) {
    uint8_t shift;

    if (this->bit_order_ == BIT_ORDER_MSB_FIRST) {
      shift = num_bits - 1 - i;
    } else {
      shift = i;
    }

    if (this->clock_phase_ == CLOCK_PHASE_LEADING) {
      // sampling on leading edge
      this->mosi_pin_->digital_write(data & (1 << shift));
      this->cycle_clock_();
      out_data |= uint16_t(this->miso_pin_->digital_read()) << shift;
      this->clk_pin_->digital_write(!this->clock_polarity_);
      this->cycle_clock_();
      this->clk_pin_->digital_write(this->clock_polarity_);
    } else {
      // sampling on trailing edge
      this->cycle_clock_();
      this->clk_pin_->digital_write(!this->clock_polarity_);
      this->mosi_pin_->digital_write(data & (1 << shift));
      this->cycle_clock_();
      out_data |= uint16_t(this->miso_pin_->digital_read()) << shift;
      this->clk_pin_->digital_write(this->clock_polarity_);
    }
  }

  App.feed_wdt();

  return out_data;
}

uint16_t SPIDelegateBitBash::transfer_(uint16_t data, size_t num_bits) {
  if (this->role_ == ROLE_SLAVE)
    return slave_transfer_(data, num_bits);
  else
    return master_transfer_(data, num_bits);
}

}  // namespace spi
}  // namespace esphome
