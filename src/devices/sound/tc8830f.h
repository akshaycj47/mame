// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    tc8830f.h - Toshiba TC8830F, CMOS voice recording/reproducing LSI

***************************************************************************/

#pragma once

#ifndef __TC8830F_H__
#define __TC8830F_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TC8830F_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TC8830F, _clock)

#define MCFG_TC8830F_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, TC8830F, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tc8830f_device : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	tc8830f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void reset();
	void write_p(uint8_t data);

	sound_stream *m_stream;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	bool m_playing;
	uint32_t m_address;
	uint32_t m_stop_address;
	uint8_t m_bitcount;
	uint8_t m_bitrate;
	uint8_t m_prevbits;
	int m_delta;
	int m_output;
	uint8_t m_command;
	int m_cmd_rw;
	uint8_t m_phrase;

	required_region_ptr<uint8_t> m_mem;
};


// device type definition
extern const device_type TC8830F;

#endif /* __TC8830F_H__ */
