#include <string>
#include <vector>
#include <cmath>
#include <zzub/zzub.h>
#include "modfile/module.h"

namespace {	// duplicate from ccm.h and pattern.cpp, mixer.cpp

int midi_to_buzz_note(int value) {
	if (value == 255 || value == 254) return value;
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

// from http://www.winehq.com/hypermail/wine-patches/2002/12/0179.html
inline short u8tos16(unsigned char b) {	// wavs are unsigned
	return (short)((b+(b << 8))-32768);
}

inline short s8tos16(char b) {	// mods are singed
	return ((short)b) * 256;
}

}

// Effect column commands
#define CMD_NONE				0
#define CMD_ARPEGGIO			1
#define CMD_PORTAMENTOUP		2
#define CMD_PORTAMENTODOWN		3
#define CMD_TONEPORTAMENTO		4
#define CMD_VIBRATO				5
#define CMD_TONEPORTAVOL		6
#define CMD_VIBRATOVOL			7
#define CMD_TREMOLO				8
#define CMD_PANNING8			9
#define CMD_OFFSET				10
#define CMD_VOLUMESLIDE			11
#define CMD_POSITIONJUMP		12
#define CMD_VOLUME				13
#define CMD_PATTERNBREAK		14
#define CMD_RETRIG				15
#define CMD_SPEED				16
#define CMD_TEMPO				17
#define CMD_TREMOR				18
#define CMD_MODCMDEX			19
#define CMD_S3MCMDEX			20
#define CMD_CHANNELVOLUME		21
#define CMD_CHANNELVOLSLIDE		22
#define CMD_GLOBALVOLUME		23
#define CMD_GLOBALVOLSLIDE		24
#define CMD_KEYOFF				25
#define CMD_FINEVIBRATO			26
#define CMD_PANBRELLO			27
#define CMD_XFINEPORTAUPDOWN	28
#define CMD_PANNINGSLIDE		29
#define CMD_SETENVPOSITION		30
#define CMD_MIDI				31
#define CMD_SMOOTHMIDI			32 //rewbs.smoothVST
#define CMD_VELOCITY			33 //rewbs.velocity
#define CMD_XPARAM				34 // -> CODE#0010 -> DESC="add extended parameter mechanism to pattern effects" -! NEW_FEATURE#0010
#define CMD_NOTESLIDEUP         35 // IMF Gxy
#define CMD_NOTESLIDEDOWN       36 // IMF Hxy
#define MAX_EFFECTS				37

void ConvertModCommand(int& command, int& param)
//-----------------------------------------------------
{
	//UINT command = m->command, param = m->param;

	switch(command)
	{
	case 0x00:	if (param) command = CMD_ARPEGGIO; break;
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_TONEPORTAVOL; if (param & 0xF0) param &= 0xF0; break;
	case 0x06:	command = CMD_VIBRATOVOL; if (param & 0xF0) param &= 0xF0; break;
	case 0x07:	command = CMD_TREMOLO; break;
	case 0x08:	command = CMD_PANNING8; break;
	case 0x09:	command = CMD_OFFSET; break;
	case 0x0A:	command = CMD_VOLUMESLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_VOLUME; break;
	case 0x0D:	command = CMD_PATTERNBREAK; param = ((param >> 4) * 10) + (param & 0x0F); break;
	case 0x0E:	command = CMD_MODCMDEX; break;
//	case 0x0F:	command = (param <= (UINT)((sf->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? 0x1F : 0x20)) ? CMD_SPEED : CMD_TEMPO;
//				if ((param == 0xFF) && (sf->m_nSamples == 15) && (sf->m_nType & MOD_TYPE_MOD)) command = 0; break; //<rewbs> what the hell is this?! :) //<jojo> it's the "stop tune" command! :-P
	// Extension for XM extended effects
	case 'G' - 55:	command = CMD_GLOBALVOLUME; break;		//16
	case 'H' - 55:	command = CMD_GLOBALVOLSLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 'K' - 55:	command = CMD_KEYOFF; break;
	case 'L' - 55:	command = CMD_SETENVPOSITION; break;
	case 'M' - 55:	command = CMD_CHANNELVOLUME; break;
	case 'N' - 55:	command = CMD_CHANNELVOLSLIDE; break;
	case 'P' - 55:	command = CMD_PANNINGSLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 'R' - 55:	command = CMD_RETRIG; break;
	case 'T' - 55:	command = CMD_TREMOR; break;
	case 'X' - 55:	command = CMD_XFINEPORTAUPDOWN;	break;
	case 'Y' - 55:	command = CMD_PANBRELLO; break;			//34
	case 'Z' - 55:	command = CMD_MIDI;	break;				//35
	case '\\' - 56:	command = CMD_SMOOTHMIDI;	break;		//rewbs.smoothVST: 36
	case ':' - 21:	command = CMD_VELOCITY;	break;			//rewbs.velocity: 37
	case '#' + 3:	command = CMD_XPARAM;	break;			//rewbs.XMfixes - XParam is 38
	default:	command = 0;
	}
}

void S3MConvert(int& command, int& param, bool bIT)
//--------------------------------------------------------
{
	//UINT command = m->command;
	//UINT param = m->param;
	switch (command + 0x40)
	{
	case 'A':	command = CMD_SPEED; break;
	case 'B':	command = CMD_POSITIONJUMP; break;
	case 'C':	command = CMD_PATTERNBREAK; if (!bIT) param = (param >> 4) * 10 + (param & 0x0F); break;
	case 'D':	command = CMD_VOLUMESLIDE; break;
	case 'E':	command = CMD_PORTAMENTODOWN; break;
	case 'F':	command = CMD_PORTAMENTOUP; break;
	case 'G':	command = CMD_TONEPORTAMENTO; break;
	case 'H':	command = CMD_VIBRATO; break;
	case 'I':	command = CMD_TREMOR; break;
	case 'J':	command = CMD_ARPEGGIO; break;
	case 'K':	command = CMD_VIBRATOVOL; break;
	case 'L':	command = CMD_TONEPORTAVOL; break;
	case 'M':	command = CMD_CHANNELVOLUME; break;
	case 'N':	command = CMD_CHANNELVOLSLIDE; break;
	case 'O':	command = CMD_OFFSET; break;
	case 'P':	command = CMD_PANNINGSLIDE; break;
	case 'Q':	command = CMD_RETRIG; break;
	case 'R':	command = CMD_TREMOLO; break;
	case 'S':	command = CMD_S3MCMDEX; break;
	case 'T':	command = CMD_TEMPO; break;
	case 'U':	command = CMD_FINEVIBRATO; break;
	case 'V':	command = CMD_GLOBALVOLUME; break;
	case 'W':	command = CMD_GLOBALVOLSLIDE; break;
	case 'X':	command = CMD_PANNING8; break;
	case 'Y':	command = CMD_PANBRELLO; break;
	case 'Z':	command = CMD_MIDI; break;
	default:	command = 0;
	}
}

void convert_effect(modimport::fxtype type, int fx, int fxvalue, int track, int row, zzub_plugin_t* tracker, zzub_plugin_t* sequence, zzub_pattern_t* pattern) {
	int tracker_id = zzub_plugin_get_id(tracker);
	int sequence_id = zzub_plugin_get_id(sequence);
	switch (type) {
		case modimport::screamtracker:
			switch (fx) {
				case 1: // set tempo/speed
					if (fxvalue == 0) {
						// tempo 0 = ignore
					} else if (fxvalue < 0x20) { // speed -> tpb
						int tpb = (int)floor((6.0f / (float)fxvalue) * 4.0f + 0.5f); // ??? works for 6 and 3
						zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 1, row, tpb, 0);
					} else
					if (fxvalue >= 0x20) { // bpm
						zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 0, row, fxvalue, 0);
					}
					break;
				default:
					S3MConvert(fx, fxvalue, false);
					if (fx != 0) {
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 4, row, fx, 0);
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 5, row, fxvalue, 0);
					}
					break;
			}
			break;
		case modimport::impulsetracker:
			switch (fx) {
				case 1: // set tempo/speed
					if (fxvalue == 0) {
						// tempo 0 = ignore
					} else if (fxvalue < 0x20) { // speed -> tpb
						int tpb = (int)floor((6.0f / (float)fxvalue) * 4.0f + 0.5f); // ??? works for 6 and 3
						zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 1, row, tpb, 0);
					} else
					if (fxvalue >= 0x20) { // bpm
						zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 0, row, fxvalue, 0);
					}
					break;
				default:
					S3MConvert(fx, fxvalue, true);
					if (fx != 0) {
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 4, row, fx, 0);
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 5, row, fxvalue, 0);
					}
					break;
			}
			break;
		case modimport::protracker:
			switch (fx) {
					case 0xC: // set volume
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 2, row, 1, 0);
						zzub_pattern_insert_value(pattern, tracker_id, 2, track, 3, row, fxvalue, 0);
						break;
					case 0xF: // set tempo/speed
						if (fxvalue == 0) {
							// tempo 0 = ignore
						} else if (fxvalue < 0x20) { // speed -> tpb
							int tpb = (int)floor((6.0f / (float)fxvalue) * 4.0f + 0.5f); // ??? works for 6 and 3
							zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 1, row, tpb, 0);
						} else if (fxvalue >= 0x20) { // bpm
							zzub_pattern_insert_value(pattern, sequence_id, 1, 0, 0, row, fxvalue, 0);
						}
						break;
					default:
						ConvertModCommand(fx, fxvalue);
						if (fx != 0) {
							zzub_pattern_insert_value(pattern, tracker_id, 2, track, 4, row, fx, 0);
							zzub_pattern_insert_value(pattern, tracker_id, 2, track, 5, row, fxvalue, 0);
						}
						break;
			}
			break;
	}
}

extern "C" int zzub_player_load_mod(zzub_player_t* player, const char* filename) {
	modimport::module* m = modimport::module::create(filename);
	if (!m) return -1;

	zzub_plugin_t* master = zzub_player_get_plugin_by_name(player, "Master");
	if (master == 0) return -1;

	zzub_pluginloader_t* sequenceloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/sequence");
	if (sequenceloader == 0) return -1;

	zzub_plugin_t* sequence = zzub_player_create_plugin(player, 0, 0, "Sequence", sequenceloader, 0);
	if (sequence == 0) return -1;

	zzub_pluginloader_t* trackerloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/modplug;1");
	//zzub_pluginloader_t* trackerloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/buzz2zzub/Matilde+Tracker2");
	if (trackerloader == 0) return -1;

	zzub_plugin_t* tracker = zzub_player_create_plugin(player, 0, 0, "modfile", trackerloader, 0);
	if (tracker == 0) return -1;

	if (m->type() == modimport::screamtracker)
		zzub_plugin_set_attribute_value(tracker, 0, 3);
	else if (m->type() == modimport::impulsetracker)
		zzub_plugin_set_attribute_value(tracker, 0, 1);
	else if (m->type() == modimport::fasttracker)
		zzub_plugin_set_attribute_value(tracker, 0, 2);
	else
		zzub_plugin_set_attribute_value(tracker, 0, 4);

	zzub_plugin_set_position(tracker, 0.5, 0.5);
	zzub_plugin_set_position(sequence, 0.5, -0.5);
	zzub_plugin_create_audio_connection(master, tracker, -1, -1, -1, -1);

	int track_count = m->pattern_tracks(0);
	int columns = m->pattern_track_columns(0);

	int notecolumn = -1, ampcolumn = -1, wavecolumn = -1, fxcolumn = -1, fxvalcolumn = -1;
	for (int i = 0; i < columns; i++) {
		modimport::column_type type = (modimport::column_type)m->pattern_column_type(0, i);
		switch (type) {
			case modimport::column_type_note:
				notecolumn = i;
				break;
			case modimport::column_type_wave:
				wavecolumn = i;
				break;
			case modimport::column_type_volume:
				ampcolumn = i;
				break;
			case modimport::column_type_effect:
				fxcolumn = i;
				break;
			case modimport::column_type_byte:
				fxvalcolumn = i;
				break;
		}
	}

	modimport::fxtype fxset = m->type();

	zzub_plugin_set_track_count(tracker, track_count);

	const char* pluginname = zzub_plugin_get_name(tracker);
	zzub_pattern_format_t* trackerformat = zzub_player_create_pattern_format(player, pluginname);

	zzub_pattern_format_add_column(trackerformat, sequence, 1, 0, 0, -1);	// bpm
	zzub_pattern_format_add_column(trackerformat, sequence, 1, 0, 1, -1);	// tpb
	for (int i = 0; i < track_count; i++) {
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 0, -1);	// note
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 1, -1);	// wave
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 2, -1);	// ampfx
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 3, -1);	// amp
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 4, -1);	// fx
		zzub_pattern_format_add_column(trackerformat, tracker, 2, i, 5, -1);	// fxval
	}

	int tracker_id = zzub_plugin_get_id(tracker);
	std::vector<zzub_pattern_t*> patterns;
	for (int i = 0; i < m->pattern_count(); i++) {
		int rows = m->pattern_rows(i);
		zzub_pattern_t* pat = zzub_player_create_pattern(player, trackerformat, "", rows);
		patterns.push_back(pat);

		for (int j = 0; j < track_count; j++) {
			for (int k = 0; k < columns; k++) {
				for (int l = 0; l < rows; l++) {
					int v = m->pattern_column_value(i, j * columns + k, l);
					if (v != 0) {
						if (k == notecolumn) {
							zzub_pattern_insert_value(pat, tracker_id, 2, j, 0, l, midi_to_buzz_note(v), 0);
						} 
						else if (k == wavecolumn) {
							zzub_pattern_insert_value(pat, tracker_id, 2, j, 1, l, v, 0);
						}
						else if (k == ampcolumn) {
							zzub_pattern_insert_value(pat, tracker_id, 2, j, 2, l, 1, 0); // ampfx1 = set volume
							zzub_pattern_insert_value(pat, tracker_id, 2, j, 3, l, v, 0);
						}
					}
					if (k == fxcolumn) {
						// TODO: translate fx and value
						int fxvalue = m->pattern_column_value(i, j * columns + fxvalcolumn, l);
						convert_effect(fxset, v, fxvalue, j, l, tracker, sequence, pat);
					}
				}
			}
		}
	}

	for (int i = 0; i < m->instrument_count(); ++i) {
		zzub_wave_t* wave = zzub_player_get_wave(player, i);
		zzub_wave_clear(wave);

		for (int j = 0; j < m->sample_count(i); j++) {
			zzub_wavelevel_t* wavelevel = zzub_wave_add_level(wave);
			int samplesize = m->sample_bytes(i, j);
			int bits = m->sample_bits_per_sample(i, j);
			int buffersize = bits == 8 ? samplesize * 2 : samplesize; // reserve extra space for in-buffer 8-to-16-bit conversion
			char* buffer = new char[buffersize];
			m->sample_seek(i, j);
			m->sample_read(buffer, samplesize);
			int numsamples = m->sample_samples(i, j);
			bool is_signed = m->sample_signed(i, j);
			bool is_looping = m->sample_looping(i, j);
			bool is_bidir = m->sample_bidir_looping(i, j);
			bool is_stereo = m->sample_stereo(i, j);
			int channels = is_stereo ? 2 : 1;
			int loop_start = m->sample_loop_start(i, j);
			int loop_end = m->sample_loop_end(i, j);
			float amp = m->sample_amp(i, j);
			
			int flags = 0;
			if (is_stereo) flags |= zzub_wave_flag_stereo;
			if (is_looping) flags |= zzub_wave_flag_loop;
			if (is_bidir) flags |= zzub_wave_flag_pingpong;

			// convert to 16 bit signed shorts if input is 8 bit
			if (bits == 8) {
				if (is_signed) {
					for (int i = numsamples - 1; i >= 0; i--) {
						((signed short*)buffer)[i * channels] = s8tos16(buffer[i * channels]);
						if (is_stereo)
							((signed short*)buffer)[i * channels + 1] = s8tos16(buffer[i * channels + 1]);
					}
				} else {
					for (int i = numsamples - 1; i>=0; i--) {
						((signed short*)buffer)[i * channels] = u8tos16(buffer[i * channels]);
						if (is_stereo)
							((signed short*)buffer)[i * channels + 1] = u8tos16(buffer[i * channels + 1]);
					}
				}
			}

			zzub_wave_set_flags(wave, flags);
			zzub_wave_set_volume(wave, amp);
			zzub_wave_set_name(wave, m->instrument_name(i).c_str());
			// NOTE: insert_sample_range before setting loop points - insert_sample_range shifts the loop
			zzub_wavelevel_insert_sample_range(wavelevel, 0, buffer, channels, zzub_wave_buffer_type_si16, numsamples); 
			zzub_wavelevel_set_loop_start(wavelevel, loop_start);
			zzub_wavelevel_set_loop_end(wavelevel, loop_end);
			zzub_wavelevel_set_samples_per_second(wavelevel, m->sample_samplespersecond(i, j));
		}
	}

	zzub_player_set_order_length(player, m->order_length());
	zzub_player_set_order_loop_end(player, m->order_length() - 1);
	for (int i = 0; i < m->order_length(); i++) {
		int patindex = m->order_pattern(i);
		if (patindex < patterns.size())
			zzub_player_set_order_pattern(player, i, patterns[patindex]);
	}

	m->close();
	delete m;

	zzub_player_history_commit(player, 0, 0, "Import Module");
	return 0;
}
