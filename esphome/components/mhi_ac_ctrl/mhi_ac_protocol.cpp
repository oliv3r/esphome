/* SPDX-License: */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mhi_ac_protocol.h"

#define FIELD_VERSION 00
constexpr uint8_t VERSION_STANDARD_MOSI = 0x6C;
constexpr uint8_t VERSION_STANDARD_MISO = 0xA9;

constexpr uint8_t VERSION_EXTENDED_MOSI = 0x6D;
constexpr uint8_t VERSION_EXTENDED_MISO = 0xAA;

#define FIELD_SIGNATURE_HI 01
constexpr uint8_t SIGNATURE_HI_MOSI = 0x80;
constexpr uint8_t SIGNATURE_HI_MISO = 0x00;

#define FIELD_SIGNATURE_LO 02
constexpr uint8_t SIGNATURE_LO_MOSI = 0x04;
constexpr uint8_t SIGNATURE_LO_MISO = 0x07;

constexpr uint8_t MHI_FRAME_BYTE_IDLE = 0xFF;


static uint16_t checksum_calc(uint8_t *frame, uint16_t seed,
			      size_t start, size_t len)
{
	uint16_t checksum = seed;

	for (size_t i = start; i < len; i++)
		checksum += frame[i];

	return checksum;
}

/*
 * Note: Only works for the first byte after a frame sync.
 */
enum MHI_frame_status frame_size_get(uint8_t frame_byte)
{
	switch (frame_byte) {
	case VERSION_STANDARD_MOSI:
		return MHI_FRAME_STATUS_SIZE_STANDARD;
	case VERSION_EXTENDED_MOSI:
		return MHI_FRAME_STATUS_SIZE_EXTENDED;
	default:
		return MHI_FRAME_STATUS_ERROR;
	}
}

bool frame_size_valid(enum MHI_frame_status frame_size)
{
	switch (frame_size) {
	case MHI_FRAME_STATUS_SIZE_STANDARD:
                /* fall-through */
	case MHI_FRAME_STATUS_SIZE_EXTENDED:
		return true;
	default:
		return false;
	}
}

const char *frame_size_str(enum MHI_frame_status frame_size)
{
	switch (frame_size) {
	case MHI_FRAME_STATUS_SIZE_STANDARD:
		return "Standard";
	case MHI_FRAME_STATUS_SIZE_EXTENDED:
		return "Extended";
	default:
		return "Unknown";
	}
}

void mhi_ac_ctrl_frame_prepare(uint8_t *frame,
			       struct MHI_climate_settings *settings,
			       enum MHI_frame_status frame_size,
			       bool active_mode)
{
	//												 sb0	 sb1	 sb2	 db0	 db1	 db2	 db3	 db4	 db5	 db6	 db7	 db8	 db9	db10	db11
	//												 db12  db13  db14  chkH  chkL  db15  db16  db17  db18  db19  db20  db21  db22  db23  db24
	//												 db25  db26  chk2L
	// uint8_t frame[MHI_FRAME_MAX] = { 0xA9, 0x00, 0x07, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
	// 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	// 0x22 };
	memset(frame, MHI_FRAME_BYTE_IDLE, MHI_FRAME_STATUS_SIZE_MAX);

	if (!active_mode)
		return;

	switch (frame_size) {
	case MHI_FRAME_STATUS_SIZE_STANDARD:
		frame[FIELD_VERSION] = VERSION_STANDARD_MISO;
		break;
	case MHI_FRAME_STATUS_SIZE_EXTENDED:
		frame[FIELD_VERSION] = VERSION_EXTENDED_MISO;
		break;
	default:
		frame[FIELD_VERSION] = MHI_FRAME_BYTE_IDLE;
		return;
	}
}

enum MHI_frame_status mhi_ac_ctrl_frame_process(uint8_t *frame,
						struct MHI_climate_settings *settings)
{
        static uint8_t frame_prev[MHI_FRAME_STATUS_SIZE_MAX] = { 0x00 };
	enum MHI_frame_status frame_size;

	if (frame == nullptr)
		return MHI_FRAME_STATUS_ERROR;

	if (settings == nullptr)
		return MHI_FRAME_STATUS_ERROR;

	if (memcmp(frame_prev, frame, MHI_FRAME_STATUS_SIZE_MAX))
		memcpy(frame_prev, frame, MHI_FRAME_STATUS_SIZE_MAX);
        else
		return MHI_FRAME_STATUS_DUPLICATE;

	frame_size = frame_size_get(frame[FIELD_VERSION]);
	if (!frame_size_valid(frame_size))
		return frame_size;

	// if (frame_signature_valid(&frame)

	return frame_size;
}
