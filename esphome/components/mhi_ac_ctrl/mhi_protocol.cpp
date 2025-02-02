/* SPDX-License: */

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mhi_protocol.h"

#define fallthrough [[ __fallthrough__ ]]

constexpr uint8_t MHI_FRAME_SRC_IR  = 0b0;
constexpr uint8_t MHI_FRAME_SRC_SPI = 0b1;

#define MHI_FIELD_VERSION 0
constexpr uint8_t MHI_VERSION_STANDARD_MOSI = 0x6C;
constexpr uint8_t MHI_VERSION_STANDARD_MISO = 0xA9;

constexpr uint8_t MHI_VERSION_EXTENDED_MOSI = 0x6D;
constexpr uint8_t MHI_VERSION_EXTENDED_MISO = 0xAA;

#define MHI_FIELD_SIGNATURE_HI 1
constexpr uint8_t MHI_SIGNATURE_HI_MOSI = 0x80;
constexpr uint8_t MHI_SIGNATURE_HI_MISO = 0x00;

#define MHI_FIELD_SIGNATURE_LO 2
constexpr uint8_t MHI_SIGNATURE_LO_MOSI = 0x04;
constexpr uint8_t MHI_SIGNATURE_LO_MISO = 0x07;

#define MHI_FIELD_CTRL_0 3
constexpr uint8_t _MHI_FRAME_POWER_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_POWER_MASK = 0b00000001;
constexpr uint8_t  MHI_FRAME_POWER_OFF  =        0b0;
constexpr uint8_t  MHI_FRAME_POWER_ON   =        0b1;
constexpr uint8_t _MHI_FRAME_POWER_SRC_SHIFT = 1;
constexpr uint8_t _MHI_FRAME_POWER_SRC_MASK = 0b00000010;
#define MHI_FRAME_POWER_SET(power) \
	((((power) << _MHI_FRAME_POWER_SHIFT) & _MHI_FRAME_POWER_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_POWER_SRC_SHIFT))
#define MHI_FRAME_POWER_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_POWER_MASK) >> _MHI_FRAME_POWER_SHIFT)
#define MHI_FRAME_POWER_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_POWER_SRC_MASK) >> _MHI_FRAME_POWER_SRC_SHIFT)

constexpr uint8_t _MHI_FRAME_MODE_SHIFT = 2;
constexpr uint8_t _MHI_FRAME_MODE_MASK = 0b00011100;
constexpr uint8_t  MHI_FRAME_MODE_AUTO =    0b000;
constexpr uint8_t  MHI_FRAME_MODE_DRY  =    0b001;
constexpr uint8_t  MHI_FRAME_MODE_COOL =    0b010;
constexpr uint8_t  MHI_FRAME_MODE_FAN  =    0b011;
constexpr uint8_t  MHI_FRAME_MODE_HEAT =    0b100;
constexpr uint8_t _MHI_FRAME_MODE_SRC_SHIFT = 5;
constexpr uint8_t _MHI_FRAME_MODE_SRC_MASK = 0b00100000;
#define MHI_FRAME_MODE_SET(mode) \
	((((mode) << _MHI_FRAME_MODE_SHIFT) & _MHI_FRAME_MODE_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_MODE_SRC_SHIFT))
#define MHI_FRAME_MODE_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_MODE_MASK) >> _MHI_FRAME_MODE_SHIFT)
#define MHI_FRAME_MODE_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_MODE_SRC_MASK) >> _MHI_FRAME_MODE_SRC_SHIFT)

constexpr uint8_t _MHI_FRAME_VANES_UD_SWING_SHIFT = 6;
constexpr uint8_t _MHI_FRAME_VANES_UD_SWING_MASK = 0b01000000;
constexpr uint8_t  MHI_FRAME_VANES_UD_SWING_OFF  =  0b0;
constexpr uint8_t  MHI_FRAME_VANES_UD_SWING_ON   =  0b1;
constexpr uint8_t _MHI_FRAME_VANES_UD_SWING_SRC_SHIFT = 7;
constexpr uint8_t _MHI_FRAME_VANES_UD_SWING_SRC_MASK = 0b10000000;
#define MHI_FRAME_VANES_UD_SWING_SET(swing) \
	((((swing) << _MHI_FRAME_VANES_UD_SWING_SHIFT) & _MHI_FRAME_VANES_UD_SWING_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_VANES_UD_SWING_SRC_SHIFT))
#define MHI_FRAME_VANES_UD_SWING_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_VANES_UD_SWING_MASK) >> _MHI_FRAME_VANES_UD_SWING_SHIFT)
#define MHI_FRAME_VANES_UD_SWING_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_0]) & _MHI_FRAME_VANES_UD_SWING_SRC_MASK) >> _MHI_FRAME_VANES_UD_SWING_SRC_SHIFT)

#define MHI_FIELD_CTRL_1 4
constexpr uint8_t _MHI_FRAME_FAN_SHIFT  = 0;
constexpr uint8_t _MHI_FRAME_FAN_MASK   = 0b00000111;
constexpr uint8_t  MHI_FRAME_FAN_QUIET  =      0b000;
constexpr uint8_t  MHI_FRAME_FAN_LOW    =      0b001;
constexpr uint8_t  MHI_FRAME_FAN_MEDIUM =      0b010;
constexpr uint8_t  MHI_FRAME_FAN_HIGH   =      0b110;
constexpr uint8_t  MHI_FRAME_FAN_AUTO   =      0b111;
constexpr uint8_t _MHI_FRAME_FAN_SRC_SHIFT = 3;
constexpr uint8_t _MHI_FRAME_FAN_SRC_MASK = 0b00001000;
#define MHI_FRAME_FAN_SET(fan) \
	((((fan) << _MHI_FRAME_FAN_SHIFT) & _MHI_FRAME_FAN_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_FAN_SHIFT))
#define MHI_FRAME_FAN_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_1]) & _MHI_FRAME_FAN_MASK) >> _MHI_FRAME_FAN_SHIFT)
#define MHI_FRAME_FAN_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_1]) & _MHI_FRAME_FAN_SRC_MASK) >> _MHI_FRAME_FAN_SRC_SHIFT)

constexpr uint8_t _MHI_FRAME_VANES_UD_MODE_SHIFT = 4;
constexpr uint8_t _MHI_FRAME_VANES_UD_MODE_MASK        = 0b01110000;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_UP          =  0b000;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_UP_CENTER   =  0b001;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_CENTER      =  0b010;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_DOWN_CENTER =  0b011;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_DOWN        =  0b100;
constexpr uint8_t  MHI_FRAME_VANES_UD_MODE_AUTO        =  0b111;
constexpr uint8_t _MHI_FRAME_VANES_UD_MODE_SRC_SHIFT = 7;
constexpr uint8_t _MHI_FRAME_VANES_UD_MODE_SRC_MASK = 0b10000000;
#define MHI_FRAME_VANES_UD_MODE_SET(mode) \
	((((mode) << _MHI_FRAME_VANES_UD_MODE_SHIFT) & _MHI_FRAME_VANES_UD_MODE_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_VANES_UD_MODE_SRC_SHIFT))
#define MHI_FRAME_VANES_UD_MODE_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_1]) & _MHI_FRAME_VANES_UD_MODE_MASK) >> _MHI_FRAME_VANES_UD_MODE_SHIFT)
#define MHI_FRAME_VANES_UD_MODE_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_1]) & _MHI_FRAME_VANES_UD_MODE_SRC_MASK) >> _MHI_FRAME_VANES_UD_MODE_SRC_SHIFT)

#define MHI_FIELD_TEMPERATURE_SETPOINT 5
constexpr uint8_t _MHI_FRAME_TEMPERATURE_SETPOINT_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_TEMPERATURE_SETPOINT_MASK = 0b01111111;
constexpr uint8_t _MHI_FRAME_TEMPERATURE_SETPOINT_SRC_SHIFT = 7;
constexpr uint8_t _MHI_FRAME_TEMPERATURE_SETPOINT_SRC_MASK = 0b10000000;
#define MHI_FRAME_TEMPERATURE_SETPOINT_SET(temperature) \
	((((uint8_t)(temperature)) << _MHI_FRAME_TEMPERATURE_SETPOINT_SHIFT) & _MHI_FRAME_TEMPERATURE_SETPOINT_MASK)
#define MHI_FRAME_TEMPERATURE_SETPOINT_GET(frame) \
	((((frame)[MHI_FIELD_TEMPERATURE_SETPOINT]) & _MHI_FRAME_TEMPERATURE_SETPOINT_MASK) >> _MHI_FRAME_TEMPERATURE_SETPOINT_SHIFT)
#define MHI_FRAME_TEMPERATURE_SETPOINT_SRC(frame) \
	((((frame)[MHI_FIELD_TEMPERATURE_SETPOINT]) & _MHI_FRAME_TEMPERATURE_SETPOINT_SRC_MASK) >> _MHI_FRAME_TEMPERATURE_SETPOINT_SRC_SHIFT)
constexpr float MHI_FRAME_TEMPERATURE_SETPOINT_MULTIPLIER = 2.0f;

#define MHI_FIELD_TEMPERATURE_ROOM 6
constexpr uint8_t _MHI_FRAME_TEMPERATURE_ROOM_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_TEMPERATURE_ROOM_MASK = 0b01111111;
constexpr uint8_t  MHI_FRAME_TEMPERATURE_ROOM_INTERNAL = 0xFF;
#define MHI_FRAME_TEMPERATURE_ROOM_SET(temp) \
	((((uint8_t)(temp)) << _MHI_FRAME_TEMPERATURE_ROOM_SHIFT) & _MHI_FRAME_TEMPERATURE_ROOM_MASK)
#define MHI_FRAME_TEMPERATURE_ROOM_GET(frame) ((frame)[MHI_FIELD_TEMPERATURE_ROOM])
constexpr float MHI_FRAME_TEMPERATURE_ROOM_MULTIPLIER = 4.0f;
constexpr uint8_t MHI_FRAME_TEMPERATURE_ROOM_OFFSET = 61;

#define MHI_FIELD_ERROR_CODE 7
constexpr uint8_t MHI_ERROR_CODE_NONE = 0x00;
#define MHI_LAST_ERROR_GET(frame) ((frame)[MHI_FIELD_ERROR_CODE])
#define MHI_HAS_ERROR(frame) (((frame)[MHI_FIELD_ERROR_CODE]) != MHI_ERROR_CODE_NONE)

#define MHI_FIELD_OP_ADDR 9
constexpr uint8_t _MHI_FRAME_OP_ADDR_SHIFT = 6;
constexpr uint8_t _MHI_FRAME_OP_ADDR_MASK   = 0b11000000;
constexpr uint8_t  MHI_FRAME_OP_ADDR_CTRL_0 = 0b01;
constexpr uint8_t  MHI_FRAME_OP_ADDR_CTRL_1 = 0b10;
constexpr uint8_t  MHI_FRAME_OP_ADDR_CTRL_2 = 0b11;
#define MHI_FRAME_OP_ADDR_SET(opaddr) \
	((((uint8_t)(opaddr)) << _MHI_FRAME_OP_ADDR_SHIFT) & _MHI_FRAME_OP_ADDR_MASK)
#define MHI_FRAME_OP_ADDR_GET(frame) \
	((((frame)[MHI_FIELD_OP_ADDR]) & _MHI_FRAME_OP_ADDR_MASK) >> _MHI_FRAME_OP_ADDR_SHIFT)

#define MHI_FIELD_OP_CMD 11
constexpr uint8_t _MHI_FRAME_OP_CMD_MASK = 0b11111111;
constexpr uint8_t  MHI_FRAME_OP_CMD_CTRL = 0xff;

#define MHI_FIELD_COMPRESSOR_REQUEST 16
constexpr uint8_t _MHI_FRAME_COMPRESSOR_POWER_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_COMPRESSOR_POWER_MASK = 0b000000001;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_POWER_OFF  =         0b0;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_POWER_ON   =         0b1;
#define MHI_FRAME_COMPRESSOR_POWER_GET(frame) \
	((((frame)[MHI_FIELD_COMPRESSOR_REQUEST]) & _MHI_FRAME_COMPRESSOR_POWER_MASK) >> _MHI_FRAME_COMPRESSOR_POWER_SHIFT)

constexpr uint8_t _MHI_FRAME_COMPRESSOR_MODE_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_COMPRESSOR_MODE_MASK = 0b000000010;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_MODE_COOL =        0b0;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_MODE_HEAT =        0b1;
#define MHI_FRAME_COMPRESSOR_MODE_GET(frame) \
	((((frame)[MHI_FIELD_COMPRESSOR_REQUEST]) & _MHI_FRAME_COMPRESSOR_MODE_MASK) >> _MHI_FRAME_COMPRESSOR_MODE_SHIFT)

constexpr uint8_t _MHI_FRAME_COMPRESSOR_STATE_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_COMPRESSOR_STATE_MASK    = 0b000000100;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_STATE_IDLE    =       0b0;
constexpr uint8_t  MHI_FRAME_COMPRESSOR_STATE_RUNNING =       0b1;
#define MHI_FRAME_COMPRESSOR_STATE_GET(frame) \
	((((frame)[MHI_FIELD_COMPRESSOR_REQUEST]) & _MHI_FRAME_COMPRESSOR_STATE_MASK) >> _MHI_FRAME_COMPRESSOR_STATE_SHIFT)

#define MHI_FIELD_OP_DATA_RW 17
const uint8_t _MHI_FRAME_OP_DATA_RW_SHIFT = 2;
const uint8_t _MHI_FRAME_OP_DATA_RW_MASK = 0b00000100;
#define MHI_FRAME_OP_DATA_RW(frame_counter) \
	(((frame_counter) % 2) ? 0b100 : 0b000)

#define MHI_FIELD_CHECKSUM_STANDARD_HI 18
#define MHI_FIELD_CHECKSUM_STANDARD_LO 19

#define MHI_FIELD_CTRL_2 21
constexpr uint8_t _MHI_FRAME_VANES_LR_MODE_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_VANES_LR_MODE_MASK         = 0b00001111;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_LEFT         =     0b0000;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_LEFT_CENTER  =     0b0001;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_CENTER       =     0b0010;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_RIGHT_CENTER =     0b0011;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_RIGHT        =     0b0100;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_WIDE         =     0b0101;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_SPOT         =     0b0110;
constexpr uint8_t  MHI_FRAME_VANES_LR_MODE_AUTO         =     0b0111;
constexpr uint8_t _MHI_FRAME_VANES_LR_MODE_SRC_SHIFT = 4;
constexpr uint8_t _MHI_FRAME_VANES_LR_MODE_SRC_MASK = 0b00010000;
#define MHI_FRAME_VANES_LR_MODE_SET(v) \
	((((v) << _MHI_FRAME_VANES_LR_MODE_SHIFT) & _MHI_FRAME_VANES_LR_MODE_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_VANES_LR_MODE_SRC_SHIFT))
#define MHI_FRAME_VANES_LR_MODE_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_2]) & _MHI_FRAME_VANES_LR_MODE_MASK) >> _MHI_FRAME_VANES_LR_MODE_SHIFT)
#define MHI_FRAME_VANES_LR_MODE_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_2]) & _MHI_FRAME_VANES_LR_MODE_SRC_MASK) >> _MHI_FRAME_VANES_LR_MODE_SRC_SHIFT)

#define MHI_FIELD_CTRL_3 22
constexpr uint8_t _MHI_FRAME_VANES_LR_SWING_SHIFT = 0;
constexpr uint8_t _MHI_FRAME_VANES_LR_SWING_MASK = 0b00000001;
constexpr uint8_t  MHI_FRAME_VANES_LR_SWING_OFF  =        0b0;
constexpr uint8_t  MHI_FRAME_VANES_LR_SWING_ON   =        0b1;
constexpr uint8_t _MHI_FRAME_VANES_LR_SWING_SRC_SHIFT = 1;
constexpr uint8_t _MHI_FRAME_VANES_LR_SWING_SRC_MASK = 0b00000010;
#define MHI_FRAME_VANES_LR_SWING_SET(s) \
	((((s) << _MHI_FRAME_VANES_LR_SWING_SHIFT) & _MHI_FRAME_VANES_LR_SWING_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_VANES_LR_SWING_SRC_SHIFT))
#define MHI_FRAME_VANES_LR_SWING_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_3]) & _MHI_FRAME_VANES_LR_SWING_MASK) >> _MHI_FRAME_VANES_LR_SWING_SHIFT)
#define MHI_FRAME_VANES_LR_SWING_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_3]) & _MHI_FRAME_VANES_LR_SWING_SRC_MASK) >> _MHI_FRAME_VANES_LR_SWING_SRC_SHIFT)

constexpr uint8_t _MHI_FRAME_VANES_3D_AUTO_SHIFT = 2;
constexpr uint8_t _MHI_FRAME_VANES_3D_AUTO_MASK = 0b00000100;
constexpr uint8_t  MHI_FRAME_VANES_3D_AUTO_OFF  =      0b0;
constexpr uint8_t  MHI_FRAME_VANES_3D_AUTO_ON   =      0b1;
constexpr uint8_t _MHI_FRAME_VANES_3D_AUTO_SRC_SHIFT = 3;
constexpr uint8_t _MHI_FRAME_VANES_3D_AUTO_SRC_MASK = 0b00001000;
#define MHI_FRAME_VANES_3D_AUTO_SET(s) \
	((((s) << _MHI_FRAME_VANES_3D_AUTO_SHIFT) & _MHI_FRAME_VANES_3D_AUTO_MASK) | \
	 (MHI_FRAME_SRC_SPI << _MHI_FRAME_VANES_3D_AUTO_SRC_SHIFT))
#define MHI_FRAME_VANES_3D_AUTO_GET(frame) \
	((((frame)[MHI_FIELD_CTRL_3]) & _MHI_FRAME_VANES_3D_AUTO_MASK) >> _MHI_FRAME_VANES_3D_AUTO_SHIFT)
#define MHI_FRAME_VANES_3D_AUTO_SRC(frame) \
	((((frame)[MHI_FIELD_CTRL_3]) & _MHI_FRAME_VANES_3D_AUTO_SRC_MASK) >> _MHI_FRAME_VANES_3D_AUTO_SRC_SHIFT)

#define MHI_FIELD_CHECKSUM_EXTENDED_LO 32

#define CHECKSUM_HI_SET(checksum) ((checksum) << 8)
#define CHECKSUM_HI_GET(checksum) ((checksum) >> 8)
#define CHECKSUM_LO_SET(checksum) ((checksum) & 0xFF)
#define CHECKSUM_LO_GET(checksum) ((checksum) & 0xFF)


static uint16_t _checksum_calc(const uint8_t *frame, const uint16_t seed,
                               const size_t start, const size_t len)
{
	uint16_t checksum = seed;

	for (size_t i = start; i < len; i++)
		checksum += frame[i];

	return checksum;
}

/*
 * Returns the average to a single decimal point.
 *
 * _Note that when the window is not yet full, the original value is returned._
 */
static inline float _moving_avg_filter(uint8_t *window, const size_t window_size,
                                       size_t *window_slot, const uint8_t value,
                                       const float multiplier, bool *initialized)
{
	uint16_t sum = 0;

	if ((window == nullptr) ||
	    (window_slot == nullptr) ||
	    (initialized == nullptr))
		return NAN;

	window[*window_slot] = value;
	*window_slot = (*window_slot + 1) % window_size;

	if (!*initialized) {
		if (*window_slot != 0)
			return (float)value / multiplier;
		else
			*initialized = true;
	}

	for (size_t i = 0; i < window_size; i++)
		sum += window[i];

	return roundf((sum * 10) / window_size) / multiplier / 10.0f;
}

const char *mhi_strerror(const enum MHI_frame_status frame_status)
{
	switch(frame_status) {
	case MHI_FRAME_ERROR_CHECKSUM_EXTENDED:
		return "extended frame checksum error";
	case MHI_FRAME_ERROR_CHECKSUM_STANDARD:
		return "standard frame checksum error";
	case MHI_FRAME_ERROR_SIGNATURE:
		return "signature mismatch error";
	case MHI_FRAME_ERROR_SIZE:
		return "frame size error";
	case MHI_FRAME_ERROR_VERSION:
		return "frame version error";
	case MHI_FRAME_ERROR_NULL:
		return "null pointer error";
	case MHI_FRAME_ERROR:
		return "generic error";
	case MHI_FRAME_DUPLICATE:
		return "duplicate frame";
	case MHI_FRAME_OK:
		return "frame OK";
	}

	return "undefined error";
}

void mhi_climate_defaults(struct MHI_climate_settings *settings)
{
	settings->last_error = 0x00;

	settings->compressor_power = MHI_COMPRESSOR_POWER_UNKNOWN;
	settings->compressor_mode = MHI_COMPRESSOR_MODE_UNKNOWN;
	settings->compressor_state = MHI_COMPRESSOR_STATE_UNKNOWN;

	settings->power = MHI_POWER_UNKNOWN;
	settings->mode = MHI_MODE_UNKNOWN;
	settings->fan = MHI_FAN_UNKNOWN;
	settings->vanes_3d_auto = MHI_VANES_3D_AUTO_UNKNOWN;
	settings->vanes_ud = MHI_VANES_UD_UNKNOWN;
	settings->vanes_lr = MHI_VANES_LR_UNKNOWN;

	settings->temperature_setpoint = NAN;
	settings->temperature_room_use_internal = true;
	settings->temperature_room = NAN;

	settings->power_change_request = false;
	settings->mode_change_request = false;
	settings->fan_change_request = false;
	settings->vanes_3d_auto_change_request = false;
	settings->vanes_ud_change_request = false;
	settings->vanes_lr_change_request = false;
	settings->temperature_setpoint_change_request = false;

	settings->power_src = MHI_CTRL_SRC_IR;
	settings->mode_src = MHI_CTRL_SRC_IR;
	settings->vanes_3d_auto_src = MHI_CTRL_SRC_IR;
	settings->vanes_ud_mode_src = MHI_CTRL_SRC_IR;
	settings->vanes_ud_swing_src = MHI_CTRL_SRC_IR;
	settings->vanes_lr_mode_src = MHI_CTRL_SRC_IR;
	settings->vanes_lr_swing_src = MHI_CTRL_SRC_IR;
	settings->fan_src = MHI_CTRL_SRC_IR;
	settings->temperature_setpoint_src = MHI_CTRL_SRC_IR;
}

void mhi_opdata_defaults(struct MHI_operational_data *opdata)
{
}

enum MHI_frame_size mhi_frame_size(const uint8_t *frame)
{
	switch (frame[MHI_FIELD_VERSION]) {
	case MHI_VERSION_STANDARD_MOSI:
		return MHI_FRAME_SIZE_STANDARD;
	case MHI_VERSION_EXTENDED_MOSI:
		return MHI_FRAME_SIZE_EXTENDED;
	default:
		return MHI_FRAME_SIZE_UNKNOWN;
	}
}

const char *mhi_frame_size_str(const enum MHI_frame_size frame_size)
{
	switch (frame_size) {
	case MHI_FRAME_SIZE_STANDARD:
		return "Standard";
	case MHI_FRAME_SIZE_EXTENDED:
		return "Extended";
	default:
		return "Unknown";
	}
}

static bool mhi_frame_version_valid(const uint8_t *frame)
{
	if (frame == nullptr)
		return false;

	switch (frame[MHI_FIELD_VERSION]) {
	case MHI_VERSION_STANDARD_MOSI:
		fallthrough;
	case MHI_VERSION_EXTENDED_MOSI:
		return true;
	default:
		return false;
	}
}

static bool mhi_frame_signature_valid(const uint8_t *frame)
{
	if (frame == nullptr)
		return false;

	return ((frame[MHI_FIELD_SIGNATURE_HI] == MHI_SIGNATURE_HI_MOSI) &&
	        (frame[MHI_FIELD_SIGNATURE_LO] == MHI_SIGNATURE_LO_MOSI));
}

static inline uint16_t mhi_checksum_calc_standard(const uint8_t *frame)
{
	return _checksum_calc(frame, 0, 0, MHI_FRAME_SIZE_STANDARD - 2);
}

static inline bool mhi_checksum_standard_valid(const uint8_t *frame, const uint16_t checksum)
{
	return (CHECKSUM_HI_SET(frame[MHI_FIELD_CHECKSUM_STANDARD_HI]) |
	        CHECKSUM_LO_SET(frame[MHI_FIELD_CHECKSUM_STANDARD_LO])) == checksum;
}

static inline uint16_t mhi_checksum_calc_extended(const uint8_t *frame, const uint16_t checksum)
{
	return frame[MHI_FIELD_CHECKSUM_STANDARD_LO] == CHECKSUM_LO_GET(checksum);
}

static inline bool mhi_checksum_extended_valid(const uint8_t *frame, const uint16_t checksum)
{
	return _checksum_calc(frame, checksum, MHI_FRAME_SIZE_STANDARD - 2, MHI_FRAME_SIZE_EXTENDED - 1);
}

static enum MHI_compressor_power mhi_compressor_power_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_COMPRESSOR_POWER_ERROR;

	switch (MHI_FRAME_COMPRESSOR_POWER_GET(frame)) {
	case MHI_FRAME_COMPRESSOR_POWER_ON:
		return MHI_COMPRESSOR_POWER_ON;
	case MHI_FRAME_COMPRESSOR_POWER_OFF:
		return MHI_COMPRESSOR_POWER_OFF;
	}

	return MHI_COMPRESSOR_POWER_UNKNOWN;
}

const char *mhi_compressor_power_str(const enum MHI_compressor_power power)
{
	switch (power) {
	case MHI_COMPRESSOR_POWER_ERROR:
		return "error";
	case MHI_COMPRESSOR_POWER_UNKNOWN:
		return "unknown";
	case MHI_COMPRESSOR_POWER_ON:
		return "on";
	case MHI_COMPRESSOR_POWER_OFF:
		return "off";
	}

	return "exception";
}

static enum MHI_compressor_mode mhi_compressor_mode_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_COMPRESSOR_MODE_ERROR;

	switch (MHI_FRAME_COMPRESSOR_MODE_GET(frame)) {
	case MHI_FRAME_COMPRESSOR_MODE_HEAT:
		return MHI_COMPRESSOR_MODE_HEAT;
	case MHI_FRAME_COMPRESSOR_MODE_COOL:
		return MHI_COMPRESSOR_MODE_COOL;
	}

	return MHI_COMPRESSOR_MODE_UNKNOWN;
}

const char *mhi_compressor_mode_str(const enum MHI_compressor_mode mode)
{
	switch (mode) {
	case MHI_COMPRESSOR_MODE_ERROR:
		return "error";
	case MHI_COMPRESSOR_MODE_UNKNOWN:
		return "unknown";
	case MHI_COMPRESSOR_MODE_HEAT:
		return "heat";
	case MHI_COMPRESSOR_MODE_COOL:
		return "cool";
	}

	return "exception";
}

static enum MHI_compressor_state mhi_compressor_state_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_COMPRESSOR_STATE_ERROR;

	switch (MHI_FRAME_COMPRESSOR_STATE_GET(frame)) {
	case MHI_FRAME_COMPRESSOR_STATE_RUNNING:
		return MHI_COMPRESSOR_STATE_RUNNING;
	case MHI_FRAME_COMPRESSOR_STATE_IDLE:
		return MHI_COMPRESSOR_STATE_IDLE;
	}

	return MHI_COMPRESSOR_STATE_UNKNOWN;
}

const char *mhi_compressor_state_str(const enum MHI_compressor_state state)
{
	switch (state) {
	case MHI_COMPRESSOR_STATE_ERROR:
		return "error";
	case MHI_COMPRESSOR_STATE_UNKNOWN:
		return "unknown";
	case MHI_COMPRESSOR_STATE_RUNNING:
		return "running";
	case MHI_COMPRESSOR_STATE_IDLE:
		return "idle";
	}

	return "exception";
}

static enum MHI_ctrl_src mhi_power_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_POWER_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_mode_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_MODE_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_fan_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_FAN_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_vanes_ud_swing_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_VANES_UD_SWING_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_vanes_ud_mode_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_VANES_UD_MODE_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_vanes_lr_swing_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_VANES_LR_SWING_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_vanes_3d_auto_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_VANES_3D_AUTO_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_vanes_lr_mode_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_VANES_LR_MODE_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

static enum MHI_ctrl_src mhi_temperature_setpoint_src(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_CTRL_SRC_ERROR;

	return MHI_FRAME_TEMPERATURE_SETPOINT_SRC(frame) == MHI_FRAME_SRC_SPI ? MHI_CTRL_SRC_SPI : MHI_CTRL_SRC_IR;
}

const char *mhi_ctrl_src_str(const enum MHI_ctrl_src src)
{
	switch (src) {
	case MHI_CTRL_SRC_ERROR:
		return "error";
	case MHI_CTRL_SRC_UNKNOWN:
		return "unknown";
	case MHI_CTRL_SRC_IR:
		return "IR Remote";
	case MHI_CTRL_SRC_SPI:
		return "SPI";
	}

	return "exception";
}

static uint8_t mhi_vanes_3d_auto_set(const enum MHI_vanes_3d_auto vanes)
{
  uint8_t retval = 0;

	switch (vanes) {
	case MHI_VANES_3D_AUTO_ON:
		retval = MHI_FRAME_VANES_3D_AUTO_ON;
		break;
	case MHI_VANES_3D_AUTO_OFF:
		retval = MHI_FRAME_VANES_3D_AUTO_OFF;
		break;
	default:
		break;
	}

	return MHI_FRAME_VANES_3D_AUTO_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_vanes_3d_auto mhi_vanes_3d_auto_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_VANES_3D_AUTO_ERROR;

	switch (MHI_FRAME_VANES_3D_AUTO_GET(frame)) {
	case MHI_FRAME_VANES_3D_AUTO_ON:
		return MHI_VANES_3D_AUTO_ON;
	case MHI_FRAME_VANES_3D_AUTO_OFF:
		return MHI_VANES_3D_AUTO_OFF;
	}

	return MHI_VANES_3D_AUTO_UNKNOWN;
}

const char *mhi_vanes_3d_auto_str(const enum MHI_vanes_3d_auto mode)
{
	switch (mode) {
	case MHI_VANES_3D_AUTO_ERROR:
		return "error";
	case MHI_VANES_3D_AUTO_UNKNOWN:
		return "unknown";
	case MHI_VANES_3D_AUTO_ON:
		return "on";
	case MHI_VANES_3D_AUTO_OFF:
		return "off";
	}

	return "exception";
}

static uint8_t mhi_vanes_lr_swing_set(const enum MHI_vanes_lr vanes)
{
	if (vanes == MHI_VANES_LR_SWING)
		return MHI_FRAME_VANES_LR_SWING_SET(MHI_FRAME_VANES_LR_SWING_ON);

	return 0;
}

static uint8_t mhi_vanes_lr_set(const enum MHI_vanes_lr vanes)
{
	uint8_t retval = 0;

	switch (vanes) {
	case MHI_VANES_LR_AUTO:
		retval = MHI_FRAME_VANES_LR_MODE_AUTO;
		break;
	case MHI_VANES_LR_LEFT:
		retval = MHI_FRAME_VANES_LR_MODE_LEFT;
		break;
	case MHI_VANES_LR_LEFT_CENTER:
		retval = MHI_FRAME_VANES_LR_MODE_LEFT_CENTER;
		break;
	case MHI_VANES_LR_CENTER:
		retval = MHI_FRAME_VANES_LR_MODE_CENTER;
		break;
	case MHI_VANES_LR_RIGHT_CENTER:
		retval = MHI_FRAME_VANES_LR_MODE_RIGHT_CENTER;
		break;
	case MHI_VANES_LR_RIGHT:
		retval = MHI_FRAME_VANES_LR_MODE_RIGHT;
		break;
	case MHI_VANES_LR_WIDE:
		retval = MHI_FRAME_VANES_LR_MODE_WIDE;
		break;
	case MHI_VANES_LR_SPOT:
		retval = MHI_FRAME_VANES_LR_MODE_SPOT;
		break;
	default:
		break;
	}

	return MHI_FRAME_VANES_LR_MODE_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_vanes_lr mhi_vanes_lr_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_VANES_LR_ERROR;

	if (MHI_FRAME_VANES_LR_SWING_GET(frame) == MHI_FRAME_VANES_LR_SWING_ON)
		return MHI_VANES_LR_SWING;

	switch (MHI_FRAME_VANES_LR_MODE_GET(frame)) {
	case MHI_FRAME_VANES_LR_MODE_AUTO:
		return MHI_VANES_LR_AUTO;
	case MHI_FRAME_VANES_LR_MODE_LEFT:
		return MHI_VANES_LR_LEFT;
	case MHI_FRAME_VANES_LR_MODE_LEFT_CENTER:
		return MHI_VANES_LR_LEFT_CENTER;
	case MHI_FRAME_VANES_LR_MODE_CENTER:
		return MHI_VANES_LR_CENTER;
	case MHI_FRAME_VANES_LR_MODE_RIGHT_CENTER:
		return MHI_VANES_LR_RIGHT_CENTER;
	case MHI_FRAME_VANES_LR_MODE_RIGHT:
		return MHI_VANES_LR_RIGHT;
	case MHI_FRAME_VANES_LR_MODE_WIDE:
		return MHI_VANES_LR_WIDE;
	case MHI_FRAME_VANES_LR_MODE_SPOT:
		return MHI_VANES_LR_SPOT;
	}

	return MHI_VANES_LR_UNKNOWN;
}

const char *mhi_vanes_lr_str(const enum MHI_vanes_lr mode)
{
	switch (mode) {
	case MHI_VANES_LR_ERROR:
		return "error";
	case MHI_VANES_LR_UNKNOWN:
		return "unknown";
	case MHI_VANES_LR_SWING:
		return "swing";
	case MHI_VANES_LR_AUTO:
		return "auto";
	case MHI_VANES_LR_LEFT:
		return "left";
	case MHI_VANES_LR_LEFT_CENTER:
		return "left-center";
	case MHI_VANES_LR_CENTER:
		return "center";
	case MHI_VANES_LR_RIGHT_CENTER:
		return "right-center";
	case MHI_VANES_LR_RIGHT:
		return "right";
	case MHI_VANES_LR_WIDE:
		return "right";
	case MHI_VANES_LR_SPOT:
		return "right";
	}

	return "exception";
}

static uint8_t mhi_vanes_ud_swing_set(const enum MHI_vanes_ud vanes)
{
	if (vanes == MHI_VANES_UD_SWING)
		return MHI_FRAME_VANES_UD_SWING_SET(MHI_FRAME_VANES_UD_SWING_ON);

	return 0;
}

static uint8_t mhi_vanes_ud_set(const enum MHI_vanes_ud vanes)
{
  uint8_t retval = 0;

	switch (vanes) {
	case MHI_VANES_UD_AUTO:
		retval = MHI_FRAME_VANES_UD_MODE_AUTO;
		break;
	case MHI_VANES_UD_UP:
		retval = MHI_FRAME_VANES_UD_MODE_UP;
		break;
	case MHI_VANES_UD_UP_CENTER:
		retval = MHI_FRAME_VANES_UD_MODE_UP_CENTER;
		break;
	case MHI_VANES_UD_CENTER:
		retval = MHI_FRAME_VANES_UD_MODE_CENTER;
		break;
	case MHI_VANES_UD_DOWN_CENTER:
		retval = MHI_FRAME_VANES_UD_MODE_DOWN_CENTER;
		break;
	case MHI_VANES_UD_DOWN:
		retval = MHI_FRAME_VANES_UD_MODE_DOWN;
		break;
	default:
		break;
	}

	return MHI_FRAME_VANES_UD_MODE_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_vanes_ud mhi_vanes_ud_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_VANES_UD_ERROR;

	if (MHI_FRAME_VANES_UD_SWING_GET(frame) == MHI_FRAME_VANES_UD_SWING_ON)
		return MHI_VANES_UD_SWING;

	switch (MHI_FRAME_VANES_UD_MODE_GET(frame)) {
	case MHI_FRAME_VANES_UD_MODE_AUTO:
		return MHI_VANES_UD_AUTO;
	case MHI_FRAME_VANES_UD_MODE_UP:
		return MHI_VANES_UD_UP;
	case MHI_FRAME_VANES_UD_MODE_UP_CENTER:
		return MHI_VANES_UD_UP_CENTER;
	case MHI_FRAME_VANES_UD_MODE_CENTER:
		return MHI_VANES_UD_CENTER;
	case MHI_FRAME_VANES_UD_MODE_DOWN_CENTER:
		return MHI_VANES_UD_DOWN_CENTER;
	case MHI_FRAME_VANES_UD_MODE_DOWN:
		return MHI_VANES_UD_DOWN;
	}

	return MHI_VANES_UD_UNKNOWN;
}

const char *mhi_vanes_ud_str(const enum MHI_vanes_ud mode)
{
	switch (mode) {
	case MHI_VANES_UD_ERROR:
		return "error";
	case MHI_VANES_UD_UNKNOWN:
		return "unknown";
	case MHI_VANES_UD_SWING:
		return "swing";
	case MHI_VANES_UD_AUTO:
		return "auto";
	case MHI_VANES_UD_UP:
		return "up";
	case MHI_VANES_UD_UP_CENTER:
		return "up-center";
	case MHI_VANES_UD_CENTER:
		return "center";
	case MHI_VANES_UD_DOWN_CENTER:
		return "down-center";
	case MHI_VANES_UD_DOWN:
		return "down";
	}

	return "exception";
}

static uint8_t mhi_fan_set(const enum MHI_fan fan)
{
  uint8_t retval = 0;

	switch (fan) {
	case MHI_FAN_AUTO:
		retval = MHI_FRAME_FAN_AUTO;
		break;
	case MHI_FAN_QUIET:
		retval = MHI_FRAME_FAN_QUIET;
		break;
	case MHI_FAN_LOW:
		retval = MHI_FRAME_FAN_LOW;
		break;
	case MHI_FAN_MEDIUM:
		retval = MHI_FRAME_FAN_MEDIUM;
		break;
	case MHI_FAN_HIGH:
		retval = MHI_FRAME_FAN_HIGH;
		break;
	default:
		break;
	}

	return MHI_FRAME_FAN_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_fan mhi_fan_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_FAN_ERROR;

	switch (MHI_FRAME_FAN_GET(frame)) {
	case MHI_FRAME_FAN_QUIET:
		return MHI_FAN_QUIET;
	case MHI_FRAME_FAN_LOW:
		return MHI_FAN_LOW;
	case MHI_FRAME_FAN_MEDIUM:
		return MHI_FAN_MEDIUM;
	case MHI_FRAME_FAN_HIGH:
		return MHI_FAN_HIGH;
	case MHI_FRAME_FAN_AUTO:
		return MHI_FAN_AUTO;
	}

	return MHI_FAN_UNKNOWN;
}

const char *mhi_fan_str(const enum MHI_fan fan)
{
	switch (fan) {
	case MHI_FAN_ERROR:
		return "error";
	case MHI_FAN_UNKNOWN:
		return "unknown";
	case MHI_FAN_AUTO:
		return "auto";
	case MHI_FAN_QUIET:
		return "quiet";
	case MHI_FAN_LOW:
		return "low";
	case MHI_FAN_MEDIUM:
		return "medium";
	case MHI_FAN_HIGH:
		return "high";
	}

	return "exception";
}

static uint8_t mhi_mode_set(const enum MHI_mode mode)
{
  uint8_t retval = 0;

	switch (mode) {
	case MHI_MODE_AUTO:
		retval = MHI_FRAME_MODE_AUTO;
		break;
	case MHI_MODE_DRY:
		retval = MHI_FRAME_MODE_DRY;
		break;
	case MHI_MODE_COOL:
		retval = MHI_FRAME_MODE_COOL;
		break;
	case MHI_MODE_FAN:
		retval = MHI_FRAME_MODE_FAN;
		break;
	case MHI_MODE_HEAT:
		retval = MHI_FRAME_MODE_HEAT;
		break;
	default:
		break;
	}

	return MHI_FRAME_MODE_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_mode mhi_mode_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_MODE_ERROR;

	switch (MHI_FRAME_MODE_GET(frame)) {
	case MHI_FRAME_MODE_AUTO:
		return MHI_MODE_AUTO;
	case MHI_FRAME_MODE_DRY:
		return MHI_MODE_DRY;
	case MHI_FRAME_MODE_COOL:
		return MHI_MODE_COOL;
	case MHI_FRAME_MODE_FAN:
		return MHI_MODE_FAN;
	case MHI_FRAME_MODE_HEAT:
		return MHI_MODE_HEAT;
	}

	return MHI_MODE_UNKNOWN;
}

const char *mhi_mode_str(const enum MHI_mode mode)
{
	switch (mode) {
	case MHI_MODE_ERROR:
		return "error";
	case MHI_MODE_UNKNOWN:
		return "unknown";
	case MHI_MODE_AUTO:
		return "auto";
	case MHI_MODE_DRY:
		return "dry";
	case MHI_MODE_COOL:
		return "cool";
	case MHI_MODE_FAN:
		return "fan";
	case MHI_MODE_HEAT:
		return "heat";
	}

	return "exception";
}

static uint8_t mhi_power_set(const enum MHI_power power)
{
  uint8_t retval = 0;

	switch (power) {
	case MHI_POWER_ON:
		retval = MHI_FRAME_POWER_ON;
		break;
	case MHI_POWER_OFF:
		retval = MHI_FRAME_POWER_OFF;
		break;
  default:
		break;
	}

	return MHI_FRAME_POWER_SET(retval) | MHI_FRAME_SRC_SPI;
}

static enum MHI_power mhi_power_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return MHI_POWER_ERROR;

	switch (MHI_FRAME_POWER_GET(frame)) {
	case MHI_FRAME_POWER_ON:
		return MHI_POWER_ON;
	case MHI_FRAME_POWER_OFF:
		return MHI_POWER_OFF;
	}

	return MHI_POWER_UNKNOWN;
}

const char *mhi_power_str(const enum MHI_power power)
{
	switch (power) {
	case MHI_POWER_ERROR:
		return "error";
	case MHI_POWER_UNKNOWN:
		return "unknown";
	case MHI_POWER_ON:
		return "on";
	case MHI_POWER_OFF:
		return "off";
	}

	return "exception";
}

static uint8_t mhi_temperature_setpoint_set(const float temperature)
{
	return MHI_FRAME_TEMPERATURE_SETPOINT_SET(temperature * MHI_FRAME_TEMPERATURE_SETPOINT_MULTIPLIER);
}

static float mhi_temperature_setpoint_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return NAN;

	return (float)MHI_FRAME_TEMPERATURE_SETPOINT_GET(frame) / MHI_FRAME_TEMPERATURE_SETPOINT_MULTIPLIER;
}

static uint8_t mhi_temperature_room_set(const float temperature)
{
	return MHI_FRAME_TEMPERATURE_ROOM_SET((temperature * MHI_FRAME_TEMPERATURE_SETPOINT_MULTIPLIER) + MHI_FRAME_TEMPERATURE_ROOM_OFFSET);
}

static float mhi_temperature_room_get(const uint8_t *frame)
{
#ifdef MHI_USE_MOVING_AVERAGE_FILTER_TEMPERATURE_ROOM
	constexpr size_t window_size = (SLIDING_WINDOW_SIZE * (size_t)MHI_FRAME_TEMPERATURE_ROOM_MULTIPLIER) % UINT8_MAX;
	static uint8_t window[window_size] = { 0 };
	static bool initialized = false;
	static size_t window_slot = 0;
#endif

	if (frame == nullptr)
		return NAN;

#ifdef MHI_USE_MOVING_AVERAGE_FILTER_TEMPERATURE_ROOM
	return _moving_avg_filter(window, window_size, &window_slot,
	                          MHI_FRAME_TEMPERATURE_ROOM_GET(frame) - MHI_FRAME_TEMPERATURE_ROOM_OFFSET,
	                          MHI_FRAME_TEMPERATURE_ROOM_MULTIPLIER, &initialized);
#else
	return (float)(MHI_FRAME_TEMPERATURE_ROOM_GET(frame) - MHI_FRAME_TEMPERATURE_ROOM_OFFSET) / MHI_FRAME_TEMPERATURE_ROOM_MULTIPLIER;
#endif
}

static uint8_t mhi_last_error_get(const uint8_t *frame)
{
	if (frame == nullptr)
		return -1;

	return MHI_LAST_ERROR_GET(frame);
}

size_t mhi_frame_prepare(uint8_t *frame,
                         struct MHI_climate_settings *target,
                         const struct MHI_climate_settings *current,
                         const enum MHI_frame_size frame_size)
{
	static size_t frame_counter = 0;
	uint16_t checksum;

	if (frame == nullptr)
		return 0;

	if (target == nullptr)
		return 0;

	if (current == nullptr)
		return 0;

	frame_counter++;
	if (frame_counter == 0)
		frame_counter = 1;

	switch (frame_size) {
	case MHI_FRAME_SIZE_STANDARD:
		frame[MHI_FIELD_VERSION] = MHI_VERSION_STANDARD_MISO;
		break;
	case MHI_FRAME_SIZE_EXTENDED:
		frame[MHI_FIELD_VERSION] = MHI_VERSION_EXTENDED_MISO;
		break;
	default:
		return 0;
	}

	frame[MHI_FIELD_SIGNATURE_HI] = MHI_SIGNATURE_HI_MISO;
	frame[MHI_FIELD_SIGNATURE_LO] = MHI_SIGNATURE_LO_MISO;

	frame[MHI_FIELD_CTRL_0] = 0;
	frame[MHI_FIELD_CTRL_1] = 0;
	frame[MHI_FIELD_CTRL_2] = 0;
	frame[MHI_FIELD_CTRL_3] = 0;

	if (target->power_change_request) {
		if (current->power != target->power)
			frame[MHI_FIELD_CTRL_0] = mhi_power_set(target->power);
		else
			target->power_change_request = false;
	}

	if (target->mode_change_request) {
		if (current->mode != target->mode)
			frame[MHI_FIELD_CTRL_0] |= mhi_mode_set(target->mode);
		else
			target->mode_change_request = false;
	}

	if (target->vanes_ud_change_request) {
		if (current->vanes_ud != target->vanes_ud) {
			frame[MHI_FIELD_CTRL_0] |= mhi_vanes_ud_swing_set(target->vanes_ud);
			frame[MHI_FIELD_CTRL_1] |= mhi_vanes_ud_set(target->vanes_ud);
		} else {
			target->vanes_ud_change_request = false;
		}
	}

	/*
	 * After a AC-Main power cycle, the AC is not aware of 'extended' fan
	 * controls, switching to auto tells the AC we support extended modes.
	 * We will deal with the real request the next frame-cycle.
	 * https://github.com/absalom-muc/MHI-AC-Ctrl/issues/99#issuecomment-1339566177
	 */
	if (current->power == MHI_POWER_UNKNOWN) {
		frame[MHI_FIELD_CTRL_1] |= mhi_fan_set(MHI_FAN_AUTO);
		target->fan_change_request = true;
		if (target->fan == MHI_FAN_UNKNOWN)
			target->fan = MHI_FAN_QUIET;
	} else if (target->fan_change_request) {
		if (target->fan != current->fan)
			frame[MHI_FIELD_CTRL_1] |= mhi_fan_set(target->fan);
		else
			target->fan_change_request = false;
	}

	if (target->vanes_lr_change_request) {
		if (current->vanes_lr != target->vanes_lr) {
			frame[MHI_FIELD_CTRL_2] |= mhi_vanes_lr_set(target->vanes_lr);
			frame[MHI_FIELD_CTRL_3] |= mhi_vanes_lr_swing_set(target->vanes_lr);
		} else {
			target->vanes_lr_change_request = false;
		}
	}

	if (target->vanes_3d_auto_change_request) {
		if (current->vanes_3d_auto != target->vanes_3d_auto)
			frame[MHI_FIELD_CTRL_3] |= mhi_vanes_3d_auto_set(target->vanes_3d_auto);
		else
			target->vanes_3d_auto_change_request = false;
	}

	if (target->temperature_setpoint_change_request) {
		if (current->temperature_setpoint != target->temperature_setpoint)
			frame[MHI_FIELD_TEMPERATURE_SETPOINT] = mhi_temperature_setpoint_set(target->temperature_setpoint);
		else
			target->temperature_setpoint_change_request = false;
	}

	if (target->temperature_room_use_internal)
		frame[MHI_FIELD_TEMPERATURE_ROOM] = MHI_FRAME_TEMPERATURE_ROOM_INTERNAL;
	else
	//	frame[MHI_FIELD_TEMPERATURE_ROOM] = MHI_FRAME_TEMPERATURE_ROOM_SET(target->temperature_room);
		frame[MHI_FIELD_TEMPERATURE_ROOM] = frame_counter; // XXX fake it till you make it

//    0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
//  sb0   sb1   sb2   db0   db1   db2   db3   db4   db5   db6   db7   db8   db9  db10  db11  db12
// 0xA9, 0x00, 0x07, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x80, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
//   16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31
// db13  db14  chkH  chkL  db15  db16  db17  db18  db19  db20  db21  db22  db23  db24  db25  db26
// 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,

	frame[MHI_FIELD_OP_ADDR] = 0;
	switch (1) {
		case 1:
			frame[MHI_FIELD_OP_ADDR] |= MHI_FRAME_OP_ADDR_SET(MHI_FRAME_OP_ADDR_CTRL_1);
			frame[MHI_FIELD_OP_CMD] = MHI_FRAME_OP_CMD_CTRL;
			break;
	}
			frame[7] = 0x00;
			frame[8] = 0x00;
			frame[9] = 0x80;
			frame[10] = 0x00;
			frame[11] = 0x00;
			frame[12] = 0xff;
			frame[13] = 0xff;
			frame[14] = 0xff;
			frame[15] = 0xff;
			frame[16] = 0x0f;

	frame[MHI_FIELD_OP_DATA_RW] = MHI_FRAME_OP_DATA_RW(frame_counter);

	checksum = mhi_checksum_calc_standard(frame);
	frame[MHI_FIELD_CHECKSUM_STANDARD_HI] = CHECKSUM_HI_SET(checksum);
	frame[MHI_FIELD_CHECKSUM_STANDARD_LO] = CHECKSUM_LO_SET(checksum);

			frame[20] = 0x00;
			frame[21] = 0x00;
			frame[22] = 0x00;
			frame[23] = 0x00;
			frame[24] = 0x00;
			frame[25] = 0x00;
			frame[26] = 0x00;
			frame[27] = 0x00;
			frame[28] = 0xff;
			frame[29] = 0xff;
			frame[30] = 0xff;
			frame[31] = 0xff;
	checksum = mhi_checksum_calc_extended(frame, checksum);
	frame[MHI_FIELD_CHECKSUM_EXTENDED_LO] = CHECKSUM_LO_SET(checksum);

	return frame_counter;
}

enum MHI_frame_status mhi_frame_process(const uint8_t *frame,
                                        struct MHI_climate_settings *climate,
                                        struct MHI_operational_data *opdata)
{
	static uint8_t frame_prev[MHI_FRAME_SIZE_MAX] = { 0x00 };
	enum MHI_frame_size frame_size;
	uint16_t checksum;

	if (frame == nullptr)
		return MHI_FRAME_ERROR_NULL;

	if (climate == nullptr)
		return MHI_FRAME_ERROR_NULL;

	if (opdata == nullptr)
		return MHI_FRAME_ERROR_NULL;

	if (!memcmp(frame_prev, frame, MHI_FRAME_SIZE_MAX))
		return MHI_FRAME_DUPLICATE;

	memcpy(frame_prev, frame, MHI_FRAME_SIZE_MAX);

	if (!mhi_frame_version_valid(frame))
		return MHI_FRAME_ERROR_VERSION;

	frame_size = mhi_frame_size(frame);
	if (frame_size == MHI_FRAME_SIZE_UNKNOWN)
		return MHI_FRAME_ERROR_SIZE;

	if (!mhi_frame_signature_valid(frame))
		return MHI_FRAME_ERROR_SIGNATURE;

	checksum = mhi_checksum_calc_standard(frame);
	if (!mhi_checksum_standard_valid(frame, checksum))
		return MHI_FRAME_ERROR_CHECKSUM_STANDARD;

	climate->last_error = mhi_last_error_get(frame);

	climate->temperature_room = mhi_temperature_room_get(frame);

	climate->temperature_setpoint = mhi_temperature_setpoint_get(frame);
	climate->temperature_setpoint_src = mhi_temperature_setpoint_src(frame);

	climate->power = mhi_power_get(frame);
	climate->power_src = mhi_power_src(frame);

	climate->mode = mhi_mode_get(frame);
	climate->mode_src = mhi_mode_src(frame);

	climate->fan = mhi_fan_get(frame);
	climate->fan_src = mhi_fan_src(frame);

	climate->vanes_ud = mhi_vanes_ud_get(frame);
	climate->vanes_ud_swing_src = mhi_vanes_ud_swing_src(frame);
	climate->vanes_ud_mode_src = mhi_vanes_ud_mode_src(frame);

	climate->compressor_power = mhi_compressor_power_get(frame);
	climate->compressor_mode = mhi_compressor_mode_get(frame);
	climate->compressor_state = mhi_compressor_state_get(frame);

	if (frame_size == MHI_FRAME_SIZE_EXTENDED) {
		checksum = mhi_checksum_calc_extended(frame, checksum);
		if (!mhi_checksum_extended_valid(frame, checksum))
			return MHI_FRAME_ERROR_CHECKSUM_EXTENDED;

		climate->vanes_lr = mhi_vanes_lr_get(frame);
		climate->vanes_lr_swing_src = mhi_vanes_lr_swing_src(frame);
		climate->vanes_lr_mode_src = mhi_vanes_lr_mode_src(frame);

		climate->vanes_3d_auto = mhi_vanes_3d_auto_get(frame);
		climate->vanes_3d_auto_src = mhi_vanes_3d_auto_src(frame);
	}

	return MHI_FRAME_OK;
}
