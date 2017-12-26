// This source is free software ; you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation ; either version 2,
// or (at your option) any later version.
// copyright 2009 Megzlna

//****************************************************************************************************************

/*

Currently this is lacking one major feature.

Midi events need to be sent early by the following amount:
(asiobuffer size) + (midi latency constant)

'midi latency constant' is a small number like '20 samples' that the user sets manually.
This is done if you know your midi interface has 20 samples latency.

..add that number onto:

asiobuffer size, because the synth will be connected to your ASIO input.

There's some complex timing issues in Buze's midi core which must be figured out!

*/

//****************************************************************************************************************

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstring>
#include <algorithm>

#include <windows.h>

#define NO_ZZUB_PLAYER_TYPE
namespace armstrong {
	namespace frontend {
		struct player;
	}
}
typedef armstrong::frontend::player zzub_player_t;

#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
	struct metaplugin;
}
typedef zzub::mixer zzub_mixer_t;

#include <player/player.h>
#include <mixing/mixer.h>

#include <zzub/plugin.h>

#include <json/json.h>

//****************************************************************************************************************
// setup

namespace zzub {
namespace plugins {
namespace hardware_to_zzub {

struct hwplugin;
struct hwplugininfo;
struct hwplugincollection;

//****************************************************************************************************************
// hwplugin

struct hwplugin : zzub::plugin
{
	hwplugin(const hwplugininfo* info);
	
	virtual void destroy();
	virtual void process_events();
	virtual bool process_stereo(float** pin, float** pout, int numsamples, int mode);
	virtual void set_track_count(int count);
	virtual void command(int index);
	virtual const char* describe_value(int param, int value);
	//virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	//virtual void get_midi_output_names(zzub::outstream* pout);
	virtual void stop();
	
	const hwplugininfo* info;	
	char one_param_description[128];
	const int global_count;
	int track_count;
	
	void midi_out(int time, unsigned int data);
	int samples_per_tick;
	int samples_in_tick;
	
	struct gvals_t
	{
		unsigned char* data;
		std::vector<int> offsets;
		std::vector<int> sizes;
		
		gvals_t(const zzub::info* info)
		{
			int offset = 0;
			
			for (int i = 0; i < info->global_parameters.size(); ++i)
			{
				int size = info->global_parameters[i]->get_bytesize();
				offsets.push_back(offset);
				sizes.push_back(size);
				offset += size;
			}
		}

		void init(unsigned char* data) { this->data = data; }
		
		__forceinline unsigned short const& operator[](const int n) const
		{
			return (sizes[n] == 1) ?
				data[offsets[n]] // note, switch, byte
			:
				*reinterpret_cast<unsigned short*>(&data[offsets[n]]); // word
		}
	} gvals;
	
	#pragma pack(push, 1)
	struct tvals_t {
		unsigned char note;
		unsigned char velo;
		unsigned char delay;
		unsigned char cut;
		unsigned char fxcmd;
		unsigned short fxarg;
	}* tvals;
	#pragma pack(pop)
	
	struct track
	{
		tvals_t* values;
		hwplugin* plugin;
		const hwplugininfo* info;
		
		int note;
		int last_note;
		int velo;
		int delay;
		int cut;
		int fxcmd;
		int fxarg;
		
		track(hwplugin* plugin, tvals_t* values);
		void process_events();
		void process_stereo(int numsamples);
		void stop();
	};
	
	std::vector<track> tracks;
};

//****************************************************************************************************************
// hwplugininfo

struct hwplugininfo : zzub::info
{
	virtual zzub::plugin* create_plugin() {
		return new hwplugin(this);
	}
	
	struct param_info
	{
		param_info() : cc(-1), has_describe_map(false) {}
		
		int cc;
		std::string describe_suffix;
		bool has_describe_map;
		std::map<int, std::string> describe_map;
	};
	
	const zzub::parameter* trackparam_note;
	const zzub::parameter* trackparam_velo;
	const zzub::parameter* trackparam_delay;
	const zzub::parameter* trackparam_cut;
	const zzub::parameter* trackparam_fxcmd;
	const zzub::parameter* trackparam_fxarg;
	
	int midichan;
	std::vector<param_info> param_infos;
	std::vector<int> input_chans;
	std::vector<int> output_chans;
};

//****************************************************************************************************************
// hwplugincollection

struct hwplugincollection : zzub::plugincollection
{
	virtual void initialize(zzub::pluginfactory* factory);
	
	virtual void destroy() {
		delete this;
	}
	
	void scan_plugins(std::string const& path, zzub::pluginfactory* factory);
	bool add_plugin(std::string const& path, zzub::pluginfactory* factory);
	zzub::info* hwplugincollection::create_plugin_info(Json::Value& root);
	std::string read_file(std::string const& path);
};

//****************************************************************************************************************
// hwplugincollection

void hwplugincollection::initialize(zzub::pluginfactory* factory)
{
	if (!factory) return;
	
	HMODULE module_handle = ::GetModuleHandle(0);
	if (!module_handle) return;
	
	char path[MAX_PATH + 32] = {0};
	::GetModuleFileName(module_handle, path, MAX_PATH);
	std::size_t n = std::strlen(path);
	if (!n) return;
	while (n--) {
		if (path[n]=='\\') {
			path[n] = 0;
			break;
		}
	}
	std::string s = path;
	s += "\\Gear\\Hardware\\";
	::CreateDirectory(s.c_str(), NULL);
	
	scan_plugins(s, factory);
}

void hwplugincollection::scan_plugins(std::string const& path, zzub::pluginfactory* factory)
{
	std::cerr << "hw2zzub: scanning hardware descriptions in: " << path << "\n";
	std::string search_path = path;
	
	search_path += "*.json";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		std::string full_path(path + '\\' + fd.cFileName);
		
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			scan_plugins(full_path.c_str(), factory);
		} else {
			char absolute_path[MAX_PATH];
			::GetFullPathName(full_path.c_str(), MAX_PATH, absolute_path, 0);
			add_plugin(absolute_path, factory);
		}
		
		if (!::FindNextFile(hFind, &fd)) break;
	}
	
	::FindClose(hFind);
}

std::string hwplugincollection::read_file(std::string const& path)
{
	FILE* file = fopen(path.c_str(), "rb");
	
	if (!file)
		return "";
	
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	std::string text;
	char* buffer = new char[size + 1];
	buffer[size] = 0;
	
	if (fread(buffer, 1, size, file) == (unsigned long)size)
		text = buffer;
	
	fclose(file);
	delete[] buffer;
	
	return text;
}

bool hwplugincollection::add_plugin(std::string const& path, zzub::pluginfactory* factory)
{
	std::cerr << "hw2zzub: reading hardware description: " << path << "\n";
	
	std::string input = read_file(path);
	if (input.empty())
	{
		std::cerr << "hw2zzub: failed to read input or empty input: " << path << "\n";
		return false;
	}
	
	Json::Reader reader;
	Json::Value root;
	bool parsingSuccessful = reader.parse(input, root);
	
	if (!parsingSuccessful)
	{
		std::cerr << "hw2zzub: failed to parse description file: " << path << "\n";
		std::cerr << "hw2zzub: parsing error: " << reader.getFormatedErrorMessages() << "\n";
		
		std::ostringstream sss; sss << path << "\n\n" << reader.getFormatedErrorMessages() << "\n";
		::MessageBox(::GetForegroundWindow(), sss.str().c_str(), "hw2zzub parse error!", MB_OK);

		return false;
	}
	
	zzub::info* info = create_plugin_info(root);
	if (info) {
		factory->register_info(info);
	} else {
		std::cerr << "hw2zzub: could not create wrapper info" << std::endl;
		return false;
	}
	
	std::cerr << "hw2zzub: registered hardware description: " << info->name << "\n";
	return true;
}

zzub::info* hwplugincollection::create_plugin_info(Json::Value& root)
{
	hwplugininfo* info = new hwplugininfo();
	if (!info) return 0;

	info->collection = this;

	info->inputs = 0;
	Json::Value ins = root["inputs"];
	if (!ins.empty())
	{
		for (Json::Value::iterator it = ins.begin(); it != ins.end(); ++it) {
			info->input_chans.push_back((*it).asInt());
			info->inputs += 1;
		}
		
		info->flags |= zzub_plugin_flag_has_audio_input;
	}
	
	info->outputs = 0;
	Json::Value outs = root["outputs"];
	if (!outs.empty())
	{
		for (Json::Value::iterator it = outs.begin(); it != outs.end(); ++it) {
			info->output_chans.push_back((*it).asInt());
			info->outputs += 1;
		}
		
		info->flags |= zzub_plugin_flag_has_audio_output;
	}
	
	info->flags |= zzub_plugin_flag_has_midi_input;
	info->min_tracks = 0;
	info->max_tracks = root.get("maxtracks", 0).asUInt();
	info->name = root.get("name", "").asString();
	info->short_name = root.get("shortname", "").asString();
	info->author = root.get("company", "").asString();
	info->commands = ""; //?
	
	std::string fixed_uri = info->name;
	std::replace(fixed_uri.begin(), fixed_uri.end(), ' ', '+');
	std::replace(fixed_uri.begin(), fixed_uri.end(), '\t', '+');
	std::replace(fixed_uri.begin(), fixed_uri.end(), '\n', '+');
	std::replace(fixed_uri.begin(), fixed_uri.end(), '\r', '+');
	info->uri = "@hw2zzub/" + fixed_uri;
	
	info->midichan = root.get("midichan", -1).asInt();
	
	// Do the global params
	/// todo add: inertia, inertia stop, pitch bend wheel, modulation wheel, channel aftertouch
	Json::Value globals = root["globals"];
	for (Json::Value::iterator it = globals.begin(); it != globals.end(); ++it)
	{
		Json::Value& v = *it;
		zzub::parameter& zzub_param = info->add_global_parameter();
		
		std::string ptype = v.get("type", "").asString();
		if (ptype == "byte")
			zzub_param.set_byte();
		else if (ptype == "word")
			zzub_param.set_word();
		else if (ptype == "switch")
			zzub_param.set_switch();
		else
			continue;
		
		zzub_param.set_name((v.get("name", "").asString()).substr(0,31).c_str());
		zzub_param.set_value_min(v.get("min", 0).asInt());
		zzub_param.set_value_max(v.get("max", 0).asInt());
		zzub_param.set_description((v.get("desc", zzub_param.name).asString()).substr(0,31).c_str());
		zzub_param.set_state_flag();
		
		// our own record keeping
		info->param_infos.push_back(hwplugininfo::param_info());
		hwplugininfo::param_info& pi = info->param_infos.back();
		pi.cc = v.get("cc", -1).asInt();
		pi.describe_suffix = v.get("suff", "").asString();
		pi.has_describe_map = v.isMember("vals");
		if (pi.has_describe_map)
		{
			Json::Value descs = v["vals"];
			for (Json::Value::iterator dit = descs.begin(); dit != descs.end(); ++dit)
			{
				std::istringstream stream(dit.key().asString());
				int n;
				stream >> n;
				pi.describe_map[n] = (*dit).asString();
			}
		}
	}
	
	// Do the track params
	if (info->max_tracks > 0)
	{
		info->trackparam_note = &info->add_track_parameter()
			.set_note()
			.set_event_on_edit_flag()
		;
		
		info->trackparam_velo = &info->add_track_parameter()
			.set_byte()
			.set_value_max(127)
			.set_value_default(127)
			.set_name("Note Velocity")
			.set_description("Note Velocity (0-127)")
			.set_flags(zzub_parameter_flag_velocity_index) // This is a cool new ZZUB feature!
			.set_event_on_edit_flag()
		;
		
		info->trackparam_delay = &info->add_track_parameter()
			.set_byte()
			.set_value_max(254)
			.set_value_default(0)
			.set_name("Note Delay")
			.set_description("Note Delay (0-254)")
			.set_flags(zzub_parameter_flag_delay_index) // This is a cool new ZZUB feature!
		;
		
		info->trackparam_cut = &info->add_track_parameter()
			.set_byte()
			.set_value_max(254)
			.set_value_default(0)
			.set_name("Note Cut")
			.set_description("Note Cut (0-254)")
		;
		
		info->trackparam_fxcmd = &info->add_track_parameter()
			.set_byte()
			.set_name("FX Command")
			.set_description("FX Command")
		;
		
		info->trackparam_fxarg = &info->add_track_parameter()
			.set_word()
			.set_name("FX Arg")
			.set_description("FX Arg")
		;
	}
	
	return info;
}

//****************************************************************************************************************
// hwplugin

hwplugin::hwplugin(const hwplugininfo* info)
:
	info(info),
	gvals(info),
	global_count(info->global_parameters.size())
{
	global_values = new unsigned char[info->global_parameters.size()];
	track_values = new unsigned char[info->track_parameters.size() * info->max_tracks];
	
	gvals.init(static_cast<unsigned char*>(global_values));
	tvals = static_cast<tvals_t*>(track_values);
	
	for (int i = 0; i < info->max_tracks; ++i) {
		tracks.push_back(track(this, &tvals[i]));
	}
	
	std::cerr << "hw2zzub: created hardware device: " << info->name << std::endl;
}

void hwplugin::destroy()
{
	delete[] global_values;
	delete[] track_values;
	delete this;
}

//	message							status			data1		data2
enum {
	midi_status_note_off			= 0x80,		//	note #		velocity
	midi_status_note_on				= 0x90,		//	note #		velocity
	midi_status_poly_aftertouch		= 0xA0,		//	note #		pressure
	midi_status_control_change		= 0xB0,		//	cc #		data
	midi_status_program_change		= 0xC0,		//	prog #		-
	midi_status_channel_aftertouch	= 0xD0,		//	pressure	-
	midi_status_pitch_wheel			= 0xE0,		//	lsb			msb
};

#define MAKEMIDI(status, data1, data2) \
	((((data2) << 16) & 0xFF0000) | \
	(((data1) << 8) & 0xFF00) | \
	((status) & 0xFF))

#define MAKEMIDI_EX(status, channel, data1, data2) \
	((((data2) << 16) & 0xFF0000) | \
	(((data1) << 8) & 0xFF00) | \
	((status) & 0xFF) | \
	((channel) & 0x0F))

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

void hwplugin::midi_out(int time, unsigned int data)
{
	zzub::midi_message msg = { -1, data, time };
	_mixer->midi_out(_id, msg);
}

void hwplugin::process_events()
{
	samples_per_tick = _master_info->samples_per_tick;
	samples_in_tick = 0;
	
	// send global data
	for (int i = 0; i < global_count; ++i)
	{
		const zzub::parameter& zzub_param = *info->global_parameters[i];
		const int val = gvals[i];
		
		if (val != zzub_param.value_none) {
			int msg = MAKEMIDI_EX(midi_status_control_change, info->midichan, info->param_infos[i].cc, val);
			midi_out(0, msg); /// sending too early?
		}
	}
	
	// send track data
	for (int i = 0; i < track_count; ++i) {
		tracks[i].process_events();
	}
}

bool hwplugin::process_stereo(float** pin, float** pout, int numsamples, int mode)
{
	zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
	int writeoffset = mpl->audiodata->output_buffer_write;

	for (int i = 0; i < track_count; ++i) {
		tracks[i].process_stereo(numsamples);
	}
	samples_in_tick += numsamples;
	
	if (mode & zzub_process_mode_write)
	{
		int pnum;
		
		//input
		pnum = 0;
		for (int i = 0; i < info->input_chans.size(); ++i) {
			int inchannel = info->input_chans[i];
			if (pout[i] != 0 && _mixer->input_buffer[inchannel] != 0) {
				memcpy(pout[i], _mixer->input_buffer[inchannel], numsamples * sizeof(float));
			} else {
				pout[i] = 0;
			}
		}
		
		//output
		pnum = 0;
		for (int i = 0; i < info->output_chans.size(); ++i) {
			_mixer->write_output(info->output_chans[i], writeoffset, pin[i], numsamples, 1.0f);
		}
	}
	
	return false;
}

void hwplugin::stop()
{
	for (int i = 0; i < track_count; ++i)
	{
		tracks[i].stop();
	}
}

void hwplugin::set_track_count(int count)
{
	if (count < track_count)
	{
		tracks[track_count - 1].stop(); // is this reliable?
	}
	
	track_count = count;
}

void hwplugin::command(int index)
{
	if (index == 0)
	{
		std::ostringstream s;
		
		s	<< "hw2zzub v0.0\n\n"
			<< "Hardware device...\n\n"
			<< "Name:\t\t" << info->name
			<< "\nLabel:\t\t" << info->short_name
			<< "\nCompany:\t\t" << info->author
			<< "\n\nNumParams:\t" << info->global_parameters.size();
		
		::MessageBox(::GetForegroundWindow(), s.str().c_str(), "hw2zzub", MB_OK);
	}
}

const char* hwplugin::describe_value(int param, int value)
{
	// warning: not reentrant!
	one_param_description[0] = 0;
	
	if (param < info->global_parameters.size())
	{
		const hwplugininfo::param_info& pi = info->param_infos[param];
		
		if (pi.has_describe_map)
		{
			std::map<int, std::string>::const_iterator found = pi.describe_map.find(value);
			
			if (found != pi.describe_map.end())
			{
				std::sprintf(one_param_description, (*found).second.c_str());
				return one_param_description;
			}
		}
	}
	
	std::sprintf(one_param_description, "%d", value);
	return one_param_description;
}

// todo: remap the incoming CC's to the parameters so that knob changes on the physical synths update the buze params
/*
void hwplugin::process_midi_events(zzub::midi_message* pin, int nummessages)
{
	
	for (int i = 0; i < nummessages; i++) {
		unsigned short status = pin[i].message & 0xff;
		int channel = status&0xf;
		int command = (status & 0xf0) >> 4;
		unsigned char data1 = (pin[i].message >> 8) & 0xff;
		unsigned char data2 = (pin[i].message >> 16) & 0xff;
		int velocity;
		switch (command) {
			case 0x8:
			case 0x9:
				velocity = data2;
				if (command == 8) velocity = 0;
				plugin->MidiNote(channel,data1,velocity);
				break;
		}
	}
}
*/

// todo: perhaps useful on a controller keyboard
/*
void hwplugin::get_midi_output_names(zzub::outstream* pout) {
	std::string name = "hw2zzub";
	pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
}
*/

//****************************************************************************************************************
// hwplugin track

hwplugin::track::track(hwplugin* plugin, tvals_t* values)
:
	plugin(plugin),
	values(values),
	info(plugin->info),
	note(info->trackparam_note->value_none),
	delay(info->trackparam_delay->value_none),
	cut(info->trackparam_cut->value_none),
	fxcmd(info->trackparam_fxcmd->value_none),
	fxarg(info->trackparam_fxarg->value_none),
	last_note(info->trackparam_note->value_none)
{}

void hwplugin::track::process_events()
{
	if (values->note != zzub_note_value_none) {
		note = values->note;
		delay = 0;
		cut = -1; ///???
		velo = 0x7F;
	}
	
	if (values->velo != info->trackparam_velo->value_none) {
		velo = values->velo;
	}
	
	float unit = (float)plugin->samples_per_tick / 255;
	
	if (values->delay != info->trackparam_delay->value_none) {
		delay = (int)(unit * values->delay); // convert to samples in tick
	}
	
	if (values->cut != info->trackparam_cut->value_none) {
		cut = (int)(unit * values->cut); // convert to samples in tick
	}
	
	/// not yet implemented
	if (values->fxcmd != info->trackparam_fxcmd->value_none) {
		fxcmd = values->fxcmd;
		///retrig
		///arp up
		///arp down
		///ornament
		///poly aftertouch
	}
	
	/// not yet implemented
	if (values->fxarg != info->trackparam_fxarg->value_none) {
		fxarg = values->fxarg;
	}
}

// the logic surrounding 'cut' is probably totally broken
void hwplugin::track::process_stereo(int numsamples)
{
	if (((plugin->samples_in_tick <= delay) && ((plugin->samples_in_tick + numsamples) >= delay))
		|| ((cut != -1) && (plugin->samples_in_tick <= cut) && ((plugin->samples_in_tick + numsamples) >= cut)))
	{
		int ts = delay / 16;
		
		if ((note == zzub_note_value_none) && (cut == -1)) return;
		
		// turn last note off
		{
			int msg = MAKEMIDI_EX(midi_status_note_off, info->midichan, last_note, 0);
			plugin->midi_out(ts++, msg);
			last_note = zzub_note_value_none; ///???wtf
		}
		
		// play new note
		if ((note != zzub_note_value_off) && (note != zzub_note_value_cut) && (cut == -1))
		{
			last_note = buzz_to_midi_note(note);
			int msg = MAKEMIDI_EX(midi_status_note_on, info->midichan, last_note, velo);
			plugin->midi_out(ts++, msg);
		}
		
		note = zzub_note_value_none;
	}
}

void hwplugin::track::stop()
{
	if (last_note != zzub_note_value_none)
	{
		int msg = MAKEMIDI_EX(midi_status_note_off, info->midichan, last_note, 0);
		plugin->midi_out(0, msg);
		note = zzub_note_value_none;
		last_note = zzub_note_value_none; ///???
	}
}

//****************************************************************************************************************

} // END namespace hardware_to_zzub
} // END namespace plugins
} // END namespace zzub

//****************************************************************************************************************
// external interface

zzub::plugincollection* hw2zzub_get_plugincollection() {
	return new zzub::plugins::hardware_to_zzub::hwplugincollection();
}
