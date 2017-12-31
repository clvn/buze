// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007 Frank Potulski <polac@gmx.de>
// copyright 2007 members of the psycle project http://psycle.sourceforge.net

#define NOMINMAX
#include <cstdlib>
#include <cstdio>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>

#if defined _WIN64 || defined _WIN32
	#include <windows.h>
#else
	#include <dlfcn.h>
	#include <sys/stat.h>
	#include <dirent.h>
#endif
#include "../../mixing/convertsample.h"
#include <zzub/plugin.h>
#include "psycle/plugin_interface.hpp" ///\todo should be <psycle/plugin_interface.hpp>
#include "psy2zzub.h"

namespace zzub { namespace plugins { namespace psycle_to_zzub {

	/*********************************************************************************************************************/
	/// module handling
	namespace module {
		typedef 
			#if defined _WIN64 || defined _WIN32
				HMODULE
			#else
				void *
			#endif
			handle_type;

		inline handle_type open(const char * filename) {
			return
				#if defined _WIN64 || defined _WIN32
					::LoadLibrary(filename);
				#else
					::dlopen(filename, RTLD_NOW);
				#endif
		}

		typedef void (*function_pointer)();
		inline function_pointer sym(handle_type handle, const char * symbol) {
			union result_union
			{
				function_pointer typed;
				#if defined _WIN64 || defined _WIN32
					::PROC
				#else
					void *
				#endif
					untyped;
			} result;
			result.untyped =
				#if defined _WIN64 || defined _WIN32
					::GetProcAddress
				#else
					::dlsym
				#endif
				(handle, symbol);
			return result.typed;
		}

		inline int close(handle_type handle) {
			return
				#if defined _WIN64 || defined _WIN32
					::FreeLibrary(handle) != 0;
				#else
					::dlclose(handle);
				#endif
		}
	}

	/*********************************************************************************************************************/
	///\name module initialisation / deinitialisation
	///\{
		#if defined _WIN64 || defined _WIN32
			namespace {
				static HINSTANCE hInst(0);
			}
			
			/*BOOL APIENTRY DllMain(HINSTANCE hm,DWORD dw,LPVOID lp) {
				switch(dw) {
					case DLL_PROCESS_ATTACH:
						hInst = hm; // hInst is never used, so why is this done?
						break;
					case DLL_PROCESS_DETACH:
						break;
					default: ;
				}
				return TRUE;
			}*/
		#elif defined __GNUG__
			namespace {
				__attribute__((constructor))
				void constructor() {
				}
				
				__attribute__((destructor))
				void destructor() {
				}
			}
		#else
			///\todo
		#endif
	///\}

	/*********************************************************************************************************************/
	///\name misc
	///\{
		float normalize(const int val, const int min, const int max) {
			return float(val - min) / (max - min);
		}

		int scale(float normal, const int min, const int max)
		{
			return static_cast<int>(normal * (max - min) + 0.5f) + min;
		}
		
		inline bool is_denormal(const float & f) {
			return (*(reinterpret_cast<const unsigned int *>(&f)) & 0x7f800000) == 0;
		}
		
		void replace_char(char *str,const char toReplace,const char replacedBy) {
			if(!str || toReplace == replacedBy) return;
			while(*str) {
				char &cmp = *str;
				if (cmp==toReplace) cmp = replacedBy;
				++str;
			}
		}

		const int version = 1;
		///\todo huh?
		const int min_tracks = 8;
		const int max_tracks = psycle::plugin_interface::MAX_TRACKS;
	///\}

	/*********************************************************************************************************************/
	// plugin_collection
	
	void plugin_collection::initialize(zzub::pluginfactory * factory) {
		this->factory = factory;
		if(!this->factory) return;
		#if defined _WIN64 || defined _WIN32
			module::handle_type module_handle(::GetModuleHandle(0));
			if(!module_handle) return;
			char path[MAX_PATH + 32] = {0};
			::GetModuleFileName(module_handle, path, MAX_PATH);
			std::size_t n(std::strlen(path));
			if(!n) return;
			while(n--) {
				if (path[n]=='\\') {
					path[n] = 0;
					break;
				}
			}
			std::string s(path);
			s += "\\Gear\\Psycle\\";
			scan_plugins(s);
		#else
			///\todo use binreloc
			char const * env(std::getenv("PSYCLE_PATH"));
			if(!env) std::cerr << "Warning: You do not have a PSYCLE_PATH environment variable set." << std::endl;
			else {
				std::string s(env);
				scan_plugins(s);
			}
		#endif
	}
			
	void plugin_collection::destroy() {
		delete this;
	}

	plugin_collection::~plugin_collection() {
		if(plugin_infos.size()) {
			for(std::list<plugin_info*>::iterator i(plugin_infos.begin()); i != plugin_infos.end(); ++i) delete *i;
			plugin_infos.clear();
		}
	}
	
	void plugin_collection::scan_plugins(std::string const & path) {
		std::cout << "enumerating psycle plugins in: " << path << "\n";
		std::string search_path(path);
		#if defined _WIN64 || defined _WIN32
			search_path += "*.dll";
			WIN32_FIND_DATA fd;
			HANDLE hFind=::FindFirstFile(search_path.c_str(), &fd);
			while(hFind != INVALID_HANDLE_VALUE) {
				std::string full_path(path + '\\' + fd.cFileName);
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) scan_plugins(full_path.c_str());
				else {
					char absolute_path[MAX_PATH];
					::GetFullPathName(full_path.c_str(), MAX_PATH, absolute_path, 0);
					add_plugin(absolute_path);
				}
				if (!::FindNextFile(hFind, &fd)) break;
			}
			::FindClose(hFind);
		#else
			struct dirent ** namelist;
			struct stat statinfo;
			/// beware, scandir is not in posix yet as of 2007
			int n(scandir(search_path.c_str(), &namelist, 0, alphasort));
			if(n < 0) {
				perror("scandir");
				return;
			}
			while(n--) {
				std::string name(namelist[n]->d_name);
				std::free(namelist[n]);
				if(name == "." || name == "..") continue;
				std::string full_path(path + '/' + name);
				if(!stat(full_path.c_str(), &statinfo))
					if(S_ISDIR(statinfo.st_mode)) scan_plugins(full_path);
					else add_plugin(full_path);
			}
			std::free(namelist);
		#endif
	}

	void plugin_collection::add_plugin(std::string const & path) {
		std::cout << "checking for psycle plugin: " << path << "\n";
		if(!factory) return;
		module::handle_type module_handle(module::open(path.c_str()));
		if(!module_handle) {
			std::cerr << "could not load module: " << path << std::endl;
			return;
		}
		using namespace psycle::plugin_interface::symbols;
		get_info_function get_info = (get_info_function) module::sym(module_handle, get_info_function_name);
		if(!get_info) std::cerr << "not a psycle plugin: " << path << std::endl;
		else {
			const CMachineInfo * const psycle_info = get_info();
			if(!psycle_info) std::cerr << "call to " << get_info_function_name << " failed" << std::endl;
			else {
				plugin_info * zzub_info(new plugin_info());
				zzub_info->collection = this;
				if(zzub_info) {
					switch(psycle_info->Flags) {
						case psycle::plugin_interface::GENERATOR:
							zzub_info->flags = zzub_plugin_flag_has_audio_output | zzub_plugin_flag_has_midi_input;
							zzub_info->inputs = 0;
							zzub_info->outputs = 2;
							break;
						case psycle::plugin_interface::SEQUENCER:
							///\todo
							zzub_info->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_audio_output;
							zzub_info->outputs = 2;
							zzub_info->inputs = 2;
							break;
						case psycle::plugin_interface::EFFECT:
						default:
							zzub_info->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_audio_output;
							zzub_info->inputs = 2;
							zzub_info->outputs = 2;
					}

					zzub_info->version = zzub_version;

					zzub_info->psy_name = psycle_info->Name;
					zzub_info->name = zzub_info->psy_name.c_str();

					zzub_info->psy_label = psycle_info->ShortName;
					zzub_info->short_name = zzub_info->psy_label.c_str();

					zzub_info->psy_author = psycle_info->Author;
					zzub_info->author = zzub_info->psy_author.c_str();

					zzub_info->psy_command = psycle_info->Command;
					zzub_info->psy_command += "\nAbout ";
					zzub_info->psy_command += psycle_info->Name;
					zzub_info->psy_command += "...";
					zzub_info->commands = zzub_info->psy_command.c_str();

					zzub_info->psy_uri = "@psycle.sourceforge.net/";
					zzub_info->psy_uri += zzub_info->psy_name;
					///\todo not a good idea to modify the underlying buffer
					replace_char(const_cast<char*>(zzub_info->psy_uri.c_str()), ' ','+');
					replace_char(const_cast<char*>(zzub_info->psy_uri.c_str()),'\t','+');
					replace_char(const_cast<char*>(zzub_info->psy_uri.c_str()),'\n','+');
					replace_char(const_cast<char*>(zzub_info->psy_uri.c_str()),'\r','+');
					zzub_info->uri = zzub_info->psy_uri.c_str();

					zzub_info->psy_path = path;

					if(psycle_info->numParameters > 0 && psycle_info->Parameters) {
						const int n = psycle_info->numParameters;
						zzub_info->psy_param.resize(n);
						for(int i(0) ; i < n; ++i) {
							zzub::parameter & zzub_param(zzub_info->add_global_parameter());
							const psycle::plugin_interface::CMachineParameter & psycle_param(*psycle_info->Parameters[i]);
							zzub_param.set_word();
							const int min_value = 0;
							///\todo what about 0xffff?
							const int max_value = 0xfffe;
							if(psycle_param.Flags == psycle::plugin_interface::MPF_STATE)
								zzub_param.set_flags(zzub_parameter_flag_state);
							if(psycle_param.MinValue >= min_value && psycle_param.MaxValue <= max_value) {
								zzub_param.set_value_min(psycle_param.MinValue);
								zzub_param.set_value_max(psycle_param.MaxValue);
								zzub_param.set_value_default(psycle_param.DefValue);
							} else if(psycle_param.MaxValue - psycle_param.MinValue <= max_value ) {
								zzub_param.set_value_min(0);
								zzub_param.set_value_max(psycle_param.MaxValue - psycle_param.MinValue);
								zzub_param.set_value_default(psycle_param.DefValue - psycle_param.MinValue);
							} else {
								zzub_param.set_value_min(min_value);
								zzub_param.set_value_max(max_value);
								zzub_param.set_value_default(zzub_param.scale(normalize(
									psycle_param.DefValue, psycle_param.MinValue, psycle_param.MaxValue)));
							}
							if(zzub_param.value_max <= 0x01) {
								zzub_param.type = zzub_parameter_type_switch;
								zzub_param.value_none = 0xff;
							} else if (zzub_param.value_max<0xff) {
								zzub_param.type = zzub_parameter_type_byte;
								zzub_param.value_none = 0xff;
							}
							std::strncpy(zzub_info->psy_param[i].name, psycle_param.Name, 32);
							zzub_info->psy_param[i].name[31] = 0;
							zzub_param.set_name(zzub_info->psy_param[i].name);

							std::strncpy(zzub_info->psy_param[i].desc, psycle_param.Description, 32);
							zzub_info->psy_param[i].desc[31] = 0;
							zzub_param.set_description(zzub_info->psy_param[i].desc);
						}
					}
					switch(psycle_info->Flags) {
						case psycle::plugin_interface::GENERATOR:
							{
								zzub_info->min_tracks = min_tracks;
								zzub_info->max_tracks = max_tracks;
								zzub::parameter &paramNote = zzub_info->add_track_parameter();
								paramNote.set_note();
								paramNote.set_flags(zzub_parameter_flag_event_on_edit);
								zzub::parameter &paramCommand = zzub_info->add_track_parameter();
								paramCommand.set_word();
								paramCommand.set_value_min(0x0001);
								paramCommand.set_value_max(0xFFFF);
								paramCommand.set_value_none(0x0000);
								paramCommand.set_name("Note Command");
								paramCommand.set_description("Note Command");
							}
							break;
						case psycle::plugin_interface::SEQUENCER:
							///\todo
						case psycle::plugin_interface::EFFECT:
						default:
							zzub_info->min_tracks = 0;
							zzub_info->max_tracks = 0;
					}
					plugin_infos.push_back(zzub_info);
					factory->register_info(zzub_info);
					std::cout << "registered psycle plugin: " << zzub_info->name << "\n";
				}
			}
		}
		module::close(module_handle);
	}

	/*********************************************************************************************************************/
	// plugin

	plugin::plugin(const plugin_info * info)
	:
		info(info),
		global_params(info),
		track_params(new track_param[info->track_parameters.size() * max_tracks]),
		module_handle(),
		psycle_plugin(),
		psycle_plugin_param_info(),
		track_count()
	{
		global_values = global_params.data;
		track_values = track_params;
		attributes = 0;
	}

	bool plugin::open() {
		if(!info) return false;
		close();
		module_handle = module::open((info->psy_path.c_str())); if(!module_handle) return false;
		using namespace psycle::plugin_interface::symbols;
		get_info_function get_info = (get_info_function) module::sym(module_handle, get_info_function_name);
		if(!get_info) {
			std::cerr << "not a psycle plugin: " << info->name << std::endl;
			module::close(module_handle);
			module_handle = 0;
			return false;
		}
		const psycle::plugin_interface::CMachineInfo * const psycle_info = get_info();
		if(!psycle_info) {
			std::cerr << "call to " << get_info_function_name << " failed" << std::endl;
			module::close(module_handle);
			module_handle = 0;
			return false;
		}
		psycle_plugin_param_info = psycle_info->Parameters;
		create_machine_function create_machine = (create_machine_function) module::sym(module_handle, create_machine_function_name);
		if(!create_machine)
		{
			std::cerr << "not a psycle plugin: " << info->name << std::endl;
			module::close(module_handle);
			module_handle = 0;
			return false;
		}
		psycle_plugin = create_machine();
		if(!psycle_plugin) {
			std::cerr << "call to " << create_machine_function_name << " failed" << std::endl;
			module::close(module_handle);
			module_handle = 0;
			return false;
		}
		psycle_plugin->pCB = this;
		psycle_plugin->Init();
		return true;
	}

	bool plugin::close() {
		if(psycle_plugin) {
			try {
				using namespace psycle::plugin_interface::symbols;
				delete_machine_function delete_machine = (delete_machine_function) module::sym(module_handle, delete_machine_function_name);
				if(delete_machine) delete_machine(*psycle_plugin);
				else delete psycle_plugin; // some early closed-source plugins might not have a DeleteMachine function
			} catch(...) {}
			psycle_plugin = 0;
		}
		psycle_plugin_param_info = 0;
		if(module_handle) {
			try {
				module::close(module_handle);
			} catch(...) {}
			module_handle = 0;
		}
		return true;
	}

	void plugin::destroy() {
		delete this;
	}

	plugin::~plugin() throw() {
		close();
		delete[] track_params;
	}

	/*********************************************************************************************************************/
	// plugin ... implementation for psycle::plugin_interface::CFxCallback

	void plugin::MessBox(char *ptxt,char *caption,unsigned int type) {
		#if defined _WIN64 || defined _WIN32
			::MessageBox(::GetForegroundWindow(),ptxt,caption,type);
		#else
			///\todo
		#endif
	}

	/*********************************************************************************************************************/
	// plugin ... implementation for zzub::plugin

	void plugin::init(zzub::archive * arc) {
		if(!open()) return;
		if(!arc) return;
		zzub::instream * is = arc->get_instream("");
		if(!is) return;
		int read_version;
		is->read<int>(read_version);
		if(read_version != version) return; // version missmatch
		int data_size;
		is->read<int>(data_size);
		if(!data_size) return;
		unsigned char * const data(new unsigned char[data_size]);
		is->read(data, data_size);
		psycle_plugin->PutData(data);
		delete[] data;
	}

	void plugin::save(zzub::archive * arc) {
		if(!arc) return;
		if(!psycle_plugin) return;
		zzub::outstream * os = arc->get_outstream("");
		if(!os) return;
		const int data_size = psycle_plugin->GetDataSize();
		if(!data_size) return;
		os->write<int>(version);
		os->write<int>(data_size);
		unsigned char * const data(new unsigned char[data_size]);
		psycle_plugin->GetData(data);
		os->write(data, data_size);
		delete[] data;
	}

	void plugin::process_events() {
		if(!psycle_plugin) return;
		const int n = int(info->global_parameters.size());
		for (int i(0); i < n ; ++i) {
			const zzub::parameter & zzub_param(*info->global_parameters[i]);
			const psycle::plugin_interface::CMachineParameter & psycle_param(*psycle_plugin_param_info[i]);
			const int val(global_params[i]);
			if(val != zzub_param.value_none)
				psycle_plugin->ParameterTweak(i, scale(
					zzub_param.normalize(val),
					psycle_param.MinValue, psycle_param.MaxValue));
		}
		for(int i(0); i < track_count; ++i) {
			const track_param & param = track_params[i];
			if(param.note!=zzub_note_value_none) {
				if(param.note!=zzub_note_value_off && param.note!=zzub_note_value_cut) {
					const int note = ((param.note >> 4) * 12) + (param.note & 15);
					psycle_plugin->SeqTick(i, note, 0, param.command >> 8, param.command & 0xff);
				}
				else psycle_plugin->SeqTick(i,psycle::plugin_interface::NOTE_NO, 0, 0, 0);
			}
		}
	}

	bool plugin::process_stereo(float ** pin, float ** pout, int numsamples, int mode) {
		float zerobuffer[zzub_buffer_size] = {0.0f};
		if (!psycle_plugin) return false;
		if (pout[0] || pout[1]) {
			if (info->flags & zzub_plugin_flag_has_audio_input) {
				if (pin[0] && pout[0])
					dspcopyamp(pout[0], pin[0], numsamples, 32768);
				if (pin[1] && pout[1])
					dspcopyamp(pout[1], pin[1], numsamples, 32768);
			}

			float * ldst = pout[0] ? pout[0] : zerobuffer;
			float * rdst = pout[1] ? pout[1] : zerobuffer;
			psycle_plugin->Work(ldst, rdst, numsamples, track_count);

			if (pout[0]) {
				dspamp(pout[0], numsamples, 1.0f / 32768.0f);
				if (!zzub::buffer_has_signals(pout[0], numsamples)) {
					memset(pout[0], 0, numsamples * sizeof(float));
					pout[0] = 0;
				}
			}
			if (pout[1]) {
				dspamp(pout[1], numsamples, 1.0f / 32768.0f);
				if (!zzub::buffer_has_signals(pout[1], numsamples)) {
					memset(pout[1], 0, numsamples * sizeof(float));
					pout[1] = 0;
				}
			}
		}
		return pout[0] != 0 || pout[1] != 0;
	}

	void plugin::command(int index) {
		if(!psycle_plugin) return;
		if(!index) psycle_plugin->Command();
		else {
			std::ostringstream s;
			s
				<< "copyright 2007 Frank Potulski <polac@gmx.de> and members of the psycle project http://psycle.sourceforge.net\n\n"
				<< "now wrapping...\n\n"
				<< "Name:\t\t" << info->name
				<< "\nLabel:\t\t" << info->short_name
				<< "\nAuthor:\t\t" << info->author
				<< "\n\nPath:\t\t" << info->psy_path
				<< "\n\nNumParams:\t" << info->global_parameters.size();
				
			#if defined _WIN64 || defined _WIN32
				::MessageBox(::GetForegroundWindow(), s.str().c_str(), "Polac Psycle Loader v0.02a", MB_OK);
			#else
				///\todo
			#endif
		}
	}

	const char * plugin::describe_value(int param, int value) {
		/// warning: not reentrant!
		one_param_description[0] = 0;
		if(!psycle_plugin) return one_param_description;
		if(param < int(info->global_parameters.size())) {
			const zzub::parameter & zzub_param = *info->global_parameters[param];
			const psycle::plugin_interface::CMachineParameter & psycle_param(*psycle_plugin_param_info[param]);
			value = scale(
				zzub_param.normalize(value),
				psycle_param.MinValue, psycle_param.MaxValue);
			if(!psycle_plugin->DescribeValue(one_param_description, param, value))
				std::sprintf(one_param_description, "%d", value);
		} else std::sprintf(one_param_description, "%.2X %.2X", value >> 8 , value & 0xff);
		return one_param_description;
	}

	void plugin::process_midi_events(zzub::midi_message* pin, int nummessages) {
		
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
					if(psycle_plugin) psycle_plugin->MidiNote(channel,data1,velocity);
					break;
			}
		}
	}

	void plugin::get_midi_output_names(zzub::outstream *pout) {
		std::string name = "Psycle";
		pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
	}
	
	/*********************************************************************************************************************/
	// plugin::global_params
	
	plugin::global_params_type::global_params_type(const plugin_info * info)
	: data(), index(), size()
	{
		if(!info) return;
		const int n(int(info->global_parameters.size()));
		if(!n) return;
		index = new int[n]; if(!index) return;
		this->size = new char[n]; if(!this->size) return;
		int size(0);
		for(int i(0); i < n; ++i) {
			const zzub::parameter &param = *info->global_parameters[i];
			switch(param.type) {
				case zzub_parameter_type_note:
				case zzub_parameter_type_switch:
				case zzub_parameter_type_byte:
					index[i] = size;
					this->size[i] = 1;
					++size;
					break;
				case zzub_parameter_type_word:
					index[i] = size;
					this->size[i] = 2;
					size += 2;
					break;
				default:
					delete[] index; index = 0;
					delete[] this->size; this->size = 0;
					return;
			}
		}
		if(size > 0) data = new unsigned char[size];
	}

	plugin::global_params_type::~global_params_type() {
		delete[] data;
		delete[] index;
		delete[] size;
	}

	/*********************************************************************************************************************/
	///\name explicit symbol exports
	///\{
		/*extern "C" {
			PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT
			const char *
			PSYCLE__PLUGIN__CALLING_CONVENTION // unspecified by zzub interface but safer
			zzub_get_signature() { return ZZUB_SIGNATURE; }

			PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT
			zzub::plugincollection *
			PSYCLE__PLUGIN__CALLING_CONVENTION // unspecified by zzub interface but safer
			zzub_get_plugincollection() { return new plugin_collection(); }
		}*/
	///\}
}}}

zzub::plugincollection* psy2zzub_get_plugincollection() {
	return new zzub::plugins::psycle_to_zzub::plugin_collection();
}
