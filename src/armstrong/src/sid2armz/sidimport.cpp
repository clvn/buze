/*
SIDDump V1.04 by Cadaver (loorni@gmail.com)

Version history:

V1.0    - Original
V1.01   - Fixed BIT instruction
V1.02   - Added incomplete illegal opcode support, enough for John Player
V1.03   - Some CPU bugs fixed
V1.04   - Improved command line handling, added illegal NOP instructions, fixed 
          illegal LAX to work again

modified to produce output as an .armz using the lunar SID emulator plugin

for reference:
http://www.waitingforfriday.com/index.php/Commodore_SID_6581_Datasheet#SID_Control_Registers
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <zzub/zzub.h>
#include "cpu.h"

#define MAX_INSTR 0x1000000

typedef struct
{
  unsigned short freq;
  unsigned short pulse;
  unsigned short adsr;
  unsigned char wave;
  int note;
} CHANNEL;

typedef struct
{
  unsigned short cutoff;
  unsigned char ctrl;
  unsigned char type;
} FILTER;

int main(int argc, char **argv);
unsigned char readbyte(FILE *f);
unsigned short readword(FILE *f);

CHANNEL chn[3];
CHANNEL prevchn[3];
CHANNEL prevchn2[3];
FILTER filt;
FILTER prevfilt;

char *notename[] =
 {"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7"};

char *filtername[] =
 {"Off", "Low", "Bnd", "L+B", "Hi ", "L+H", "B+H", "LBH"};

unsigned char freqtbllo[] = {
  0x17,0x27,0x39,0x4b,0x5f,0x74,0x8a,0xa1,0xba,0xd4,0xf0,0x0e,
  0x2d,0x4e,0x71,0x96,0xbe,0xe8,0x14,0x43,0x74,0xa9,0xe1,0x1c,
  0x5a,0x9c,0xe2,0x2d,0x7c,0xcf,0x28,0x85,0xe8,0x52,0xc1,0x37,
  0xb4,0x39,0xc5,0x5a,0xf7,0x9e,0x4f,0x0a,0xd1,0xa3,0x82,0x6e,
  0x68,0x71,0x8a,0xb3,0xee,0x3c,0x9e,0x15,0xa2,0x46,0x04,0xdc,
  0xd0,0xe2,0x14,0x67,0xdd,0x79,0x3c,0x29,0x44,0x8d,0x08,0xb8,
  0xa1,0xc5,0x28,0xcd,0xba,0xf1,0x78,0x53,0x87,0x1a,0x10,0x71,
  0x42,0x89,0x4f,0x9b,0x74,0xe2,0xf0,0xa6,0x0e,0x33,0x20,0xff};

unsigned char freqtblhi[] = {
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x04,
  0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
  0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1f,0x20,
  0x22,0x24,0x27,0x29,0x2b,0x2e,0x31,0x34,0x37,0x3a,0x3e,0x41,
  0x45,0x49,0x4e,0x52,0x57,0x5c,0x62,0x68,0x6e,0x75,0x7c,0x83,
  0x8b,0x93,0x9c,0xa5,0xaf,0xb9,0xc4,0xd0,0xdd,0xea,0xf8,0xff};


void create_sequence(zzub_player_t* player, zzub_plugin_t** seqplugin, zzub_pattern_t** seqpattern) {
	const char* sequri = "@zzub.org/sequence/sequence";
	zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, sequri);
	assert(loader);

	const char* plugname = zzub_player_get_new_plugin_name(player, sequri);
	*seqplugin = zzub_player_create_plugin(player, 0, 0, plugname, loader, 0);

	zzub_plugin_set_parameter_value(*seqplugin, 1, 0, 0, 188, 0); // bpm
	zzub_plugin_set_parameter_value(*seqplugin, 1, 0, 1, 16, 0); // tpb
	zzub_plugin_tick(*seqplugin, 0);
}

void init_armz(zzub_player_t* player, int numframes, zzub_plugin_t** sidplugin, zzub_pattern_t** pat) {

	zzub_pattern_t* seqpattern;
	zzub_plugin_t* seqplugin;
	create_sequence(player, &seqplugin, &seqpattern);

	zzub_pluginloader_t* sidloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/lunar/sid;1");
	assert(sidloader);

	*sidplugin = zzub_player_create_plugin(player, 0, 0, "SID", sidloader, 0);
	zzub_plugin_set_track_count(*sidplugin, 3);

	zzub_plugin_t* masterplugin = zzub_player_get_plugin_by_name(player, "Master");
	zzub_plugin_create_audio_connection(masterplugin, *sidplugin, -1, -1, -1, -1);
	
	const char* name = zzub_plugin_get_name(*sidplugin);
	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, name);

	int screenidx = 0;
	// add all global params
	for (int i = 0; i < 4; i++) {
		zzub_pattern_format_add_column(patfmt, *sidplugin, 1, 0, i, screenidx);
		++screenidx;
	}
	// add all track params
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < zzub_plugin_get_parameter_count(*sidplugin, 2, j); i++) {
			zzub_pattern_format_add_column(patfmt, *sidplugin, 2, j, i, screenidx);
			++screenidx;
		}
	}

	*pat = zzub_player_create_pattern(player, patfmt, "SID", numframes);

	zzub_player_set_order_length(player, 1);
	zzub_player_set_order_pattern(player, 0, *pat);
	//zzub_pattern_set_value(seqpattern, 0, seqplugin, 2, 0, 0, zzub_pattern_get_id(*pat), 0);

}

int armznote(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

// track
const int sid_param_note = 0;
//const int sid_param_wave = 4;
// sync = 6
// ringmod = 7
// gate, sync, ringmod, test, triangle, saw, square, noise
const int sid_param_gate = 1;
const int sid_param_sync = 2;
const int sid_param_ringmod = 3;
const int sid_param_test = 4;
const int sid_param_triangle = 5;
const int sid_param_saw = 6;
const int sid_param_square = 7;
const int sid_param_noise = 8;
const int sid_param_pulse = 9;
const int sid_param_filter_enable = 10;

const int sid_param_attack = 11;
const int sid_param_decay = 12;
const int sid_param_sustain = 13;
const int sid_param_release = 14;

// globals
const int sid_param_cutoff = 0;
const int sid_param_reso = 1;
const int sid_param_filter_type = 2;
const int sid_param_volume = 3;

int zzub_player_import_sid(zzub_player_t* player, const char* sidname, int seconds, int subtune) {
	//int subtune = 0;
	//int seconds = 60;
	int instr = 0;
	int frames = 0;
	int spacing = 0;
	int pattspacing = 0;
	int firstframe = 0;
	int counter = 0;
	int basefreq = 0;
	int basenote = 0xb0;
	int lowres = 0;
	int rows = 0;
	int oldnotefactor = 1;
	int timeseconds = 0;
	int usage = 0;
	unsigned loadend;
	unsigned loadpos;
	unsigned loadsize;
	unsigned loadaddress;
	unsigned initaddress;
	unsigned playaddress;
	unsigned dataoffset;
	FILE *in;

	int c;

	zzub_plugin_t* sidplugin;
	zzub_pattern_t* sidpattern;
	init_armz(player, seconds * 50, &sidplugin, &sidpattern);

	int sidpluginid = zzub_plugin_get_id(sidplugin);

	// Recalibrate frequencytable
	if (basefreq)
	{
	basenote &= 0x7f;
	if ((basenote < 0) || (basenote > 96))
	{
  		printf("Warning: Calibration note out of range. Aborting recalibration.\n");
	}
	else
	{
		for (c = 0; c < 96; c++)
		{
    		double note = c - basenote;
    		double freq = (double)basefreq * pow(2.0, note/12.0);
    		int f = (int)freq;
    		if (freq > 0xffff) freq = 0xffff;
    		freqtbllo[c] = f & 0xff;
    		freqtblhi[c] = f >> 8;
		}
	}
	}

	// Check other parameters for correctness
	if ((lowres) && (!spacing)) lowres = 0;

	in = fopen(sidname, "rb");
	if (!in)
	{
	printf("Error: couldn't open SID file.\n");
	return 1;
	}

	// Read interesting parts of the SID header
	fseek(in, 6, SEEK_SET);
	dataoffset = readword(in);
	loadaddress = readword(in);
	initaddress = readword(in);
	playaddress = readword(in);
	fseek(in, dataoffset, SEEK_SET);
	if (loadaddress == 0)
	loadaddress = readbyte(in) | (readbyte(in) << 8);

	// Load the C64 data
	loadpos = ftell(in);
	fseek(in, 0, SEEK_END);
	loadend = ftell(in);
	fseek(in, loadpos, SEEK_SET);
	loadsize = loadend - loadpos;
	if (loadsize + loadaddress >= 0x10000)
	{
	printf("Error: SID data continues past end of C64 memory.\n");
	fclose(in);
	return 1;
	}
	fread(&mem[loadaddress], loadsize, 1, in);
	fclose(in);

	// Print info & run initroutine
	printf("Load address: $%04X Init address: $%04X Play address: $%04X\n", loadaddress, initaddress, playaddress);
	printf("Calling initroutine with subtune %d\n", subtune);
	initcpu(initaddress, subtune, 0, 0);
	instr = 0;
	while (runcpu())
	{
	instr++;
	if (instr > MAX_INSTR)
	{
  		printf("Error: CPU executed abnormally high amount of instructions.\n");
  		return 1;
	}
	}

	// Clear channelstructures in preparation & print first time info
	memset(&chn, 0, sizeof chn);
	memset(&filt, 0, sizeof filt);
	memset(&prevchn, 0, sizeof prevchn);
	memset(&prevchn2, 0, sizeof prevchn2);
	memset(&prevfilt, 0, sizeof prevfilt);
	printf("Calling playroutine for %d frames, starting from frame %d\n", seconds*50, firstframe);
	printf("Middle C frequency is $%04X\n\n", freqtbllo[48] | (freqtblhi[48] << 8));
	printf("| Frame | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | FCut RC Typ V |\n");
	printf("+-------+---------------------------+---------------------------+---------------------------+---------------+\n");

	// Data collection & display loop
	while (frames < firstframe + seconds*50)
	{
		int c;

		// Run the playroutine
		instr = 0;
		initcpu(playaddress, 0, 0, 0);
		while (runcpu())
		{
			instr++;
			if (instr > MAX_INSTR)
			{
    			printf("Error: CPU executed abnormally high amount of instructions.\n");
    			return 1;
			}
		}

		// Get SID parameters from each channel and the filter
		for (c = 0; c < 3; c++)
		{
			chn[c].freq = mem[0xd400 + 7*c] | (mem[0xd401 + 7*c] << 8);
			chn[c].pulse = (mem[0xd402 + 7*c] | (mem[0xd403 + 7*c] << 8)) & 0xfff;
			chn[c].wave = mem[0xd404 + 7*c];
			chn[c].adsr = mem[0xd406 + 7*c] | (mem[0xd405 + 7*c] << 8);
		}
		filt.cutoff = (mem[0xd415] & 0x7) | (mem[0xd416] << 3);
		filt.ctrl = mem[0xd417];
		filt.type = mem[0xd418];

		// Frame display
		if (frames >= firstframe)
		{
			char output[512];
			int time = frames - firstframe;
			output[0] = 0;      

			if (!timeseconds)
			sprintf(&output[strlen(output)], "| %5d | ", time);
			else
			sprintf(&output[strlen(output)], "|%01d:%02d.%02d| ", time/3000, (time/50)%60, time%50);

			// Loop for each channel
			for (c = 0; c < 3; c++)
			{
				int newnote = 0;

				// Keyoff-keyon sequence detection
				if (chn[c].wave >= 0x10)
				{
					if ((chn[c].wave & 1) && ((!(prevchn2[c].wave & 1)) || (prevchn2[c].wave < 0x10)))
        				prevchn[c].note = -1;
				}

				// Frequency
				if ((frames == firstframe) || (prevchn[c].note == -1) || (chn[c].freq != prevchn[c].freq))
				{
					int d;
					int dist = 0x7fffffff;
					int delta = ((int)chn[c].freq) - ((int)prevchn2[c].freq);

					sprintf(&output[strlen(output)], "%04X ", chn[c].freq);

					if (chn[c].wave >= 0x10)
					{
					// Get new note number
					for (d = 0; d < 96; d++)
					{
						int cmpfreq = freqtbllo[d] | (freqtblhi[d] << 8);
						int freq = chn[c].freq;

						if (abs(freq - cmpfreq) < dist)
						{
						dist = abs(freq - cmpfreq);
						// Favor the old note
						if (d == prevchn[c].note) dist /= oldnotefactor;
						chn[c].note = d;
						}
					}

					// Print new note
					if (chn[c].note != prevchn[c].note)
					{
						if (prevchn[c].note == -1)
						{
             				if (lowres) newnote = 1;
             				sprintf(&output[strlen(output)], " %s %02X  ", notename[chn[c].note], chn[c].note | 0x80);
							zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_note, time, armznote(chn[c].note), 0);
						}
						else {
							sprintf(&output[strlen(output)], "(%s %02X) ", notename[chn[c].note], chn[c].note | 0x80);
							zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_note, time, armznote(chn[c].note), 0);
						}
					}
					else
					{
						// If same note, print frequency change (slide/vibrato)
						if (delta)
						{
						if (delta > 0)
               				sprintf(&output[strlen(output)], "(+ %04X) ", delta);
						else
               				sprintf(&output[strlen(output)], "(- %04X) ", -delta);
						}
						else sprintf(&output[strlen(output)], " ... ..  ");
					}
					}
					else sprintf(&output[strlen(output)], " ... ..  ");
				}
				else sprintf(&output[strlen(output)], "....  ... ..  ");

				// Waveform
				if ((frames == firstframe) || (newnote) || (chn[c].wave != prevchn[c].wave)) {
					sprintf(&output[strlen(output)], "%02X ", chn[c].wave);
					// wave -> 8 bit params:
					// gate, sync, ringmod, test, triangle, saw, square, noise

					int bitvalue, prevbitvalue;
#define test_and_set(n) \
	bitvalue = ((chn[c].wave & (1 << n)) != 0) ? 1 : 0;\
	prevbitvalue = ((prevchn[c].wave & (1 << n)) != 0) ? 1 : 0;\
	if (frames == firstframe || bitvalue != prevbitvalue)\
		zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_gate + n, time, bitvalue, 0);

					test_and_set(0);
					test_and_set(1);
					test_and_set(2);
					test_and_set(3);
					test_and_set(4);
					test_and_set(5);
					test_and_set(6);
					test_and_set(7);
#undef test_and_set
					/*int bitvalue = ((chn[c].wave & (1 << 0)) != 0) ? 1 : 0;
					int prevbitvalue = ((prevchn[c].wave & (1 << 0)) != 0) ? 1 : 0;
					if (bitvalue && bitvalue != prevbitvalue)
						zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_wave, time, bitvalue, 0);*/
				} 
				else sprintf(&output[strlen(output)], ".. ");

				// ADSR
				if ((frames == firstframe) || (newnote) || (chn[c].adsr != prevchn[c].adsr)) {
					sprintf(&output[strlen(output)], "%04X ", chn[c].adsr);
					//unsigned char attackdecay = (voices[t].attack << 4) | voices[t].decay;
					//unsigned char sustainrelease = (voices[t].sustain << 4) | voices[t].release;
					unsigned char attack = (chn[c].adsr & 0xf000) >> 12;
					unsigned char decay = (chn[c].adsr & 0xf00) >> 8;
					unsigned char sustain = (chn[c].adsr & 0xf0) >> 4;
					unsigned char release = (chn[c].adsr & 0xf) >> 0;
					unsigned char prevattack = (prevchn[c].adsr & 0xf000) >> 12;
					unsigned char prevdecay = (prevchn[c].adsr & 0xf00) >> 8;
					unsigned char prevsustain = (prevchn[c].adsr & 0xf0) >> 4;
					unsigned char prevrelease = (prevchn[c].adsr & 0xf) >> 0;

					if ((frames == firstframe) || attack != prevattack)
						zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_attack, time, attack, 0);

					if ((frames == firstframe) || decay != prevdecay)
						zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_decay, time, decay, 0);

					if ((frames == firstframe) || sustain != prevsustain)
						zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_sustain, time, sustain, 0);

					if ((frames == firstframe) || release != prevrelease)
						zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_release, time, release, 0);

				}
				else sprintf(&output[strlen(output)], ".... ");

				// Pulse
				if ((frames == firstframe) || (newnote) || (chn[c].pulse != prevchn[c].pulse)) {
					sprintf(&output[strlen(output)], "%03X ", chn[c].pulse);
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, c, sid_param_pulse, time, chn[c].pulse, 0);
				}
				else sprintf(&output[strlen(output)], "... ");

				sprintf(&output[strlen(output)], "| ");
			}

			// Filter cutoff
			if ((frames == firstframe) || (filt.cutoff != prevfilt.cutoff)) {
				sprintf(&output[strlen(output)], "%04X ", filt.cutoff);
				zzub_pattern_insert_value(sidpattern, sidpluginid, 1, 0, sid_param_cutoff, time, filt.cutoff, 0);
			}
			else sprintf(&output[strlen(output)], ".... ");

			// Filter control
			if ((frames == firstframe) || (filt.ctrl != prevfilt.ctrl)) {
				sprintf(&output[strlen(output)], "%02X ", filt.ctrl);
				int res = filt.ctrl >> 4;
				int f = filt.ctrl & 0xf;
				zzub_pattern_insert_value(sidpattern, sidpluginid, 1, 0, sid_param_reso, time, res, 0);
				if ((f & 0x1) != 0)
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 0, sid_param_filter_enable, time, 1, 0); else
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 0, sid_param_filter_enable, time, 0, 0);

				if ((f & 0x2) != 0)
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 1, sid_param_filter_enable, time, 1, 0); else
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 1, sid_param_filter_enable, time, 0, 0);

				if ((f & 0x4) != 0)
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 2, sid_param_filter_enable, time, 1, 0); else
					zzub_pattern_insert_value(sidpattern, sidpluginid, 2, 2, sid_param_filter_enable, time, 0, 0);
			}
			else sprintf(&output[strlen(output)], ".. ");

			// Filter passband
			if ((frames == firstframe) || ((filt.type & 0x70) != (prevfilt.type & 0x70))) {
				sprintf(&output[strlen(output)], "%s ", filtername[(filt.type >> 4) & 0x7]);
				zzub_pattern_insert_value(sidpattern, sidpluginid, 1, 0, sid_param_filter_type, time, (filt.type >> 4) & 0x3, 0);
			}
			else sprintf(&output[strlen(output)], "... ");

			// Mastervolume
			if ((frames == firstframe) || ((filt.type & 0xf) != (prevfilt.type & 0xf))) {
				sprintf(&output[strlen(output)], "%01X ", filt.type & 0xf);
				zzub_pattern_insert_value(sidpattern, sidpluginid, 1, 0, sid_param_volume, time, filt.type & 0xf, 0);
			}
			else sprintf(&output[strlen(output)], ". ");
		    
			// End of frame display, print info so far and copy SID registers to old registers
			sprintf(&output[strlen(output)], "|\n");
			if ((!lowres) || (!((frames - firstframe) % spacing)))
			{
				printf("%s", output);
				for (c = 0; c < 3; c++)
				{
					prevchn[c] = chn[c];
				}
				prevfilt = filt;
			}
			for (c = 0; c < 3; c++) prevchn2[c] = chn[c];

			// Print note/pattern separators
			if (spacing)
			{
			counter++;
			if (counter >= spacing)
			{
				counter = 0;
				if (pattspacing)
				{
					rows++;
					if (rows >= pattspacing)
					{
          				rows = 0;
						printf("+=======+===========================+===========================+===========================+===============+\n");
					}
					else
						if (!lowres) printf("+-------+---------------------------+---------------------------+---------------------------+---------------+\n");
					}
					else
					if (!lowres) printf("+-------+---------------------------+---------------------------+---------------------------+---------------+\n");
				}
			}
		}

		// Advance to next frame
		frames++;
	}
	return 0;
}

unsigned char readbyte(FILE *f)
{
  unsigned char res;

  fread(&res, 1, 1, f);
  return res;
}

unsigned short readword(FILE *f)
{
  unsigned char res[2];

  fread(&res, 2, 1, f);
  return (res[0] << 8) | res[1];
}





int main(int argc, char **argv) {

	if (argc != 5) {
		printf("usage: sid2armz sidfile armzfile seconds songnum\n\n");
		return 1;
	}

	const char* sidfile = argv[1];
	const char* armzfile = argv[2];
	int seconds = atoi(argv[3]);
	int songnum = atoi(argv[4]);

	zzub_player_t* player = zzub_player_create(0, 0, 0);
	zzub_player_initialize(player, 0);

	if (zzub_player_import_sid(player, sidfile, seconds, songnum) != 0) {
		printf("cant open sid");
		return 2;
	}

	zzub_player_save_armz(player, armzfile, 0, 0, 0);

	zzub_player_destroy(player);

	return 0;
}
