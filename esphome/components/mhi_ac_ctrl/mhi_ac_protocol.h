/* SPDX-License: */

#ifndef _MHI_AC_PROTOCOL__
#define _MHI_AC_PROTOCOL__ (__file__)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr size_t MHI_FRAME_SYNC_MS = 5;

enum MHI_frame_status {
	MHI_FRAME_STATUS_ERROR = -1,
	MHI_FRAME_STATUS_DUPLICATE = 0,
	MHI_FRAME_STATUS_SIZE_STANDARD = 20,
	MHI_FRAME_STATUS_SIZE_EXTENDED = 33,
	MHI_FRAME_STATUS_SIZE_MAX,
};

struct MHI_climate_settings {
	uint8_t stuff;
};

enum MHI_frame_status frame_size_get(uint8_t frame_byte);
bool frame_size_valid(enum MHI_frame_status frame_status);
const char *frame_size_str(enum MHI_frame_status frame_size);
void mhi_ac_ctrl_frame_prepare(uint8_t *frame,
			       struct MHI_climate_settings *settings,
			       enum MHI_frame_status frame_size,
			       bool active_mode);
enum MHI_frame_status mhi_ac_ctrl_frame_process(uint8_t *frame,
						struct MHI_climate_settings *settings);

#endif /* _MHI_AC_PROTOCOL__ */
