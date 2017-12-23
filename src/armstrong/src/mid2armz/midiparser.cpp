#include <algorithm>
#include <cassert>
#include "midiparser.h"

// http://stackoverflow.com/questions/1080297/how-does-midi-tempo-message-apply-to-other-tracks
// -> http://home.roadrunner.com/~jgglatt/tech/midifile.htm
// http://www.sonicspot.com/guide/midifiles.html

template <typename T>
void endian_reverse(T& c) {
	T v;
	char* p = (char*)&v;
	for (int i = 0; i < sizeof(T); i++) {
		p[i] = ((char*)&c)[sizeof(T) - 1 - i];
	}
	c = v;
}

int unpack_value(std::istream& strm) {
	int result;
	unsigned char c;
	strm.read((char*)&c, 1);
	if ((c & 0x80) == 0) return c;
	result = (c & 0x7F) << 7;
	
	strm.read((char*)&c, 1);
	if ((c & 0x80) == 0) return result | c;
	result |= (c & 0x7F) << 7;

	strm.read((char*)&c, 1);
	if ((c & 0x80) == 0) return result | c;
	result |= (c & 0x7F) << 7;

	strm.read((char*)&c, 1);
	return result | (c & 0x7f);
}

void miditrack::get_unique_deltas(std::vector<int>& deltas) {
	int delta = 0;
	int lasttime = 0;
	for (std::vector<midievent>::iterator i = events.begin(); i != events.end(); ++i) {
		delta = i->timestamp - lasttime;
		if (delta == 0) continue;
		std::vector<int>::iterator d = std::find(deltas.begin(), deltas.end(), delta);
		if (d == deltas.end()) {
			deltas.push_back(delta);
		}
		lasttime = i->timestamp;
	}
}

// armstrong pattern resolution = header.delta / get_time_multiplier()
int miditrack::get_time_multiplier(int global_delta) {
	std::vector<int> deltas;
	deltas.push_back(global_delta);
	get_unique_deltas(deltas);

	int mindelta = 0;
	for (std::vector<int>::iterator i = deltas.begin(); i != deltas.end(); ++i)
		if (mindelta == 0 || *i < mindelta)
			mindelta = *i;

	int mindenom = 1;
	for (int j = 2; j <= mindelta; j++) {
		bool passed = true;
		for (std::vector<int>::iterator i = deltas.begin(); i != deltas.end(); ++i) {
			int rem = *i % j;
			if (rem != 0) {
				passed = false;
				break;
			}
		}
		if (passed) mindenom = j;
	}
	return mindenom;
}

void miditrack::get_tempo_changes(std::vector<midievent*>& tempoevents) {
	for (std::vector<midievent>::iterator i = events.begin(); i != events.end(); ++i) {
		if (i->command == 0xf && i->metacommand == 0x51) {
			tempoevents.push_back(&*i);
		}
	}
}

int miditrack::get_length() {
	if (events.size() == 0) return 0;
	return events.back().timestamp;
}

int miditrack::get_channel_polyphony(int channel) {
	channelmanager trkmgr;
	int polycount = 0;
	int lasttime = 0;

	for (std::vector<midievent>::iterator i = events.begin(); i != events.end(); ++i) {
		if (i->timestamp - lasttime > 0) {
			lasttime = i->timestamp;
			trkmgr.clear_released();
		}
		if (i->channel == channel) {
			switch (i->command) {
				case 8:
					// note off
					trkmgr.release_channel(i->nv.note);
					break;
				case 9:
					if (i->nv.velocity != 0) {
						// note on
						trkmgr.allocate_channel(i->nv.note);
					} else {
						// note off
						trkmgr.release_channel(i->nv.note);
					}
					break;
			}

			int currentpoly = trkmgr.get_allocated_channel_count();
			if (currentpoly > polycount) polycount = currentpoly;
		}
	}
	return polycount;
}


bool midifile::parse(std::istream& strm) {
	strm.read((char*)&header, sizeof(MIDIFILEHEADER));
	if (*(unsigned int*)header.magic != *(unsigned int*)"MThd") return false;
	endian_reverse(header.size);
	endian_reverse(header.format);
	endian_reverse(header.track_count);
	endian_reverse(header.delta);

	for (int i = 0; i < header.track_count; i++) {
		tracks.push_back(miditrack());
		if (!parse_track(strm, tracks.back())) return false;
	}
	return true;
}

bool midifile::parse_track(std::istream& strm, miditrack& track) {
	unsigned char command, channel;
	unsigned short delta;
	int timestamp = 0;

	strm.read((char*)&track.header, sizeof(MIDITRACKHEADER));
	if (*(unsigned int*)track.header.magic != *(unsigned int*)"MTrk") return false;
	endian_reverse(track.header.size);

	int lastcommand = 0;
	for (;;) {
		int delta;

		delta = unpack_value(strm);
		timestamp += delta;

		if (strm.peek() & 0x80) {
			strm.read((char*)&command, 1);
			lastcommand = command;
		} else {
			// running status
			command = lastcommand;
		}

		midievent midiev;
		midiev.timestamp = timestamp;
		midiev.command = (command & 0xF0) >> 4;
		midiev.channel = command & 0xF;

		parse_command(strm, track, midiev);
		if (midiev.command == 0) break;
		if (midiev.command == -1) continue;

		track.events.push_back(midiev);

	}
	return true;
}

bool midifile::parse_command(std::istream& strm, miditrack& track, midievent& midiev) {
	unsigned char note, velocity;
	unsigned short delta;

	switch (midiev.command) {
		case 0x8: // note off
		case 0x9: // note on
		case 0xA: // key after-touch
		case 0xB: // control change
		case 0xE: // pitch wheel change
			strm.read((char*)&note, 1);
			strm.read((char*)&velocity, 1);
			midiev.nv.note = note;
			midiev.nv.velocity = velocity;
			break;
		case 0xC: // program change
			strm.read((char*)&note, 1);
			midiev.program = note;
			midiev.command = -1; // ignore
			break;
		case 0xD: // channel after-touch
			strm.read((char*)&note, 1);
			midiev.command = -1; // ignore
			break;
		case 0xF:
			if (midiev.channel == 0x0 || midiev.channel == 0x7) {
				// sysex (0xF0 and 0xF7)
				int sysexsize = unpack_value(strm);
				for (int i = 0; i < sysexsize; i++) {
					char t;
					strm.read(&t, 1);
				}
				midiev.command = -1; // ignore
			} else
			if (midiev.channel == 0xF) {
				// meta event
				parse_metacommand(strm, track, midiev);
			} else {
				assert(false); // not sysex nor meta
			}
			break;
		default:
			//cout << "unhandled midi thingy" << endl;
			assert(false);
			return false;
	}

	return true;
}

bool midifile::parse_metacommand(std::istream& strm, miditrack& track, midievent& midiev) {

	unsigned char metacommand, metasize, num, denom, met, q32;
	std::vector<char> tempbuf;
	unsigned int tempo;

	strm.read((char*)&metacommand, 1);
	strm.read((char*)&metasize, 1);
	midiev.metacommand = metacommand;
	switch (metacommand) {
		case 0x03: // track name
			tempbuf.resize(metasize);
			strm.read(&tempbuf.front(), metasize);
			track.name = std::string(tempbuf.begin(), tempbuf.end());//, metasize);
			break;
		case 0x01: // any text
		case 0x02: // copyright text
		case 0x04: // instrument name
		case 0x05: // lyrics
		case 0x06: // text marker
		case 0x07: // cue text
			for (int i = 0; i < metasize; i++) {
				char t;
				strm.read(&t, 1);
			}
			midiev.command = -1; // ignore
			break;
		case 0x2f: // end of track
			assert(metasize == 0);
			midiev.command = 0; // finish
			break;
		case 0x51: // tempo
			assert(metasize == 3);
			tempo = 0;
			strm.read((char*)&tempo, 3);
			endian_reverse(tempo);
			tempo >>= 8;
			//To convert the MIDI file format's tempo (ie, the 3 bytes that specify the amount of microseconds per quarter note) to BPM:
			//BPM = 60,000,000/(tt tt tt)
			midiev.t.musec = tempo;
			break;
		case 0x58: // time signature
			assert(metasize == 4);
			strm.read((char*)&num, 1);
			strm.read((char*)&denom, 1);
			strm.read((char*)&met, 1);
			strm.read((char*)&q32, 1);
			midiev.ts.num = num;
			midiev.ts.denom = denom;
			midiev.ts.met = met;
			midiev.ts.q32 = q32;
			//cout << "time signature: " << (int)num << "/" << (int)denom << ", met=" << (int)met << ", q32=" << (int)q32 << endl;
			break;
		case 0x59: // key signature
		case 0x7f: // host specific
		default:
			midiev.command = -1; // ignore
			for (int i = 0; i < metasize; i++) {
				char t;
				strm.read(&t, 1);
			}
			break;
	}

	return true;
}

