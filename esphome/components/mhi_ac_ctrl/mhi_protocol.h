/* SPDX-License: */

#ifndef _MHI_PROTOCOL__
#define _MHI_PROTOCOL__ (__file__)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr uint8_t MHI_FRAME_BYTE_IDLE = 0xFF;
constexpr size_t MHI_FRAME_SYNC_MS = 5;
constexpr size_t SLIDING_WINDOW_SIZE = 16; /* Multiplied by the internal MHI multiplier */

enum MHI_frame_status {
	MHI_FRAME_OK = 0,
	MHI_FRAME_DUPLICATE,
	MHI_FRAME_ERROR,
	MHI_FRAME_ERROR_NULL,
	MHI_FRAME_ERROR_VERSION,
	MHI_FRAME_ERROR_SIZE,
	MHI_FRAME_ERROR_SIGNATURE,
	MHI_FRAME_ERROR_CHECKSUM_STANDARD,
	MHI_FRAME_ERROR_CHECKSUM_EXTENDED,
};

enum MHI_frame_size : uint8_t {
	MHI_FRAME_SIZE_UNKNOWN = 0,
	MHI_FRAME_SIZE_STANDARD = 20,
	MHI_FRAME_SIZE_EXTENDED = 33,
	MHI_FRAME_SIZE_MAX,
};

struct MHI_operational_data {
	enum MHI_frame_size frame_format;
};

enum MHI_ctrl_src {
	MHI_CTRL_SRC_ERROR,
	MHI_CTRL_SRC_UNKNOWN,
	MHI_CTRL_SRC_IR,
	MHI_CTRL_SRC_SPI,
};

enum MHI_compressor_power {
	MHI_COMPRESSOR_POWER_ERROR,
	MHI_COMPRESSOR_POWER_UNKNOWN,
	MHI_COMPRESSOR_POWER_OFF,
	MHI_COMPRESSOR_POWER_ON,
};

enum MHI_compressor_mode {
	MHI_COMPRESSOR_MODE_ERROR,
	MHI_COMPRESSOR_MODE_UNKNOWN,
	MHI_COMPRESSOR_MODE_COOL,
	MHI_COMPRESSOR_MODE_HEAT,
};

enum MHI_compressor_state {
	MHI_COMPRESSOR_STATE_ERROR,
	MHI_COMPRESSOR_STATE_UNKNOWN,
	MHI_COMPRESSOR_STATE_IDLE,
	MHI_COMPRESSOR_STATE_RUNNING,
};

enum MHI_power {
	MHI_POWER_ERROR,
	MHI_POWER_UNKNOWN,
	MHI_POWER_OFF,
	MHI_POWER_ON,
};

enum MHI_mode {
	MHI_MODE_ERROR,
	MHI_MODE_UNKNOWN,
	MHI_MODE_AUTO,
	MHI_MODE_DRY,
	MHI_MODE_COOL,
	MHI_MODE_FAN,
	MHI_MODE_HEAT,
};

enum MHI_vanes_3d_auto {
	MHI_VANES_3D_AUTO_ERROR,
	MHI_VANES_3D_AUTO_UNKNOWN,
	MHI_VANES_3D_AUTO_ON,
	MHI_VANES_3D_AUTO_OFF,
};

enum MHI_vanes_ud {
	MHI_VANES_UD_ERROR,
	MHI_VANES_UD_UNKNOWN,
	MHI_VANES_UD_SWING,
	MHI_VANES_UD_UP,
	MHI_VANES_UD_UP_CENTER,
	MHI_VANES_UD_CENTER,
	MHI_VANES_UD_DOWN_CENTER,
	MHI_VANES_UD_DOWN,
	MHI_VANES_UD_AUTO,
};

enum MHI_vanes_lr {
	MHI_VANES_LR_ERROR,
	MHI_VANES_LR_UNKNOWN,
	MHI_VANES_LR_AUTO,
	MHI_VANES_LR_LEFT,
	MHI_VANES_LR_LEFT_CENTER,
	MHI_VANES_LR_CENTER,
	MHI_VANES_LR_RIGHT_CENTER,
	MHI_VANES_LR_RIGHT,
	MHI_VANES_LR_WIDE,
	MHI_VANES_LR_SPOT,
	MHI_VANES_LR_SWING,
};

enum MHI_fan {
	MHI_FAN_ERROR,
	MHI_FAN_UNKNOWN,
	MHI_FAN_QUIET,
	MHI_FAN_LOW,
	MHI_FAN_MEDIUM,
	MHI_FAN_HIGH,
	MHI_FAN_AUTO,
};

struct MHI_climate_settings {
	uint8_t last_error;

	enum MHI_compressor_power compressor_power;
	enum MHI_compressor_mode compressor_mode;
	enum MHI_compressor_state compressor_state;

	enum MHI_power power;
	enum MHI_ctrl_src power_src;
	bool power_change_request;

	enum MHI_mode mode;
	enum MHI_ctrl_src mode_src;
	bool mode_change_request;

	enum MHI_fan fan;
	enum MHI_ctrl_src fan_src;
	bool fan_change_request;

	enum MHI_vanes_ud vanes_ud;
	enum MHI_ctrl_src vanes_ud_mode_src;
	enum MHI_ctrl_src vanes_ud_swing_src;
	bool vanes_ud_change_request;

	enum MHI_vanes_lr vanes_lr;
	enum MHI_ctrl_src vanes_lr_mode_src;
	enum MHI_ctrl_src vanes_lr_swing_src;
	bool vanes_lr_change_request;

	enum MHI_vanes_3d_auto vanes_3d_auto;
	enum MHI_ctrl_src vanes_3d_auto_src;
	bool vanes_3d_auto_change_request;

	float temperature_setpoint;
	enum MHI_ctrl_src temperature_setpoint_src;
	bool temperature_setpoint_change_request;

	bool temperature_room_use_internal;
	float temperature_room;
};

const char *mhi_strerror(const enum MHI_frame_status frame_status);
enum MHI_frame_size mhi_frame_size(const uint8_t *frame);
const char *mhi_frame_size_str(const enum MHI_frame_size frame_size);
const char *mhi_compressor_power_str(const enum MHI_compressor_power power);
const char *mhi_compressor_state_str(const enum MHI_compressor_state state);
const char *mhi_compressor_mode_str(const enum MHI_compressor_mode mode);
const char *mhi_vanes_3d_auto_str(const enum MHI_vanes_3d_auto mode);
const char *mhi_fan_str(const enum MHI_fan fan);
const char *mhi_vanes_ud_str(const enum MHI_vanes_ud mode);
const char *mhi_vanes_lr_str(const enum MHI_vanes_lr mode);
const char *mhi_power_str(const enum MHI_power power);
const char *mhi_mode_str(const enum MHI_mode mode);
const char *mhi_ctrl_src_str(const enum MHI_ctrl_src src);
void mhi_climate_defaults(struct MHI_climate_settings *settings);
void mhi_opdata_defaults(struct MHI_operational_data *opdata);
size_t mhi_frame_prepare(uint8_t *frame,
                         struct MHI_climate_settings *target,
                         const struct MHI_climate_settings *current,
                         const enum MHI_frame_size frame_size);
enum MHI_frame_status mhi_frame_process(const uint8_t *frame,
                                        struct MHI_climate_settings *settings,
                                        struct MHI_operational_data *opdata);

#endif /* _MHI_PROTOCOL__ */
