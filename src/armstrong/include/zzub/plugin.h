// zzub Plugin Interface
// Copyright (C) 2006 Leonard Ritter (contact@leonard-ritter.com)
// Copyright (C) 2006-2013 Anders Ervik
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#if !defined(__ZZUBPLUGIN_H)
#define __ZZUBPLUGIN_H

#include <vector>
#include <string>
#include <cassert>
#include "pluginenum.h"

#if !defined(NO_ZZUB_PLAYER_TYPE)
typedef struct _zzub_player zzub_player_t;
#endif

#if !defined(NO_ZZUB_MIXER_TYPE)
typedef struct _zzub_mixer zzub_mixer_t;
#endif

#if !defined(NO_ZZUB_PLUGIN_TYPE)
typedef struct _zzub_plugin zzub_plugin_t;
#endif

namespace zzub {

struct parameter {
	zzub_parameter_type type;
	std::string name;
	std::string description;
	int value_min;
	int value_max;
	int value_none;
	int flags;
	int value_default;
	
	parameter() {
		type = zzub_parameter_type_switch;
		value_min = 0;
		value_max = 0;
		value_none = 0;
		flags = 0;
		value_default = 0;
	}
	
	parameter &set_type(zzub_parameter_type type) { this->type = type; return *this; }
	parameter &set_note() {
		this->type = zzub_parameter_type_note;
		this->name = "Note";
		this->description = "Note";
		this->value_min = zzub_note_value_min;
		this->value_max = zzub_note_value_max;
		this->value_none = zzub_note_value_none;
		this->value_default = this->value_none;
		return *this;
	}
	parameter &set_switch() {
		this->type = zzub_parameter_type_switch;
		this->name = "Switch";
		this->description = "Switch";
		this->value_min = zzub_switch_value_off;
		this->value_max = zzub_switch_value_on;
		this->value_none = zzub_switch_value_none;
		this->value_default = this->value_none;
		return *this;
	}
	parameter &set_byte() {
		this->type = zzub_parameter_type_byte;
		this->name = "Byte";
		this->description = "Byte";
		this->value_min = 0;
		this->value_max = 128;
		this->value_none = 255;
		this->value_default = this->value_none;
		return *this;
	}
	parameter &set_word() {
		this->type = zzub_parameter_type_word;
		this->name = "Word";
		this->description = "Word";
		this->value_min = 0;
		this->value_max = 32768;
		this->value_none = 65535;
		this->value_default = this->value_none;
		return *this;
	}
	parameter &set_meta() {
		this->type = zzub_parameter_type_meta;
		this->name = "Meta";
		this->description = "Meta";
		this->value_min = 0;
		this->value_max = 0;
		this->value_none = 0;
		this->value_default = 0;
		return *this;
	}
	parameter &set_wavetable_index() {
		this->type = zzub_parameter_type_byte;
		this->name = "Wave";
		this->description = "Wave to use (01-C8)";
		this->value_min = zzub_wavetable_index_value_min;
		this->value_max = zzub_wavetable_index_value_max;
		this->value_none = zzub_wavetable_index_value_none;
		this->flags = zzub_parameter_flag_wavetable_index;
		this->value_default = 0;
		return *this;
	}
	parameter &set_name(const char *name) { this->name = name; return *this; }
	parameter &set_description(const char *description) { this->description = description; return *this; }
	parameter &set_value_min(int value_min) { this->value_min = value_min; return *this; }
	parameter &set_value_max(int value_max) { this->value_max = value_max; return *this; }
	parameter &set_value_none(int value_none) { this->value_none = value_none; return *this; }
	parameter &set_flags(int flags) { this->flags = flags; return *this; }
	parameter &set_state_flag() { this->flags |= zzub_parameter_flag_state; return *this; }
	parameter &set_wavetable_index_flag() { this->flags |= zzub_parameter_flag_wavetable_index; return *this; }
	parameter &set_velocity_index_flag() { this->flags |= zzub_parameter_flag_velocity_index; return *this; }
	parameter &set_delay_index_flag() { this->flags |= zzub_parameter_flag_delay_index; return *this; }
	parameter &set_event_on_edit_flag() { this->flags |= zzub_parameter_flag_event_on_edit; return *this; }
	parameter &set_value_default(int value_default) { this->value_default = value_default; return *this; }
	parameter &set_meta_note() { this->flags |= zzub_parameter_flag_meta_note; return *this; }
	parameter &set_meta_wave() { this->flags |= zzub_parameter_flag_meta_wave; return *this; }
	
	float normalize(int value) const {
		assert(value != this->value_none);
		return float(value - this->value_min) / float(this->value_max - this->value_min);
	}
	
	int scale(float normal) const {
		return int(normal * float(this->value_max - this->value_min) + 0.5) + this->value_min;
	}
	
	int get_bytesize() const {
		switch(this->type) {
			case zzub_parameter_type_note:
			case zzub_parameter_type_switch:
			case zzub_parameter_type_byte:
				return 1;
			case zzub_parameter_type_word:
				return 2;
			default:
				return 0;
		}
		return 0;
	}
	
	parameter &append(std::vector<const parameter *>& paramlist) {
		paramlist.push_back(this);
		return *this;
	}
};

struct attribute {
	std::string name;
	int value_min;
	int value_max;
	int value_default;
	
	attribute() {
		name = "";
		value_min = 0;
		value_max = 0;
		value_default = 0;
	}
	
	attribute &set_name(const char *name) { this->name = name; return *this; }
	attribute &set_value_min(int value_min) { this->value_min = value_min; return *this; }
	attribute &set_value_max(int value_max) { this->value_max = value_max; return *this; }
	attribute &set_value_default(int value_default) { this->value_default = value_default; return *this; }

};

struct envelope_info {
	const char* name;
	int flags;
};

struct master_info {
	int beats_per_minute;
	int ticks_per_beat;
	int samples_per_second;
	int samples_per_tick;
	int tick_position;
	float ticks_per_second;
	float samples_per_tick_frac;    // zzub extension
	int row_position;				// zzub extension
	float tick_position_frac;
	float swing_amount;
	int swing_ticks;
};


struct envelope_point {
	unsigned short x, y;
	unsigned char flags;		// flags: bit 0 = sustain
};

struct envelope_entry {
	unsigned short attack, decay, sustain, release;
	char subDivide, flags;	// ADSR Subdivide & Flags
	bool disabled;
	std::vector<envelope_point> points;
};

struct wave_info {
	int flags;
	float volume;
	int id;
	int wavelevel_count;
};

// the order of the members in wave_level are matched with buzz' CWaveInfo
// members prefixed with legacy_ should be handled in buzz2zzub and removed from here
struct wave_level {
	int legacy_sample_count;
	short *legacy_sample_ptr;
	int root_note;
	int samples_per_second;
	int legacy_loop_start;
	int legacy_loop_end;
	int sample_count;
	short* samples;
	int loop_start;
	int loop_end;
	zzub_wave_buffer_type format;	// zzub_wave_buffer_type
	std::vector<int> slices; // slice offsets in the wave
	int id;
	int wave_id;

	int get_bytes_per_sample() const {
		switch (format) {
			case zzub_wave_buffer_type_si16:
				return 2;
			case zzub_wave_buffer_type_si24:
				return 3;
			case zzub_wave_buffer_type_si32:
			case zzub_wave_buffer_type_f32:
				return 4;
			default:
				assert(false);
				return 0;
		}
	}
};

struct outstream;
struct plugin;
struct plugincollection;

struct host_info {
	int id;
	int version;
	void* host_ptr;	// host-specific data
};

struct lib {
	virtual void get_instrument_list(outstream *os) = 0;
};
		
struct instream	{
	virtual ~instream() {};
	virtual int read(void *buffer, int size) = 0;

	virtual long position() = 0;
	virtual void seek(long, int) = 0;
	
	virtual long size() = 0;

	template <typename T>
	int read(T &d) { return read(&d, sizeof(T)); }

	int read(std::string& d) {
		char c = -1;
		d = "";
		int i = 0;
		do {
			if (!read<char>(c)) break;
			if (c) d += c;
			i++;
		} while (c != 0);
		return i;
	}
};

struct outstream {
	virtual ~outstream() {};
	virtual int write(void *buffer, int size) = 0;
	
	template <typename T>
	int write(T d) { return write(&d, sizeof(T)); }

	#if defined(_STRING_H_) || defined(_INC_STRING)
	// include string.h or cstring before zzubplugin.h to get this function
	int write(const char *str) { return write((void*)str, (int)strlen(str) + 1); }
	#elif defined(_GLIBCXX_CSTRING)
	int write(const char *str) { return write((void*)str, (int)std::strlen(str) + 1); }
	#endif

	virtual long position() = 0;
	virtual void seek(long, int) = 0;
};

struct archive {
	virtual ~archive() {};
	virtual outstream *get_outstream(const char *path) = 0;
	virtual instream *get_instream(const char *path) = 0;
};

struct info	{
	int version;
	int flags;
	unsigned int min_tracks;
	unsigned int max_tracks;
	std::string name;
	std::string short_name;
	std::string author;
	std::string commands;
	lib* plugin_lib;
	std::string uri;
	int outputs; // when flags & mono_to_stereo -> outputs = 2, actual 1 or 2
	int inputs;  // when flags & mono_to_stereo -> inputs = 2, actual 1 or 2, or 0 for gens
	
	std::vector<const zzub::parameter*> global_parameters;
	std::vector<const zzub::parameter*> track_parameters;

	// for controller plugins: those will be associated with parameters of remote plugins
	// they are purely internal and will not be visible in the pattern editor or gui
	std::vector<const zzub::parameter*> controller_parameters; 

	std::vector<const zzub::attribute*> attributes;

	// internal parameters for mute, bypass, meta. set by the engine on all plugins
	std::vector<const zzub::parameter*>* internal_parameters;

	// virtual parameters for pianoroll, notematrix, wave editor. set by the engine on suitable plugins
	std::vector<const zzub::parameter*> virtual_parameters;

	// details about what formats import and stream plugins handle
	std::vector<std::string> supported_import_extensions;
	std::vector<std::string> supported_stream_extensions;

	// plugins can set these themselves, or if left to -1, the engine makes a good guess based on the parameters:
	int note_group, note_column, velocity_column, wave_column;
	
	// full path to the dll implementing this plugin. blank for built-in plugins.
	std::string pluginfile;

	// the plugin factory used to create the plugininfo. set by the plugin.
	zzub::plugincollection* collection;
	
	virtual zzub::plugin* create_plugin() = 0;
	virtual bool store_info(zzub::archive *arc) const { return false; }

	zzub::parameter& add_global_parameter() {
		zzub::parameter *param = new zzub::parameter();
		global_parameters.push_back(param);
		return *param;
	}

	zzub::parameter& add_track_parameter() {
		zzub::parameter *param = new zzub::parameter();
		track_parameters.push_back(param);
		return *param;
	}

	zzub::parameter& add_controller_parameter() {
		zzub::parameter *param = new zzub::parameter();
		controller_parameters.push_back(param);
		return *param;
	}

	zzub::attribute& add_attribute() {
		zzub::attribute *attrib = new zzub::attribute();
		attributes.push_back(attrib);
		return *attrib;
	}
	
	info() {
		version = zzub_version;
		flags = 0;
		min_tracks = 0;
		max_tracks = 0;
		name = "";
		short_name = "";
		author = "";
		commands = "";
		plugin_lib = 0;
		uri = "";
		note_group = -1;
		velocity_column = -1;
		wave_column = -1;
		internal_parameters = 0;
		collection = 0;
	}
	
	virtual ~info() {
		if (internal_parameters)
			internal_parameters = 0; // owned by the pluginmgr

		for (std::vector<const zzub::parameter*>::iterator i = global_parameters.begin();
			i != global_parameters.end(); ++i) {
			delete *i;
		}
		global_parameters.clear();
		for (std::vector<const zzub::parameter*>::iterator i = track_parameters.begin();
			i != track_parameters.end(); ++i) {
			delete *i;
		}
		track_parameters.clear();
		for (std::vector<const zzub::parameter*>::iterator i = controller_parameters.begin();
			i != controller_parameters.end(); ++i) {
			delete *i;
		}
		controller_parameters.clear();
		for (std::vector<const zzub::parameter*>::iterator i = virtual_parameters.begin();
			i != virtual_parameters.end(); ++i) {
			delete *i;
		}
		virtual_parameters.clear();
		for (std::vector<const zzub::attribute*>::iterator i = attributes.begin();
			i != attributes.end(); ++i) {
			delete *i;
		}
		attributes.clear();
	}

    static int calc_column_size(const std::vector<const zzub::parameter*> &params) {
        int size = 0;
        for (unsigned i = 0; i<params.size(); i++) {
            size += params[i]->get_bytesize();
        }
        return size;
    }

    int get_group_size(int group) const {
        switch (group) {
            case 0:
                return 2*sizeof(short);
            case 1:
                return calc_column_size(global_parameters);
            case 2:
                return calc_column_size(track_parameters);
            default:
                return 0;
        }
    }

};

struct midi_message {
	int device;
	unsigned long message;
	unsigned long timestamp;
};

struct plugin {
	virtual ~plugin() { }
	virtual void destroy() { delete this; }
	virtual void init(zzub::archive *arc) {}
	virtual void process_events() {}
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) { return false; }
	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int loopstart, int loopend) { return false; }
	virtual void stop() {}
	virtual void load(zzub::archive *arc) {}
	virtual void save(zzub::archive *arc) {}
	virtual void attributes_changed() {}
	virtual void command(int index) {}
	virtual void set_track_count(int count) {}
	virtual void mute_track(int index) {}
	virtual bool is_track_muted(int index) const { return false; }
	virtual void event(unsigned int data)  {}
	virtual const char * describe_value(int param, int value) { return 0; }
	virtual const zzub::envelope_info ** get_envelope_infos() { return 0; }
	virtual bool play_wave(int wave, int note, float volume, int offset, int length) { return false; }
	virtual void stop_wave() {}
	virtual int get_wave_envelope_play_position(int env) { return -1; }
	virtual void update_timesource() {}
	virtual int get_latency() { return 0; }
	virtual int get_output_channel_count() { return -1; }
	virtual int get_input_channel_count() { return -1; }
	virtual const char* get_output_channel_name(int i) { return 0; }
	virtual const char* get_input_channel_name(int i) { return 0; }

	// these have been in zzub::plugin2 before
	virtual const char* describe_param(int param) { return 0; }
	virtual bool set_instrument(const char *name) { return false; }
	virtual void get_sub_menu(int index, zzub::outstream *os) {}
	virtual void midi_event(unsigned short status, unsigned char data1, unsigned char data2) { }

	// zzub_plugin_flag_has_event_input | zzub_plugin_flag_has_event_output
	virtual void process_controller_events() {}

	// zzub_plugin_flag_does_input_mixing
	virtual void add_input(int connection_id) {}
	virtual void delete_input(int connection_id) {}
	virtual void rename_input(const char *oldname, const char *newname) {}
	virtual void input(int connection_id, int first_input, int first_output, int inputs, int outputs, int flags, float **samples, int size, float amp) {}
	virtual bool handle_input(int index, int amp, int pan) { return false; }
	virtual void set_input_channels(int from_plugin_id, int first_input, int first_output, int inputs, int outputs, int flags) { }

	// plugin_flag_has_midi_input
	virtual void process_midi_events(midi_message* pin, int nummessages) {}

	// plugin_flag_has_midi_output
	virtual void get_midi_output_names(outstream *pout) {}

	// plugin_flag_stream | plugin_flag_has_audio_output
	virtual void set_stream_source(const char* resource) {}
	virtual void enable_stream_source() {}
	virtual void flush_stream_source() {}
	
	// plugin_flag_has_interval
	virtual void process_interval() {}
	virtual int get_interval_size() { return 0; }

	// plugin_flag_is_sequence
	virtual int get_currently_playing_row(int patternid) { return -1; }
	virtual int get_currently_playing_patterns(int** patterns) { return 0; }
	virtual void get_time_info(master_info** timeinfo) { *timeinfo = 0; }

	// plugin_flag_is_connection
	virtual void set_connection(int connid) { }
	virtual void process_connection_events() { }

	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {}

	// plugin_flag_is_encoder
	virtual bool process_encoder(int state, float** buffers, int numsamples) { return false; } // called from encoder thread! return false = engine frees memory, true = plugin frees memory
	virtual int get_encoder_digest(int type, float** buffers, int numsamples) { return 0; } // called from user thread!

	// Called by the host to embed plugin specific gui in the parameter view.
	virtual bool has_embedded_gui() { return false; }
	virtual bool create_embedded_gui(void* hwnd) { return false; }
	virtual void resize_embedded_gui(void* hwnd, int* outwidth, int* outheight) {}

	plugin() {
		global_values = 0;
		track_values = 0;
		controller_values = 0;
		attributes = 0;
		_master_info = 0;
		_player = 0;
		_plugin = 0;
	}

	void *global_values;
	void *track_values;
	void *controller_values;
	int *attributes;

	master_info *_master_info;
	zzub_player_t* _player;
	zzub_mixer_t* _mixer;
	zzub_plugin_t* _plugin;
	int _id;
};

// A plugin factory allows to add and replace plugin infos
// known to the host.
struct pluginfactory {
	
	// Registers a plugin info to the host. If the uri argument
	// of the info struct designates a plugin already existing
	// to the host, the old info struct will be replaced.
	virtual void register_info(zzub::info *_info) = 0;
};

// A plugin collection registers plugin infos and provides
// serialization services for plugin info, to allow
// loading of plugins from song data.
struct plugincollection {
	
	virtual ~plugincollection() { }
	// Called by the host initially. The collection registers
	// plugins through the pluginfactory::register_info method.
	// The factory pointer remains valid and can be stored
	// for later reference.
	virtual void initialize(zzub::pluginfactory *factory) {}
	
	// Called by the host upon song loading. If the collection
	// can not provide a plugin info based on the uri or
	// the metainfo passed, it should return a null pointer.
	// This will usually only be called if the host does
	// not know about the uri already.
	virtual zzub::info *get_info(const char *uri, zzub::archive *arc) { return 0; }
	
	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return 0; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {}
	
	// Called by the host upon destruction. You should
	// delete the instance in this function
	virtual void destroy() { delete this; }

	// Returns a user-readable string identifying the plugin collection.
	virtual const char* get_name() { return 0; }
};

#define SIGNAL_TRESHOLD (0.0000158489f)

inline bool buffer_has_signals(const float *buffer, int ns) {
	while (ns--) {
		if ((*buffer > SIGNAL_TRESHOLD)||(*buffer < -SIGNAL_TRESHOLD)) {
			return true;
		}
		buffer++;
	}
	return false;
}

} // namespace zzub

// these are for external native plugins (dlls). unsupported and unused.
typedef zzub::plugincollection *(*zzub_get_plugincollection_function)();
typedef const char *(*zzub_get_signature_function)();

#endif  // __ZZUBPLUGIN_H
