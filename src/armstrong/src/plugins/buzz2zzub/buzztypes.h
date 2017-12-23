#pragma once

#define BUZZ_PLUGIN_FLAGS_MASK (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output)
#define BUZZ_ROOT_PLUGIN_FLAGS (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input)
#define BUZZ_GENERATOR_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_output)
#define BUZZ_EFFECT_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output)

class CPattern {
public:
	std::string name;
	int length;
};

class CMachine {
public:
	// Jeskola Buzz compatible CMachine header.
	// Some machines look up these by reading directly from zzub::metaplugin memory.

	char _placeholder[16];
	char* _internal_name;					// 0x14: polac's VST reads this string, set to 0
	char _placeholder2[52];
	void* _internal_machine;				// pointer to CMachine*, scanned for by some plugins
	void* _internal_machine_ex;				// 0x50: same as above, but is not scanned for
	char _placeholder3[20];
	char* _internal_global_state;			// 0x68: copy of machines global state
	char* _internal_track_state;			// 0x6C: copy of machines track state
	char _placeholder4[120];
	int _internal_seqCommand;				// 0xE8: used by mooter, 0 = --, 1 = mute, 2 = thru
	char _placeholder6[17];
	bool hardMuted;							// 0xFD: true when muted by user, used by mooter

	// End of Buzz compatible header

	zzub_plugin_t* plugin;
	CMachineInfo* buzzinfo;

	CMachine() {
		plugin = 0;
		buzzinfo = 0;
	}
	virtual ~CMachine() { }

	static bool checkBuzzCompatibility() {
		// check offsets that may be used for known hacks
		int nameofs = offsetof(CMachine, _internal_name);			// 0x14 / 0x18 (+/- vtbl)
		int exofs = offsetof(CMachine, _internal_machine_ex);		// 0x50
		int gstateofs = offsetof(CMachine, _internal_global_state);	// 0x68
		int tstateofs = offsetof(CMachine, _internal_track_state);	// 0x6c
		//int xofs = offsetof(CMachine, x);							// 0xa8
		//int yofs = offsetof(CMachine, y);							// 0xac
		int seqcmdofs = offsetof(CMachine, _internal_seqCommand); // 0xe8
		int hardmuteofs = offsetof(CMachine, hardMuted);			// 0xfd

		if (exofs != 0x50) return false;
		if (gstateofs != 0x68) return false;
		if (tstateofs != 0x6c) return false;
		
		if (seqcmdofs != 0xe8) return false;
		if (hardmuteofs != 0xfd) return false;
		return true;
	}
};

namespace buzz2zzub {

struct plugin;
struct buzzplugininfo;

class CMachineManager {
public:
	std::map<zzub_plugin_t*, CMachine*> plugin_to_machine;
	std::map<CMachine*, zzub_plugin_t*> machine_to_plugin;

	CMachine* get(zzub_player_t* host, zzub_plugin_t* metaplugin);
	CMachine* create(buzz2zzub::plugin* plugin);
	CMachine* create(zzub::plugin* plugin);
	void destroy(zzub_plugin_t* metaplugin);
	bool exists(CMachine* pmac);
};

class CMachineDataInputWrap : public CMachineDataInput {
public:
	zzub::instream* pi;

	CMachineDataInputWrap(zzub::instream *pi) {
		this->pi = pi;
	}

	virtual void Read(void *pbuf, int const numbytes) {
		if (pi->position()+numbytes <= pi->size())
			pi->read(pbuf, numbytes);
	}
};

class CMachineDataOutputWrap : public CMachineDataOutput {
public:
	zzub::outstream* po;

	CMachineDataOutputWrap(zzub::outstream *po) {
		this->po = po;
	}

	virtual void Write(void *pbuf, int const numbytes) {
		po->write(pbuf, numbytes);
	}
};

class outstreamwrap : public zzub::outstream {
public:
	CMachineDataOutput* po;

	outstreamwrap(CMachineDataOutput *po) {
		this->po = po;
	}

	virtual int write(void *buffer, int size) {
		po->Write(buffer, size);
		return size;
	}

	virtual long position() {
		assert(false);
		return 0;
	}

	virtual void seek(long, int) {
		assert(false);
	}
};

struct libwrap : public zzub::lib {
	CLibInterface* blib;
	buzz2zzub::buzzplugininfo* info;

	libwrap(CLibInterface* mlib, buzz2zzub::buzzplugininfo* _info) ;
	virtual void get_instrument_list(zzub::outstream* os);
};

};
