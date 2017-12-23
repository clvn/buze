// lunar plugin system for zzub
// Copyright (C) 2006 Leonard Ritter
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


#define LUNAR_STD_BUILD
#include "lunarcs.h"
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

#if defined(LUNARTARGET_LLVM)
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Type.h"
#include "llvm/Bytecode/Reader.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/System/Process.h"
#include "llvm/System/Signals.h"
#include "llvm/Constants.h"
using namespace llvm;
#endif

#include <sstream>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <list>
#include <algorithm>
#include <map>
#include <cmath>

#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#if defined(LUNARTARGET_GCC)

#if defined(POSIX)
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

// cross platform library loading

namespace lunar {
#if defined(_WIN32)
	typedef HMODULE xp_modulehandle;
#elif defined(POSIX)
	typedef void *xp_modulehandle;
#endif


xp_modulehandle xp_dlopen(const char* path)
{
#if defined(_WIN32)
	return LoadLibrary(path);
#elif defined(POSIX)
	return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

const char *xp_dlerror() {
#if defined(_WIN32)
	return "";
#elif defined(POSIX)
	return dlerror();
#endif
}

void* xp_dlsym(xp_modulehandle handle, const char* symbol)
{
#if defined(_WIN32)
	return (void*)GetProcAddress((HINSTANCE)handle, symbol);
#elif defined(POSIX)
	return dlsym(handle, symbol);
#endif
}
	
void xp_dlclose(xp_modulehandle handle)
{
#if defined(_WIN32)
	FreeLibrary((HINSTANCE)handle);
#elif defined(POSIX)
	dlclose(handle);
#endif
}

} // namespace lunar

#endif // LUNARTARGET_GCC


extern "C" {
#include "sha1.h"
}

#include "pugxml.h"

#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};

typedef struct zzub::mixer zzub_mixer_t;

#include "zzub/plugin.h"

#include "lunar.h"
#include "mixing/mixer.h"

#include "lunarutils.h"
#include "lunarcs.h"

typedef enum {} lunar_parameter_names_t;

#include "lunarstd.h"
#include "lunargui.h"

#if defined(_WIN32)
HMODULE g_hModule;
#endif

// TODO: share this struct with lunarstd.cpp
namespace zzub {
	struct host {
		void (*request_gui_redraw)(lunar_host_t *);
		void (*midi_out)(lunar_host_t *, lunar_midi_message_t* outm, int n);
		void (*note_out)(lunar_host_t *, lunar_note_message_t* outn, int n);
		bool (*is_channel_connected)(lunar_host_t *, int index);
	};
};

namespace lunar {

#if defined(LUNARTARGET_LLVM)
llvm::ExecutionEngine *EE;
#endif

float ipol_log(float v1, float v2, float x) {
	if (x <= 0.0f)
		return v1;
	if (x >= 1.0f)
		return v2;
	if (v1 == 0.0f)
		v1 = -8; // -48dB or so
	else
		v1 = log(v1);
	v2 = log(v2);
	return exp(v1*(1-x) + v2*x);
}

struct metaparameter {
	std::string id;
	zzub::parameter *param;
	bool isfloat;
	bool islog;
	bool ismidinote;
	float power;
	float offset;
	float scalar;
	float precision;
	std::map<float, std::string> valuenames;
	
	int translate_back(float value) const {
		if (isfloat) {
			if (islog) {
				// FIXME
				return param->scale((value-offset)/scalar);
			} else {
				return param->scale((value-offset)/scalar);
			}
		} else if (param->type == zzub_parameter_type_note) {
			if (value == 0)
				return zzub_note_value_off;
			int midinote = 0;
			if (ismidinote)
				midinote = (int)(value + 0.5f);
			else
				midinote = (int)((log(value / 440.0f) / log(2.0f))*12.0f+57.5f);
			midinote = std::min(std::max(midinote, 1), 1 + 8 * 12); // 8 octaves?
			return ((midinote/12)<<4)|((midinote%12)+1);
		} else {
			return int(value+0.5f);
		}
		return param->value_none;
	}
	
	float translate(int value) const {
		if (isfloat) {
			if (islog) {
				return ipol_log(offset,offset+scalar,pow(param->normalize(value),power));
			} else {
				return offset + param->normalize(value) * scalar;
			}
		} else if (param->type == zzub_parameter_type_note) {
			if (value == zzub_note_value_off || value == zzub_note_value_cut)
				return 0;
			int midinote = 12 * (value >> 4) + (value & 0xf) - 1;
			if (!ismidinote)
				return 440.0f * pow(2.0f, float(midinote-57) / 12.0f);
			else
				return float(midinote);
		} else {
			return float(value);
		}
	}
	
	int get_good_value_max() const {
		float p = precision;
		if (!p)
			p = 0.01f;
		return std::min(int(scalar/p + 0.5f),65534);
	}
	
	metaparameter() {
		power = 1.0;
		param = 0;
		ismidinote = false;
		isfloat = false;
		islog = false;
		offset = 0;
		scalar = 1;
		precision = 0.01f;
	}
};

#if !defined(S_ISDIR)
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

#if defined(_WIN32)
#define mkdir(file,mod) mkdir(file)
#endif

struct dspplugin : zzub::plugin {
	struct info : zzub::info {
		std::string basepath;
		std::list<std::string> stringmemory;
		std::map<std::string,std::string> files;
		
		std::vector<metaparameter> gparamids;
		std::vector<metaparameter> tparamids;
		std::vector<metaparameter> cparamids;
		std::vector<std::string> attribids;

		bool hasgui;
		lunar_gui_info_t guiinfo;

		// true if plugin/@type='generic', means process_audio is called instead of process_stereo
		bool is_generic;
		std::vector<std::string> audioinputnames;
		std::vector<std::string> audiooutputnames;
		std::vector<std::string> midiinputnames;

		// enum used for parsing input and output names
		enum iotype {
			iotype_input = 1,
			iotype_output = 2
		};

#if defined(LUNARTARGET_LLVM)
		std::list<llvm::Module *> modules;
#elif defined(LUNARTARGET_GCC)
		std::list<xp_modulehandle> modules;
#endif
		lunar_new_fx_t new_fx;
		
		~info() {
#if defined(LUNARTARGET_LLVM)
			std::list<llvm::Module *>::iterator m;
			for (m = modules.begin(); m != modules.end(); ++m) {
				call_global_dtors_ctors(*m, true);
			}
#elif defined(LUNARTARGET_GCC)
			std::list<xp_modulehandle>::iterator m;
			for (m = modules.begin(); m != modules.end(); ++m) {
				xp_dlclose(*m);
			}
#endif
		}

		// creates resident memory for string buffers
		// that is managed by a list class
		const char *new_string(const char *text) {
			stringmemory.push_back(text);
			return stringmemory.back().c_str();
		}
		
		bool setup_attribute_from_xml(zzub::attribute &attrib, pug::xml_node item) {
			if (item.has_attribute("id")) {
				attribids.push_back(item.attribute("id").value());
			} else {
				std::cerr << "lunar: attribute has no id." << std::endl;
				return false;
			}
			if (item.has_attribute("name")) {
				attrib.set_name(new_string(item.attribute("name")));
			}
			if (item.has_attribute("minvalue")) {
				attrib.set_value_min((long)item.attribute("minvalue"));
			}
			if (item.has_attribute("maxvalue")) {
				attrib.set_value_max((long)item.attribute("maxvalue"));
			}
			if (item.has_attribute("defvalue")) {
				attrib.set_value_default((long)item.attribute("defvalue"));
			}
			return true;
		}
	
		bool setup_parameter_from_xml(std::vector<metaparameter> &paramids, zzub::parameter &param, pug::xml_node item, pug::xml_node parameters) {
			metaparameter mp;
			mp.param = &param;
			if (item.has_attribute("id")) {
				mp.id = item.attribute("id").value();
			} else {
				std::cerr << "lunar: parameter has no id." << std::endl;
				return false;
			}
			
			std::string type = item.attribute("type").value();
			if (type == "note") {
				param.set_note();
			} else if (type == "midinote") {
				param.set_note();
				mp.ismidinote = true;
			} else if (type == "switch") {
				param.set_switch();
			} else if (type == "byte") {
				param.set_byte();
			} else if (type == "word") {
				param.set_word();
			} else if (type == "waveindex") {
				param.set_wavetable_index();
			} else if (type == "float") {
				param.set_word();
				mp.isfloat = true;
			} else {
				std::cerr << "lunar: unknown parameter type '" << type << "'" << std::endl;
				return false;
			}
			
			if (item.has_attribute("valuenames")) {
				std::string vnid = item.attribute("valuenames").value();
				pug::xml_node vng = parameters.first_element_by_name("valuenamegroup");
				if (!vng.empty()) {
					bool found = false;
					for (pug::xml_node::child_iterator vn = vng.children_begin(); vn != vng.children_end(); ++vn) {					
						if (vn->has_attribute("id") && (vn->attribute("id").value() == vnid)) {
							for (pug::xml_node::child_iterator v = vn->children_begin(); v != vn->children_end(); ++v) {
								if (v->has_name("alias")) {
									if (v->has_attribute("value")) {
										mp.valuenames.insert(std::pair<float,std::string>(
											double(v->attribute("value")),
											v->attribute("name").value()
										));
									}
								}
							}
							found = true;
							break;
						}
					}
					if (!found) {
						std::cerr << "lunar: can't find '" << vnid << "' valuenames" << std::endl;
					}
				}
				
			}
			if (item.has_attribute("name")) {
				param.set_name(new_string(item.attribute("name")));
			}
			if (item.has_attribute("description")) {
				param.set_description(new_string(item.attribute("description")));
			}
			if (!mp.isfloat) {
				if (item.has_attribute("minvalue")) {
					param.set_value_min((long)item.attribute("minvalue"));
				}
				if (item.has_attribute("maxvalue")) {
					param.set_value_max((long)item.attribute("maxvalue"));
				}
				if (item.has_attribute("novalue")) {
					param.set_value_none((long)item.attribute("novalue"));
				}
				if (item.has_attribute("defvalue")) {
					param.set_value_default((long)item.attribute("defvalue"));
				}
			} else {
				float minv = 0.0f;
				float maxv = 1.0f;
				float defv = 1.0f;
				float v1;
				if (item.has_attribute("logarithmic")) {
					mp.islog = bool(item.attribute("logarithmic"));
					if (item.has_attribute("power")) {
						mp.power = float(double(item.attribute("power")));
					}
				}
				if (item.has_attribute("precision")) {
					mp.precision = float(double(item.attribute("precision")));
				}
				if (item.has_attribute("minvalue")) {
					minv = float(double(item.attribute("minvalue")));
					defv = minv;
				}
				if (item.has_attribute("maxvalue")) {
					maxv = float(double(item.attribute("maxvalue")));
					defv = maxv;
				}
				if (item.has_attribute("defvalue")) {
					defv = float(double(item.attribute("defvalue")));
				}
				if (defv < minv) {
					std::cerr << "lunar: defvalue is smaller than minvalue. (" << param.name << ": " << defv << "/" << minv << ")" << std::endl;
					return false;
				}
				if (defv > maxv) {
					std::cerr << "lunar: defvalue is larger than maxvalue. (" << param.name << ": " << defv << "/" << maxv << ")" << std::endl;
					return false;
				}
				mp.scalar = maxv - minv;
				mp.offset = minv;
				param.set_value_max(mp.get_good_value_max());
				if (mp.islog) {
					// uncalculate
					if (minv == 0.0f)
						v1 = -8; // -48dB or so
					else
						v1 = std::log(minv);
					defv = std::min(std::max(std::pow((std::log(defv) - v1) / (std::log(maxv) - v1), 1.0f/mp.power),0.0f),1.0f);
					//defv = std::min(std::max(std::pow((std::log(defv) - std::log(minv)) / (std::log(maxv) - std::log(minv)), 1.0f/mp.power),0.0f),1.0f);
					param.set_value_default(param.scale(defv));
				} else {
					param.set_value_default(param.scale((defv - mp.offset) / mp.scalar));
				}
			}
			bool state = item.attribute("state");
			bool waveindex = item.attribute("waveindex");
			bool editevent = item.attribute("editevent");
			if (state) {
				param.set_state_flag();
			}
			if (waveindex) {
				param.set_wavetable_index_flag();
			}
			if (editevent) {
				param.set_event_on_edit_flag();
			}
			
/*			if (item.has_attribute("controllermode")) {
				std::string controllermode = item.attribute("controllermode");
				if (controllermode == "read")
					param.set_controller_read(); else
				if (controllermode == "write")
					param.set_controller_write(); else
				if (controllermode == "readwrite")
					param.set_controller_readwrite();
			}*/

			paramids.push_back(mp);

			return true;
		}

		bool setup_rect_from_xml(long& width, long& height, pug::xml_node item) {
			if (item.has_attribute("width")) {
				width = (long)item.attribute("width");
			} else {
				width = 0;
			}
			if (item.has_attribute("height")) {
				height = (long)item.attribute("height");
			} else {
				height = 0;
			}

			return true;
		}

		void parse_pin_names(pug::xml_node& parent, const std::string& name, std::vector<std::string>& result) {
			for (pug::xml_node::child_iterator parameter = parent.children_begin(); parameter != parent.children_end(); ++parameter) {
				if (!strcmp(parameter->name(), name.c_str())) {
					result.push_back(parameter->attribute("name").value());
				}
			}
		}

		int parse_outputs_inputs(pug::xml_node& parent, std::vector<std::string>& outputnames, std::vector<std::string>& inputnames) {
			int successbits = 0;
			outputnames.clear();
			inputnames.clear();
			if (!parent.empty()) {
				pug::xml_node outputs = parent.first_element_by_name("outputs");
				if (!outputs.empty()) {
					parse_pin_names(outputs, "output", outputnames);
					successbits |= iotype_output;
				}

				pug::xml_node inputs = parent.first_element_by_name("inputs");
				if (!inputs.empty()) {
					parse_pin_names(inputs, "input", inputnames);
					successbits |= iotype_input;
				}
			}
			return successbits;
		}

		bool init(const std::string& basepath, pug::xml_node item, zzub::archive *arc=0) {
			this->basepath = basepath;
			
			// info properties
			this->uri = new_string(item.attribute("uri").value());
			this->name = new_string(item.attribute("name").value());
			this->short_name = new_string(item.attribute("shortname").value());
			this->author = new_string(item.attribute("author").value());
			//this->commands = "Reload";
			
			if (item.has_attribute("mintracks")) {
				this->min_tracks = (long)item.attribute("mintracks");
			}
			if (item.has_attribute("maxtracks")) {
				this->max_tracks = (long)item.attribute("maxtracks");
			}
			
			// info type
			this->is_generic = false;
			std::string type = item.attribute("type").value();
			if (type == "generator") {
				this->inputs = 0;
				this->outputs = 2;
				this->flags = zzub_plugin_flag_has_audio_output;
			} else if (type == "effect") {
				this->inputs = 2;
				this->outputs = 2;
				this->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_audio_output;
			} else if (type == "controller") {
				this->inputs = 0;
				this->outputs = 0;
				this->flags = zzub_plugin_flag_has_event_output;
			} else if (type == "generic") {
				this->flags = 0;
				this->is_generic = true;
				pug::xml_node audio = item.first_element_by_name("audio");
				int success = parse_outputs_inputs(audio, audiooutputnames, audioinputnames);

				if (success & iotype_output) {
					assert(audiooutputnames.size() > 0);
					this->flags |= zzub_plugin_flag_has_audio_output;
					this->outputs = (int)audiooutputnames.size();
				} else
					this->outputs = 0;

				if (success & iotype_input) {
					assert(audioinputnames.size() > 0);
					this->flags |= zzub_plugin_flag_has_audio_input;
					this->inputs = (int)audioinputnames.size();
				} else
					this->inputs = 0;

				std::vector<std::string> outputnames;
				pug::xml_node midi = item.first_element_by_name("midi");
				success = parse_outputs_inputs(midi, outputnames, midiinputnames);
				if (success & iotype_output) {
					assert(outputnames.size() == 1);
					this->flags |= zzub_plugin_flag_has_midi_output;
				}
				if (success & iotype_input) {
					assert(midiinputnames.size() > 0);
					this->flags |= zzub_plugin_flag_has_midi_input;
				}

				pug::xml_node controller = item.first_element_by_name("controller");
				if (!controller.empty())
					this->flags |= zzub_plugin_flag_has_event_output;

				pug::xml_node note = item.first_element_by_name("note");
				if (!note.empty())
					this->flags |= zzub_plugin_flag_has_note_output;
			} else {
				std::cerr << "lunar: unknown value '" << type << "' for type attribute." << std::endl;
				return false;
			}

			std::string timer = item.attribute("timer").value();
			if (timer == "true")
				this->flags |= zzub_plugin_flag_is_interval;
			
			// enumerate parameters
			pug::xml_node parameters = item.first_element_by_name("parameters");
			if (!parameters.empty()) {
				// enumerate global parameters
				pug::xml_node global = parameters.first_element_by_name("global");
				if (!global.empty()) {
					for (pug::xml_node::child_iterator parameter = global.children_begin(); parameter != global.children_end(); ++parameter) {
						if (!strcmp(parameter->name(), "parameter")) {
							zzub::parameter &p = add_global_parameter();
							if (!setup_parameter_from_xml(gparamids, p, *parameter, parameters))
								return false;
						}
					}
				}
				// enumerate track parameters
				pug::xml_node track = parameters.first_element_by_name("track");
				if (!track.empty()) {
					for (pug::xml_node::child_iterator parameter = track.children_begin(); parameter != track.children_end(); ++parameter) {
						if (!strcmp(parameter->name(), "parameter")) {
							zzub::parameter &p = add_track_parameter();
							if (!setup_parameter_from_xml(tparamids, p, *parameter, parameters))
								return false;
						}
					}
				}
				// enumerate controller parameters
				pug::xml_node controller = parameters.first_element_by_name("controller");
				if (!controller.empty()) {
					for (pug::xml_node::child_iterator parameter = controller.children_begin(); parameter != controller.children_end(); ++parameter) {
						if (!strcmp(parameter->name(), "parameter")) {
							zzub::parameter &p = add_controller_parameter();
							if (!setup_parameter_from_xml(cparamids, p, *parameter, parameters))
								return false;
						}
					}
				}
			}

			// enumerate attributes
			pug::xml_node attribs = item.first_element_by_name("attributes");
			if (!attribs.empty()) {
				for (pug::xml_node::child_iterator attrib = attribs.children_begin(); attrib != attribs.children_end(); ++attrib) {
					if (!strcmp(attrib->name(), "attribute")) {
						zzub::attribute &a = add_attribute();
						if (!setup_attribute_from_xml(a, *attrib))
							return false;
					}
				}
			}

			// enumerate gui configuration
			pug::xml_node opengl = item.first_element_by_name("opengl");
			if (!opengl.empty()) {
				hasgui = true;
				
				guiinfo.suggestsize.x = guiinfo.suggestsize.y = 0;
				guiinfo.minsize.x = guiinfo.minsize.y = 0;
				guiinfo.maxsize.x = guiinfo.maxsize.y = 0;

				for (pug::xml_node::child_iterator param = opengl.children_begin(); param != opengl.children_end(); ++param) {
					if (!strcmp(param->name(), "suggestsize")) {
						if (!setup_rect_from_xml(guiinfo.suggestsize.x, guiinfo.suggestsize.y, *param))
							return false;
					}
				}
			} else {
				hasgui = false;
			}

			// enumerate required files
			pug::xml_node files = item.first_element_by_name("files");
			if (!files.empty()) {
				for (pug::xml_node::child_iterator file = files.children_begin(); file != files.children_end(); ++file) {
					if (!strcmp(file->name(), "file")) {
						struct stat fileinfo;
						std::string name = file->attribute("ref").value();
						std::string fullpath = basepath + "/" + name;
						int res = stat(fullpath.c_str(), &fileinfo);
						if (!res && !S_ISDIR(fileinfo.st_mode)) { // file exists
							// store a map entry
							this->files.insert(std::pair<std::string,std::string>(name,fullpath));
						} else if (arc) { // do we have an archive?
							// try to get file from stream
							zzub::instream* strm = arc->get_instream(name.c_str());
							if (strm) { // write stream to disk
								FILE *f = fopen(fullpath.c_str(), "wb");
								if (f) {
									std::vector<char> buffer;
									buffer.resize(strm->size());
									strm->read(&buffer[0], strm->size());
									size_t written = fwrite(&buffer[0], 1, buffer.size(), f);
									assert(written == strm->size());
									fclose(f);
									// everything ok, store a map entry
									this->files.insert(std::pair<std::string,std::string>(name,fullpath));
								} else {
									std::cerr << "lunar: couldn't open '" << fullpath << "' for writing." << std::endl;
									return false;
								}
							} else {
								std::cerr << "lunar: couldn't read '" << name << "' from stream." << std::endl;
								return false;
							}
						} else {
							std::cerr << "lunar: file '" << fullpath << "' does not exist." << std::endl;
							return false;
						}
					}
				}
			}
			
			// enumerate scripts to run
			pug::xml_node scripts = item.first_element_by_name("modules");
			if (!scripts.empty()) {
				for (pug::xml_node::child_iterator node = scripts.children_begin(); node != scripts.children_end(); ++node) {
					if (!strcmp(node->name(), "module")) {
						std::string ref = node->attribute("id").value();
						std::string path = basepath + "/" + ref;
						if (!load_module(path)) {
							std::cerr << "lunar: could not load module '" << ref << "'." << std::endl;
							return false;
						}
					}
				}
			}
			
			return true;
		}
		
		bool is_file(const std::string &ref) const {
			return strlen(get_file(ref).c_str())?true:false;
		}
		
		std::string get_file(const std::string &ref) const {
			std::map<std::string,std::string>::const_iterator i = files.find(ref);
			if (i == files.end())
				return "";
			return i->second;
		}
			
#if defined(LUNARTARGET_LLVM)
		void add_symbol_func(llvm::Module *M, const char *name, void *ptr) {
			Function *F = M->getNamedFunction(name);
			if (F && F->isExternal()) {
				std::cout << "exporting function '" << name << "' to module." << std::endl;
				EE->addGlobalMapping(F, ptr);
			}
		}

		void add_symbol_gval(llvm::Module *M, const char *name, void *ptr) {
			GlobalVariable *GV = M->getNamedGlobal(name);
			if (GV && GV->isExternal()) {
				std::cout << "exporting gval '" << name << "' to module." << std::endl;
				EE->addGlobalMapping(GV, ptr);
			}
		}
		
		void add_symbols(llvm::Module *M) {
			// math
			add_symbol_func(M, "lunar_pow", (void*)&lunar_pow);
			add_symbol_func(M, "lunar_log", (void*)&lunar_log);
			add_symbol_func(M, "lunar_exp", (void*)&lunar_exp);
			add_symbol_func(M, "lunar_min", (void*)&lunar_min);
			add_symbol_func(M, "lunar_max", (void*)&lunar_max);
			add_symbol_func(M, "lunar_abs", (void*)&lunar_abs);
			add_symbol_func(M, "lunar_sin", (void*)&lunar_sin);
			add_symbol_func(M, "lunar_cos", (void*)&lunar_cos);
			add_symbol_func(M, "lunar_tan", (void*)&lunar_tan);
			add_symbol_func(M, "lunar_sinh", (void*)&lunar_sinh);
			add_symbol_func(M, "lunar_cosh", (void*)&lunar_cosh);
			add_symbol_func(M, "lunar_atan", (void*)&lunar_atan);  
			add_symbol_func(M, "lunar_sqrt", (void*)&lunar_sqrt);
			add_symbol_func(M, "lunar_fabs", (void*)&lunar_fabs);
			add_symbol_func(M, "lunar_floor", (void*)&lunar_floor);
			add_symbol_func(M, "lunar_ceil", (void*)&lunar_ceil);
			add_symbol_func(M, "lunar_lrint", (void*)&lunar_lrint);

			// generic purpose
			add_symbol_func(M, "lunar_printf", (void*)&lunar_printf);
			add_symbol_func(M, "lunar_memset", (void*)&lunar_memset);
			add_symbol_func(M, "lunar_memcpy", (void*)&lunar_memcpy);
			add_symbol_func(M, "lunar_malloc", (void*)&lunar_malloc);
			add_symbol_func(M, "lunar_realloc", (void*)&lunar_realloc);
			add_symbol_func(M, "lunar_calloc", (void*)&lunar_calloc);
			add_symbol_func(M, "lunar_free", (void*)&lunar_free);
			
			// lunar api
			add_symbol_func(M, "lunar_is_envelope", (void*)&lunar_is_envelope);
			add_symbol_func(M, "lunar_get_envelope_sustain_position", (void*)&lunar_get_envelope_sustain_position);
			add_symbol_func(M, "lunar_get_envelope", (void*)&lunar_get_envelope);
			add_symbol_func(M, "lunar_new_voice", (void*)&lunar_new_voice);
			add_symbol_func(M, "lunar_voice_set_instrument", (void*)&lunar_voice_set_instrument);
			add_symbol_func(M, "lunar_voice_set_note", (void*)&lunar_voice_set_note);
			add_symbol_func(M, "lunar_voice_process_stereo", (void*)&lunar_voice_process_stereo);
			add_symbol_func(M, "lunar_voice_reset", (void*)&lunar_voice_reset);
		}
			
		void call_global_dtors_ctors(llvm::Module *M, bool dtors) {
			const char *name = dtors ? "llvm.global_dtors" : "llvm.global_ctors";
			GlobalVariable *GV = M->getNamedGlobal(name);
			if (!GV || GV->isExternal() || GV->hasInternalLinkage())
				return;
			// Should be an array of '{ int, void ()* }' structs.  The first value is
			// the init priority, which we ignore.
			ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
			if (!InitList)
				return;
			for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i)
				if (ConstantStruct *CS = dyn_cast<ConstantStruct>(InitList->getOperand(i))) {
					if (CS->getNumOperands() != 2) break; // Not array of 2-element structs.

					Constant *FP = CS->getOperand(1);
					if (FP->isNullValue())
						break;  // Found a null terminator, exit.

					if (ConstantExpr *CE = dyn_cast<ConstantExpr>(FP))
						if (CE->getOpcode() == Instruction::Cast)
							FP = CE->getOperand(0);
					if (Function *F = dyn_cast<Function>(FP)) {
					  // Execute the ctor/dtor function!
					  EE->runFunction(F, std::vector<GenericValue>());
					}
				}
		}
		
		void *get_symbol_func(llvm::Module *M, const char *name) {
			llvm::Function *F = M->getNamedFunction(name);
			if (F && !F->isExternal()) {
				return EE->getPointerToFunction(F);
			} else {
				std::cerr << "error: no such function '" << name << "' in module." << std::endl;
			}
			return 0;
		}
		
		bool get_symbols(llvm::Module *M) {
			new_fx = (lunar_new_fx_t)get_symbol_func(M, "new_fx");
			if (!new_fx) {
				std::cerr << "error: couldn't retrieve new_fx symbol." << std::endl;
				return false;
			}
			return true;
		}
#elif defined(LUNARTARGET_GCC)
		bool get_symbols(xp_modulehandle M) {
			new_fx = (lunar_new_fx_t)xp_dlsym(M, "new_fx");
			if (!new_fx) {
				std::cerr << "error: couldn't retrieve new_fx symbol." << std::endl;
				return false;
			}
			return true;
		}
#endif
		
		bool load_module(const std::string &path) {
			std::string fullpath;
#if defined(LUNARTARGET_LLVM)
			fullpath = path + ".bc";
			std::cout << "lunar: loading module '" << fullpath << "'" << std::endl;
			std::string errormsg;
			llvm::Module *M = ParseBytecodeFile(fullpath, &errormsg);
			if (M) {
				add_symbols(M);
				EE->addModuleProvider(new ExistingModuleProvider(M));
				if (!get_symbols(M))
					return false;
				call_global_dtors_ctors(M, false);
				modules.push_back(M);
				return true;
			} else {
				std::cerr << "lunar: error loading module '" << fullpath << "':" << errormsg << std::endl;
			}
#elif defined(LUNARTARGET_GCC)
			fullpath = path + SHLIBSUFFIX;
			std::cout << "lunar: loading module '" << fullpath << "'" << std::endl;
			xp_modulehandle m = xp_dlopen(fullpath.c_str());
			if (m) {
				if (!get_symbols(m)) {
					xp_dlclose(m);
					return false;
				}
				modules.push_back(m);
				return true;
			} else {
				std::cerr << "lunar: error loading module '" << fullpath << "': " << xp_dlerror() << std::endl;
			}
#endif
			return false;
		}
		
		virtual zzub::plugin* create_plugin() { return new dspplugin(*this); }
		virtual bool store_info(zzub::archive *arc) const {
			bool result = true;
			std::map<std::string,std::string>::const_iterator i;
			for (i = files.begin(); i != files.end(); ++i) {
				struct stat fileinfo;
				std::string name = i->first;
				std::string fullpath = i->second;
				int res = stat(fullpath.c_str(), &fileinfo);				
				if (!res && fileinfo.st_size) {
					char buffer[2048];
					FILE *f = fopen(fullpath.c_str(), "rb");
					zzub::outstream *strm = arc->get_outstream(name.c_str());
					while (!feof(f)) {
						size_t read = fread(buffer, 1, 2048, f);
						strm->write(buffer, read);
					}
					fclose(f);
				} else {
					std::cerr << "lunar: couldn't save data for file " << fullpath << std::endl;
					result = false;
				}
			}
			return result;
		}

	};

	const dspplugin::info& _info;
#if defined(WIN32)
	CLunarPluginGui pluginGui;
#endif

	size_t global_size;
	size_t track_size;
	size_t track_count;
	size_t controller_size;
	std::vector<size_t> global_offsets;
	std::vector<size_t> track_offsets;
	std::vector<size_t> controller_offsets;
	
	std::vector<float> gvalues;
	std::vector< std::vector<float> > tvalues;
	std::vector<float> cvalues;

	enum {
		MAX_GPARAMS = 64,
		MAX_CPARAMS = 64,
		MAX_TRACKS = 64,
		MAX_TPARAMS = 16,
	};
	
	float* grefs[MAX_GPARAMS];
	float* trefs[MAX_TRACKS * MAX_TPARAMS];
	float* crefs[MAX_CPARAMS];
	
	int get_value(int group, int track, int param) {
		const zzub::parameter *paraminfo = 0;
		char *offset = 0;
		if (group == 1) {
			paraminfo = _info.global_parameters[param];
			offset = (char*)global_values + global_offsets[param];
		} else if (group == 2) {
			paraminfo = _info.track_parameters[param];
			offset = &((char *)track_values)[track * track_size] + track_offsets[param];
		} else if (group == 3) {
			paraminfo = _info.controller_parameters[param];
			offset = (char*)controller_values + controller_offsets[param];
		}
		unsigned char value = *(unsigned char *)offset;
		switch(paraminfo->type) {
			case zzub_parameter_type_note:
			case zzub_parameter_type_byte:
			case zzub_parameter_type_switch:
			{
				unsigned char value = *(unsigned char *)offset;
				return (int)value;
			} break;
			case zzub_parameter_type_word:
			{
				unsigned short value = *(unsigned short *)offset;
				return (int)value;
			} break;
		}
		return 0;
	}
	
	virtual void set_track_count(int c) {
		track_count = (size_t)c;
	}
	
	void on_global_parameter_changed(int param, int value) {
		const metaparameter &mp = _info.gparamids[param];
		if (value == -1) {
			grefs[param] = 0;
		} else {
			gvalues[param] = mp.translate(value);
			grefs[param] = &gvalues[param];
		}
	}

	void on_track_parameter_changed(int track, int param, int value) {
		const metaparameter &mp = _info.tparamids[param];
    int trefindex = track * _info.track_parameters.size() + param;
		if (value == -1) {
			trefs[trefindex] = 0;
		} else {
			tvalues[track][param] = mp.translate(value);
			trefs[trefindex] = &tvalues[track][param];
		}
	}

	void on_controller_parameter_changed(int param, int value) {
		const metaparameter &mp = _info.cparamids[param];
		if (value == -1) {
			crefs[param] = 0;
		} else {
			cvalues[param] = mp.translate(value);
			crefs[param] = &cvalues[param];
		}
	}
	
	int get_final_controller_parameter(int param) {
		if (!crefs[param])
			return -1;
		cvalues[param] = *crefs[param];
		const metaparameter &mp = _info.cparamids[param];
		return mp.translate_back(cvalues[param]);
	}
	
	virtual void process_controller_events() {
		char *offset = (char *)controller_values + controller_size;
		int index = _info.controller_parameters.size();
		std::vector<const zzub::parameter *>::const_reverse_iterator i;
		for (i = _info.controller_parameters.rbegin(); i != _info.controller_parameters.rend(); ++i) {
			index--;
			offset -= (*i)->get_bytesize();
			switch ((*i)->type) {
				case zzub_parameter_type_note:
				case zzub_parameter_type_byte:
				case zzub_parameter_type_switch:
				{
					unsigned char value = *(unsigned char *)offset;
					if (value != (*i)->value_none) {
						on_controller_parameter_changed(index, (int)value);
					} else {
						on_controller_parameter_changed(index, -1);
					}
				} break;
				case zzub_parameter_type_word:
				{
					unsigned short value = *(unsigned short *)offset;
					if (value != (*i)->value_none) {
						on_controller_parameter_changed(index, (int)value);
					} else {
						on_controller_parameter_changed(index, -1);
					}
				} break;
			}
		}
		call_process_controller_events();
		// write back controller values
		offset = (char *)controller_values + controller_size;
		index = _info.controller_parameters.size();
		for (i = _info.controller_parameters.rbegin(); i != _info.controller_parameters.rend(); ++i) {
			index--;
			offset -= (*i)->get_bytesize();
			int value = get_final_controller_parameter(index);
			if (value == -1)
				value = (*i)->value_none;
			switch ((*i)->type) {
				case zzub_parameter_type_note:
				case zzub_parameter_type_byte:
				case zzub_parameter_type_switch:
				{
					unsigned char *pvalue = (unsigned char *)offset;
					*pvalue = (unsigned char)value;
				} break;
				case zzub_parameter_type_word:
				{
					unsigned short *pvalue = (unsigned short *)offset;
					*pvalue = (unsigned short)value;
				} break;
			}
		}
	}

	virtual void process_events() {
		update_masterinfo_fields();
		std::vector<const zzub::parameter *>::const_reverse_iterator i;
		char *offset = (char *)global_values + global_size;
		int index = _info.global_parameters.size();
		for (i = _info.global_parameters.rbegin(); i != _info.global_parameters.rend(); ++i) {
			index--;
			offset -= (*i)->get_bytesize();
			switch ((*i)->type) {
				case zzub_parameter_type_note:
				case zzub_parameter_type_byte:
				case zzub_parameter_type_switch:
				{
					unsigned char value = *(unsigned char *)offset;
					if (value != (*i)->value_none) {
						on_global_parameter_changed(index, (int)value);
					} else {
						on_global_parameter_changed(index, -1);
					}
				} break;
				case zzub_parameter_type_word:
				{
					unsigned short value = *(unsigned short *)offset;
					if (value != (*i)->value_none) {
						on_global_parameter_changed(index, (int)value);
					} else {
						on_global_parameter_changed(index, -1);
					}
				} break;
			}
		}
		for (size_t t = 0; t < track_count; ++t) {
			index = _info.track_parameters.size();
			offset = &((char *)track_values)[t * track_size] + track_size;
			for (i = _info.track_parameters.rbegin(); i != _info.track_parameters.rend(); ++i) {
				index--;
				offset -= (*i)->get_bytesize();
				switch ((*i)->type) {
					case zzub_parameter_type_note:
					case zzub_parameter_type_byte:
					case zzub_parameter_type_switch:
					{
						unsigned char value = *(unsigned char *)offset;
						if (value != (*i)->value_none) {
							on_track_parameter_changed((int)t, index, (int)value);
						} else {
							on_track_parameter_changed((int)t, index, -1);
						}
					} break;
					case zzub_parameter_type_word:
					{
						unsigned short value = *(unsigned short *)offset;
						if (value != (*i)->value_none) {
							on_track_parameter_changed((int)t, index, (int)value);
						} else {
							on_track_parameter_changed((int)t, index, -1);
						}
					} break;
				}
			}
		}
		call_process_events();
	}

	virtual void get_midi_output_names(zzub::outstream* outs) {
		for (std::vector<std::string>::const_iterator i = _info.midiinputnames.begin(); i != _info.midiinputnames.end(); ++i) {
			outs->write(i->c_str());
		}
	}

	virtual void process_midi_events(zzub::midi_message* pin, int nummessages) {
		if (fxcopy.size >= 88 && fxcopy.process_midi) {
			fxcopy.process_midi(fx, reinterpret_cast<lunar_midi_message_t*>(pin), nummessages);
		}
	}
	
	virtual ~dspplugin() {
		exit_host();
		if (global_values)
			delete[] (char*)global_values;
		if (track_values)
			delete[] (char*)track_values;
		if (controller_values)
			delete[] (char*)controller_values;
		if (attributes)
			delete[] attributes;
	}
	
	struct stereopair {
		float *buffers[2];
	};
	std::list< std::vector<float> > buffers; // list of buffers
	std::list< stereopair > stereopairs; // list of stereopairs
	zzub::master_info last_master_info;
	
	float *create_buffer(int size) {
		buffers.push_back(std::vector<float>());
		std::vector<float> &buffer = buffers.back();
		buffer.resize(size);
		return &buffer[0];
	}
	
	float **create_stereopair(int size) {
		stereopairs.push_back(stereopair());
		stereopair &pairs = stereopairs.back();
		pairs.buffers[0] = create_buffer(size);
		pairs.buffers[1] = create_buffer(size);
		return pairs.buffers;
	}
	
	lunar_fx fxcopy;
	lunar_fx *fx;
	lunar_transport_t transport;
	_lunar_host host;
	zzub::host host_cb;
	lunar_gui_params_t guiparams;

	int silencecount; // how many samples there has been no signal on i/o

	static void _request_gui_redraw(lunar_host_t *lunar_host);
	static void _midi_out(lunar_host_t *lunar_host, lunar_midi_message_t* outm, int n);
	static void _note_out(lunar_host_t *lunar_host, lunar_note_message_t* outn, int n);
	static bool _is_channel_connected(lunar_host_t * lunar_host, int index);

	dspplugin(const dspplugin::info &dspinfo) : _info(dspinfo) {
		fx = dspinfo.new_fx();
		assert((fx->size > 0) && (fx->size <= sizeof(lunar_fx)));
		// make a safe copy
		lunar_init_fx(&fxcopy);
		// copy how much we can copy,
		// the rest of the struct will be empty
		memcpy(&fxcopy, fx, std::min(sizeof(lunar_fx), (size_t)fx->size));
		global_size = 0;
		track_size = 0;
		track_count = _info.min_tracks;
		controller_size = 0;
		char *offset = 0;
		std::vector<const zzub::parameter *>::const_iterator i;
		for (i = _info.global_parameters.begin(); i != _info.global_parameters.end(); ++i) {
			global_offsets.push_back(global_size);
			global_size += (*i)->get_bytesize();
		}
		for (i = _info.track_parameters.begin(); i != _info.track_parameters.end(); ++i) {
			track_offsets.push_back(track_size);
			track_size += (*i)->get_bytesize();
		}
		for (i = _info.controller_parameters.begin(); i != _info.controller_parameters.end(); ++i) {
			controller_offsets.push_back(controller_size);
			controller_size += (*i)->get_bytesize();
		}
		if (global_size) {
			assert(_info.global_parameters.size() <= MAX_GPARAMS);
			global_values = new char[global_size];
			gvalues.resize(_info.global_parameters.size());
		}
		if (track_size) {
			assert(_info.max_tracks <= MAX_TRACKS);
			assert(_info.track_parameters.size() <= MAX_TPARAMS);
			track_values = new char[track_size * _info.max_tracks];
			tvalues.resize(_info.max_tracks);
			for (int t = 0; t < _info.max_tracks; ++t) {
				tvalues[t].resize(_info.track_parameters.size());
			}
		}
		if (controller_size) {
			assert(_info.controller_parameters.size() <= MAX_CPARAMS);
			controller_values = new char[controller_size];
			cvalues.resize(_info.controller_parameters.size());
		}
		if (_info.attributes.size())
			attributes = new int[_info.attributes.size()];

		// initialize fx members
		fx->transport = &transport;
		fx->host = &host;
		fx->globals = &grefs[0];
		fx->tracks = &trefs[0];
		fx->controllers = &crefs[0];
		fx->attributes = attributes;
		silencecount = 0;
		memset(&last_master_info, 0, sizeof(zzub::master_info));
	}
	
	void exit_host() {
		if (fxcopy.exit) {
			fxcopy.exit(fx);
		}
	}
	
	void update_masterinfo_fields() {
		transport.samples_per_tick = float(_master_info->samples_per_tick) + _master_info->samples_per_tick_frac;
		transport.beats_per_minute = _master_info->beats_per_minute;
		transport.ticks_per_beat = _master_info->ticks_per_beat;
		transport.samples_per_second = _master_info->samples_per_second;
		transport.tick_position = _master_info->tick_position;
		transport.ticks_per_second = _master_info->ticks_per_second;
		transport.song_position = _mixer->song_position;
		fx->track_count = track_count;
	}
	
	void check_masterinfo_changed() {
		if ((last_master_info.beats_per_minute != _master_info->beats_per_minute)
			||(last_master_info.samples_per_second != _master_info->samples_per_second)
		||(last_master_info.ticks_per_beat != _master_info->ticks_per_beat))
		{
			last_master_info = *_master_info;
			call_transport_changed();
		}
	}
	
	virtual void init(zzub::archive *)
	{
		//host.cb = 0;//this->_host;
		host_cb.request_gui_redraw = _request_gui_redraw;
		host_cb.midi_out = _midi_out;
		host_cb.note_out = _note_out;
		host_cb.is_channel_connected = _is_channel_connected;
		host.cb = &host_cb;

		update_masterinfo_fields();
		
		if (fxcopy.init) {
			fxcopy.init(fx);
		}

		{
			// initialize with default values
			std::vector<const zzub::parameter *>::const_reverse_iterator i;
			int index = _info.global_parameters.size();
			for (i = _info.global_parameters.rbegin(); i != _info.global_parameters.rend(); ++i) {
				index--;
				if ((*i)->flags & zzub_parameter_flag_state) {
					on_global_parameter_changed(index, (*i)->value_default);
				} else {
					on_global_parameter_changed(index, -1);
				}
			}
			for (size_t t = 0; t < _info.max_tracks; ++t) {
				index = _info.track_parameters.size();
				for (i = _info.track_parameters.rbegin(); i != _info.track_parameters.rend(); ++i) {
					index--;
					if ((*i)->flags & zzub_parameter_flag_state) {
						on_track_parameter_changed((int)t, index, (*i)->value_default);
					} else {
						on_track_parameter_changed((int)t, index, -1);
					}
				}
			}
			index = _info.controller_parameters.size();
			for (i = _info.controller_parameters.rbegin(); i != _info.controller_parameters.rend(); ++i) {
				index--;
				if ((*i)->flags & zzub_parameter_flag_state) {
					on_controller_parameter_changed(index, (*i)->value_default);
				} else {
					on_controller_parameter_changed(index, -1);
				}
			}
		}
		
		call_process_events();
	}
	
	virtual void command(int i) {
	}
	
	virtual void load(zzub::archive *arc) {}
	virtual void save(zzub::archive *) {}
		
	const char *describe_value(const metaparameter &mp, int value) {
		static char s[128];
		float lv;
		if (mp.isfloat) {
			lv = mp.translate(value);
		} else {
			lv = float(value);
		}
		std::map<float,std::string>::const_iterator i = mp.valuenames.find(lv);
		if (i != mp.valuenames.end()) {
			sprintf(s, "%s", i->second.c_str());
			return s;
		}
		if (mp.isfloat) {
			sprintf(s, "%.2f", mp.translate(value));
		} else {
			sprintf(s, "%i", value);
		}
		return s;
	}
	
	virtual const char * describe_value(int param, int value) {

		if (fxcopy.describe_value) {
			const char *str = fxcopy.describe_value(fx, lunar_parameter_names_t(param), value);
			if (str) {
				return str;
			}
		}

		if (param < (int)_info.global_parameters.size()) {
			return describe_value(_info.gparamids[param], value);
		} else {
			param -= _info.global_parameters.size();
			return describe_value(_info.tparamids[param], value);
		}
		return 0;
	}
	
	void call_transport_changed() {
		if (fxcopy.transport_changed) {
			fxcopy.transport_changed(fx);

		}
	}
	
	void call_process_events() {
		if (fxcopy.process_events) {
			fxcopy.process_events(fx);

		}
	}
	
	void call_process_controller_events() {
		if (fxcopy.process_controller_events) {
			fxcopy.process_controller_events(fx);
		}
	}

	void call_attributes_changed() {
		if (fxcopy.attributes_changed) {
			fxcopy.attributes_changed(fx);
		}
	}
	
	virtual int get_interval_size() {
		int numsamples = _master_info->samples_per_second;
		int processevents = 0;
		transport.song_position = _mixer->song_position;

		if (fxcopy.process_timer)
			fxcopy.process_timer(fx, &numsamples, &processevents);
		return numsamples;
	}

	virtual int get_latency() { 
		if (fxcopy.size >= 72 && fxcopy.get_latency) 
			return fxcopy.get_latency(fx);
		return 0;
	}

	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		check_masterinfo_changed();

		if (_info.is_generic && fxcopy.process_audio) {
			if ((mode&zzub_process_mode_read) == 0)
				for (int i = 0; i < _info.inputs; i++)
					pin[i] = 0;
			if ((mode&zzub_process_mode_write) == 0)
				for (int i = 0; i < _info.outputs; i++)
					pout[i] = 0;
			return fxcopy.process_audio(fx, pin, pout, numsamples);
		} else if (!_info.is_generic && fxcopy.process_stereo) {

			if( (mode&zzub_process_mode_write)==0 )
				return false;
			// if we are an effect, the input is empty and there was more than one second of silence,
			// don't generate any data.
			
			if (_info.flags & zzub_plugin_flag_has_audio_input) {
				if (!(mode&zzub_process_mode_read)) {
					if (silencecount > _master_info->samples_per_second)
						return false;
				} else {
					silencecount = 0;
				}
			}

			// use empty buffers instead of null-buffers for unconnected audio channels
			float* plin[2] = {0};
			float* plout[2] = {0};
			float zerobuffer[2][zzub_buffer_size] = {0.0f};
			float tempbuffer[2][zzub_buffer_size] = {0.0f};
			for (int i = 0; i < _info.inputs; i++)
				plin[i] = (pin[i] != 0) ? pin[i] : zerobuffer[i];
			for (int i = 0; i < _info.outputs; i++)
				plout[i] = (pout[i] != 0) ? pout[i] : tempbuffer[i];

			fxcopy.process_stereo(fx,plin[0],plin[1],plout[0],plout[1],numsamples);
			if ((pout[0] && zzub::buffer_has_signals(pout[0], numsamples)) || (pout[1] && zzub::buffer_has_signals(pout[1], numsamples))) {
				silencecount = 0;
				return true;
			}
			silencecount += numsamples;
			return false;
		}
		return false;
	}
	
	// ::zzub::plugin methods
	virtual void destroy() { delete this; }
	virtual void stop() {
		update_masterinfo_fields();
		std::vector<const zzub::parameter *>::const_reverse_iterator i;
		int index = _info.global_parameters.size();
		for (i = _info.global_parameters.rbegin(); i != _info.global_parameters.rend(); ++i) {
			index--;
			switch ((*i)->type) {
				case zzub_parameter_type_note:
					on_global_parameter_changed(index, zzub_note_value_off);
					break;
				default:
					on_global_parameter_changed(index, -1);
					break;
			}
		}
		for (size_t t = 0; t < track_count; ++t) {
			index = _info.track_parameters.size();
			for (i = _info.track_parameters.rbegin(); i != _info.track_parameters.rend(); ++i) {
				index--;
				switch ((*i)->type) {
					case zzub_parameter_type_note:
						on_track_parameter_changed((int)t, index, zzub_note_value_off);
						break;
					default:
						on_track_parameter_changed((int)t, index, -1);
						break;
				}
			}
		}
		call_process_events();
	}
	virtual void attributes_changed() {
		call_attributes_changed();
	}
#if defined(WIN32)
	virtual bool has_embedded_gui() { return _info.hasgui; }
	virtual bool create_embedded_gui(void* hwnd) {
		guiparams.fx = fx;
		guiparams.info = &_info.guiinfo;
		HWND hResult = pluginGui.Create((HWND)hwnd, 0, 0, WS_CHILD|WS_VISIBLE, 0, (HMENU)0, &guiparams);
		//if (hResult != 0) pluginGui.OpenGLInit();
		return hResult != 0;
	}

	virtual void resize_embedded_gui(void* hwnd, int* outwidth, int* outheight) {
		pluginGui.UpdateSize(outwidth, outheight);
	}
#endif

	virtual const char* get_output_channel_name(int i) { 
		if (!_info.is_generic)
			return 0;
		return _info.audiooutputnames[i].c_str();
	}

	virtual const char* get_input_channel_name(int i) { 
		if (!_info.is_generic)
			return 0;
		return _info.audioinputnames[i].c_str();
	}

};

void dspplugin::_midi_out(lunar_host_t *lunar_host, lunar_midi_message_t* outm, int n) {
	// get containing dspplugin address
	dspplugin* plugin = reinterpret_cast<dspplugin*>(
		reinterpret_cast<uintptr_t>(lunar_host) -
		reinterpret_cast<uintptr_t>(&(reinterpret_cast<dspplugin*>(NULL)->host)));
	
	for (int i = 0; i < n; i++) {
		plugin->_mixer->midi_out(plugin->_id, *reinterpret_cast<zzub::midi_message*>(&outm[i]));
	}
}

void dspplugin::_note_out(lunar_host_t *lunar_host, lunar_note_message_t* outn, int n) {
	// get containing dspplugin address
	dspplugin* plugin = reinterpret_cast<dspplugin*>(
		reinterpret_cast<uintptr_t>(lunar_host) -
		reinterpret_cast<uintptr_t>(&(reinterpret_cast<dspplugin*>(NULL)->host)));
	
	for (int i = 0; i < n; i++) {
		plugin->_mixer->note_out(plugin->_id, *reinterpret_cast<zzub::note_message*>(&outn[i]));
	}
}

void dspplugin::_request_gui_redraw(lunar_host_t *lunar_host)
{
	// get containing dspplugin address
	dspplugin* plugin = reinterpret_cast<dspplugin*>(
		reinterpret_cast<uintptr_t>(lunar_host) -
		reinterpret_cast<uintptr_t>(&(reinterpret_cast<dspplugin*>(NULL)->host)));
#if defined(WIN32)
	plugin->pluginGui.RequestRedraw();
#endif
}

bool dspplugin::_is_channel_connected(lunar_host_t *lunar_host, int index) {
	// get containing dspplugin address
	dspplugin* plugin = reinterpret_cast<dspplugin*>(
		reinterpret_cast<uintptr_t>(lunar_host) -
		reinterpret_cast<uintptr_t>(&(reinterpret_cast<dspplugin*>(NULL)->host)));
	
	return plugin->_mixer->is_input_channel_connected(plugin->_id, index);
}

void digest_to_hex(const uint8_t digest[SHA1_DIGEST_SIZE], char *output)
{
	int i;
	char *c = output;
	
	for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
		sprintf(c,"%02x", digest[i]);
		c += 2;
	}
	*c = '\0';
}

std::string hash_name(const std::string &name) {
	SHA1_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const uint8_t*)name.c_str(), strlen(name.c_str()));
	unsigned char digest[SHA1_DIGEST_SIZE];
	SHA1_Final(&ctx, digest);
	char buffer[128];
	digest_to_hex(digest, buffer);
	return buffer;
}

#if defined(_WIN32)
	#define PATHSEP "\\"
#else
	#define PATHSEP "/"
#endif

struct dspplugincollection : zzub::plugincollection {
	std::list<dspplugin::info*> infos;
	std::string storagedir;
	
	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return "@zzub.org/plugincollections/lunar"; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {
		if (!strcmp(key, "local_storage_dir")) {
			storagedir = value;
		}
		if (!strcmp(key, "register_plugin")) {
			assert(false); // ??
			//register_plugin(value);
		}
	}
	
	dspplugincollection() {
#if defined(POSIX)
		storagedir = "/tmp";
#elif defined(_WIN32)
		char path[MAX_PATH];
		GetTempPath(MAX_PATH, path);
		storagedir = path;
#endif

#if defined(LUNARTARGET_LLVM)
		std::cout << "Creating new Module" << std::endl;
		Module *M = new Module("lunar");
		/*
		M->setTargetTriple("i686-pc-linux-gnu");
		M->setEndianness(llvm::Module::LittleEndian);
		M->setPointerSize(llvm::Module::Pointer32);
		*/
		std::cout << "Creating new ExistingModuleProvider" << std::endl;
		ModuleProvider *MP = new ExistingModuleProvider(M);
		std::cout << "Creating ExecutionEngine" << std::endl;
		EE = ExecutionEngine::create(MP, false);
		assert(EE);
#endif
	}
	
	~dspplugincollection() {
		for (std::list<dspplugin::info*>::iterator i = infos.begin(); i != infos.end(); ++i) {
			delete (*i);
		}
#if defined(LUNARTARGET_LLVM)
		if (EE) {
			delete EE;
			EE = 0;
		}
#endif
	}
	
	
	void enumerate_plugins(const std::string &folder, zzub::pluginfactory *factory) {
		// XXX: support win32
#if defined(POSIX)
		struct dirent **namelist;
		struct stat statinfo;
		int n;
		
		std::string fullpath = folder + "/";
		
		n = scandir(fullpath.c_str(), &namelist, 0, alphasort);
		if (n < 0)
			return;
		else {
			while(n--) {
				if (strcmp(namelist[n]->d_name,".") && strcmp(namelist[n]->d_name,"..")) {
					std::string fullFilePath=fullpath + namelist[n]->d_name;
					std::cout << "lunar: enumerating folder '" << fullFilePath << "'" << std::endl;
					if (!stat(fullFilePath.c_str(), &statinfo))
					{
						if (S_ISDIR(statinfo.st_mode))
						{
							register_plugin(fullFilePath, factory);
						}
					}
				}
				free(namelist[n]);
			}
			free(namelist);
		}
#elif defined(_WIN32)
	std::string searchPath=folder + PATHSEP"*";
	std::cout << "lunar: enumerating " << searchPath << std::endl;

	WIN32_FIND_DATA fd;
	HANDLE hFind=FindFirstFile(searchPath.c_str(), &fd);

	while (hFind!=INVALID_HANDLE_VALUE) {
		std::string fullFilePath=folder + PATHSEP + fd.cFileName;
		std::cout << "lunar: enumerating '" << fullFilePath << "'" << std::endl;
		if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {			
			std::cout << "lunar: enumerating folder '" << fullFilePath << "'" << std::endl;
			char fullPath[1024];
			char* filePart;
			GetFullPathName(fullFilePath.c_str(), 1024, fullPath, &filePart);

			register_plugin(fullPath, factory);
		}

		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);		
#endif
	}
	
	virtual void initialize(zzub::pluginfactory *factory) {
		const char* loc = setlocale(LC_NUMERIC, "C");
		
#if defined(_WIN32)
		// The Lunar Audio Plugins and SDK must have been installed using the .msi
		char* programFiles = getenv("PROGRAMFILES");
		if (programFiles != 0) {
			std::string lunarPath = (std::string)programFiles + "\\Lunar\\fx";
			enumerate_plugins(lunarPath, factory);
		}
#else
		enumerate_plugins("/usr/local/lib64/lunar/fx", factory);
		enumerate_plugins("/usr/local/lib/lunar/fx", factory);
		enumerate_plugins("/usr/lib64/lunar/fx", factory);
		enumerate_plugins("/usr/lib/lunar/fx", factory);
#endif
		
		for (std::list<dspplugin::info*>::iterator i = infos.begin(); i != infos.end(); ++i) {
			factory->register_info(*i);
		}
		setlocale(LC_NUMERIC, loc);
	}
	
	virtual zzub::info *get_info(const char *uri, zzub::archive *arc) {
		const char* loc = setlocale(LC_NUMERIC, "C");
		std::cout << "lunar: searching for '" << uri << "' in archive" << std::endl;
		zzub::info *result = 0;
		zzub::instream *strm = arc->get_instream("manifest.xml");
		if (strm) {
			char *manifestdata = new char[strm->size()+1];
			manifestdata[strm->size()] = '\0';
			strm->read(manifestdata, strm->size());
			std::string manifestdata_original = manifestdata; // untouched copy
			pug::xml_parser xml;
			if (xml.parse(manifestdata)) {
				pug::xml_node root = xml.document();
				pug::xml_node zzubnode = root.first_element_by_name("zzub");
				if (!zzubnode.empty()) {
					for (pug::xml_node::child_iterator item = zzubnode.children_begin(); item != zzubnode.children_end(); ++item) {
						if (item->has_name("plugin") && item->attribute("uri").has_value(uri)) {
							std::cout << "lunar: building info for '" << uri << "'." << std::endl;
							// we can import this one
							std::string fullpath = storagedir + PATHSEP + hash_name(uri);
							// make sure the path exists
							struct stat dirinfo;
							int res = stat(fullpath.c_str(), &dirinfo);
							if (res) {
								// create dir
								if (mkdir(fullpath.c_str(), 0755)) {
									std::cerr << "could not create directory " << fullpath << std::endl;
								}
							}							
							std::string manifestpath = fullpath + PATHSEP"manifest.xml";							
							FILE *f = fopen(manifestpath.c_str(), "wb");							
							if (f) {
								size_t written = fwrite(manifestdata_original.c_str(), 1, strlen(manifestdata_original.c_str()), f);
								assert(written == strlen(manifestdata_original.c_str()));
								fclose(f);
								dspplugin::info *_info = new dspplugin::info();
								if (_info->init(fullpath, *item, arc)) {
									_info->collection = this;
									// add manifest to list of available files
									_info->files.insert(std::pair<std::string,std::string>("manifest.xml", manifestpath));
									result = _info;
								} else {
									delete _info;
								}
							} else {
								std::cerr << "couldn't export manifest to " << manifestpath << std::endl;
							}							
						}
					}
				} else {
					std::cerr << "lunar: document node is not zzub." << std::endl;
				}
				delete[] manifestdata;
			} else {
				std::cerr << "lunar: error parsing xml while reading data for " << uri << std::endl;
			}
		} else {
			std::cerr << "lunar: no '" << uri << "' in archive." << std::endl;
		}
		setlocale(LC_NUMERIC, loc);
		return result;
	}
	virtual void destroy() {
	}
	
	void register_plugin(const std::string &fullpath, zzub::pluginfactory *factory) {
		std::string manifestpath = fullpath + PATHSEP"manifest.xml";
		struct stat statinfo;
		if (stat(manifestpath.c_str(), &statinfo)) {
			std::cerr << "error: " << manifestpath << " does not exist." << std::endl;
			return;
		}
		if (S_ISDIR(statinfo.st_mode)) {
			std::cerr << "error: " << manifestpath << " is a folder, not a file." << std::endl;
			return;
		}
		pug::xml_parser xml;
		if (xml.parse_file(manifestpath.c_str())) {
			pug::xml_node root = xml.document();
			pug::xml_node zzubnode = root.first_element_by_name("zzub");
			if (!zzubnode.empty()) {
				for (pug::xml_node::child_iterator item = zzubnode.children_begin(); item != zzubnode.children_end(); ++item) {
					if (!strcmp(item->name(), "plugin")) {
						dspplugin::info *_info = new dspplugin::info();
						if (_info->init(fullpath, *item)) {
							_info->collection = this;
							// add manifest to list of available files
							_info->files.insert(std::pair<std::string,std::string>("manifest.xml", manifestpath));
							infos.push_back(_info);
						} else {
							delete _info;
						}
						
					}
				}
			} else {
				std::cerr << "lunar: no zzub node in '" << manifestpath << "'." << std::endl;
			}
		} else {
			std::cerr << "lunar: error loading manifest from '" << manifestpath << "'." << std::endl;
		}
	}
	
	const char* get_name() { return "Lunar"; }

};

static dspplugincollection the_collection;

} // namespace lunar

zzub::plugincollection *lunar_get_plugincollection() {
	return &lunar::the_collection;
}
/*
zzub::plugincollection *zzub_get_plugincollection() {
	return &lunar::the_collection;
}*/

void lunar_register_plugin(const char *path) {
//	lunar::the_collection.register_plugin(path);
	assert(false);
}

void lunar_set_local_storage_dir(const char *path) {
	lunar::the_collection.storagedir = path;
}

/*const char *zzub_get_signature() { return ZZUB_SIGNATURE; }

#if defined(_WIN32)

BOOL WINAPI DllMain( HMODULE hModule, DWORD fdwreason, LPVOID lpReserved )
{
	switch(fdwreason)
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;
		} break;
		case DLL_PROCESS_DETACH:
		{
		} break;
		default:
			break;
	}

	return TRUE;
}
#endif
*/
