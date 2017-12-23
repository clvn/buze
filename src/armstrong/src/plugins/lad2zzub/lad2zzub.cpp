/* lad2zzub.cpp

   Copyright (C) 2007 Frank Potulski (polac@gmx.de)
   Copyright (C) 2012 Armstrong Developers

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
   USA. */

#include <windows.h>
#include <list>
#include <set>
#include <map>
#include <zzub/plugin.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "../plugincache.h"
#include "ladspa/ladspa.h"
#include "ladspacollection.h"

inline float scale(float x,float min,float max) {
	return expf( logf(min) * (1.0f-x) + logf(max) * x );
}

//**** CLadspa ****************************************************************************************************************************************

const LADSPA_Descriptor* ladspa_get_descriptor(HMODULE hm, unsigned long index);

CLadspa::CLadspa(CLadspaInfo *_info) : info(_info), ladspaInitialized(false), inertia(eDefaultInertia) {
	assert(_info != 0);

	global_values = 0;
	track_values = 0;
	attributes = 0;

	// match with free_library in destroy to use the HMODULE-refcount to track the descriptor
	// TODO: could do this more transparently in the info-class
	HMODULE hm = info->owner->load_library(info->pluginfile.c_str());
	if (!hm) return ;

	if (info->lad_descriptor == 0) {
		info->lad_descriptor = ladspa_get_descriptor(hm, info->lad_index);
		if (info->lad_descriptor == 0) return ;
	}

	info->fill_parameter_info();

	inbuffers.resize(info->inputs);
	for (int i = 0; i < info->inputs; i++) {
		inbuffers[i] = boost::shared_array<float>(new float[zzub_buffer_size]);
		memset(inbuffers[i].get(), 0, zzub_buffer_size * sizeof(float));
	}

	outbuffers.resize(info->outputs);
	for (int i = 0; i < info->outputs; i++) {
		outbuffers[i] = boost::shared_array<float>(new float[zzub_buffer_size]);
		memset(outbuffers[i].get(), 0, zzub_buffer_size * sizeof(float));
	}

	paramInertia.resize(info->global_parameters.size());
	pluginparams.resize(info->global_parameters.size());
	pluginoutparams.resize(info->lad_outparameters.size());
	params.resize(info->global_parameters.size());
	global_values = &params.front();
}

CLadspa::~CLadspa() {
}

void CLadspa::init(zzub::archive *arc) {
	inertia = eDefaultInertia;
	handle = info->lad_descriptor->instantiate(info->lad_descriptor, _master_info->samples_per_second);
	if (!handle) return;

	int paramcount = 0;
	int outparamcount = 0;
	int inbufcount = 0;
	int outbufcount = 0;

	for (int i = 0; i < (int)info->lad_descriptor->PortCount; i++) {
		if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_CONTROL) {
			if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_INPUT) {

				const zzub::parameter* param = info->global_parameters[paramcount];
				paramInertia[paramcount].newParam = param->value_default;
				paramInertia[paramcount].curParam = float(paramInertia[paramcount].newParam);
				paramInertia[paramcount].inertia = 0;

				info->lad_descriptor->connect_port(handle, i, &pluginparams[paramcount]);
				paramcount++;
			} else if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_OUTPUT) {
				// peer!
				info->lad_descriptor->connect_port(handle, i, &pluginoutparams[outparamcount]);
				outparamcount++;
			}
		} else if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_AUDIO) {
			if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_INPUT) {
				info->lad_descriptor->connect_port(handle, i, inbuffers[inbufcount++].get());
			} else if (info->lad_descriptor->PortDescriptors[i] & LADSPA_PORT_OUTPUT) {
				info->lad_descriptor->connect_port(handle, i, outbuffers[outbufcount++].get());
			}
		}
	}

	if (info->lad_descriptor->activate) {
		info->lad_descriptor->activate(handle);
	}

	if (arc) {
		int version = 0;

		zzub::instream *istr = arc->get_instream("");
		
		if ( istr && istr->size() == sizeof(int) ) {
			istr->read<int>(version);			
		}

		ladspaInitialized = (version==eVersion) ? true : false;		
	} else {
		ladspaInitialized = true;
	}	
}

void CLadspa::save(zzub::archive *arc) {
	if (!arc) return;
	if (!ladspaInitialized) return;
		
	zzub::outstream *ostr = arc->get_outstream("");

	if (ostr) {
		ostr->write<int>(eVersion);
	}
}

void CLadspa::destroy(void) {	

	if (info->lad_descriptor->deactivate) {
		info->lad_descriptor->deactivate(handle);
	}
	
	if (info->lad_descriptor->cleanup) {
		try {
			info->lad_descriptor->cleanup(handle);
		}
		catch(...){}
	}

	if (info->owner->free_library(info->pluginfile.c_str()))
		info->lad_descriptor = 0;

	delete this;
}

void CLadspa::set_parameter(int idx, float val) {
	//if (idx < 0 || idx >= numInParams) return;	
//	if (!inparams) return;

	PARAMS *p = &info->lad_parameters[idx];

	if ( LADSPA_IS_HINT_INTEGER(p->hint) ) {
		pluginparams[idx] = (int) ( p->min + ( (p->max - p->min) * val ) );
	} else if ( LADSPA_IS_HINT_TOGGLED(p->hint) ) {
		pluginparams[idx] =(int)(val+0.5f);
	} else if ( LADSPA_IS_HINT_LOGARITHMIC(p->hint) ) {
		pluginparams[idx] = scale(val,p->min,p->max);
	} else {
		pluginparams[idx] =  p->min + ( (p->max - p->min) * val );
	}
}

float CLadspa::get_parameter(int idx, float val) {
	//if (!inparams || idx < 0 || idx >= numInParams) return 0.0f;

	if (val != -1.0f) {
		float ret;

		const PARAMS *p = &info->lad_parameters[idx];

		if ( LADSPA_IS_HINT_INTEGER(p->hint) ) {
			ret = float ( (int) ( p->min + ( (p->max - p->min) * val ) ) );
		} else if ( LADSPA_IS_HINT_TOGGLED(p->hint) ) {
			ret = float ( (int)(val+0.5f) );
		} else if ( LADSPA_IS_HINT_LOGARITHMIC(p->hint) ) {
			ret = scale(val,p->min,p->max);
		} else {
			ret = p->min + ( (p->max - p->min) * val );
		}

		return ret;
	}

	return pluginparams[idx];
}

void CLadspa::process_events(void) {
	assert(info->global_parameters.size() >= 1);

	if (!ladspaInitialized) return;

	const int n = (int)info->global_parameters.size() - 1; //ladspa->getNumParams();

	const unsigned short &inertiavalue = params[n];
	if (inertiavalue != 0xffff) {
		inertia = inertiavalue;
	}

	if (!inertia) {
		for (int i = 0; i < n; ++i) {
			const unsigned short &value = params[i];

			if (value != 0xffff) {
				paramInertia[i].curParam = value;
				paramInertia[i].newParam = value;
				paramInertia[i].inertia = 0;

				set_parameter(i, float(value) / eMaxParam);
			}
		}
	} else {
		for (int i = 0; i < n; ++i) {
			const unsigned short &value = params[i];

			if (value != 0xffff) {
				paramInertia[i].newParam = value;

				paramInertia[i].inertia = float( ( paramInertia[i].newParam - paramInertia[i].curParam ) /  ( ( float(inertia) / eInertiaTick ) * _master_info->samples_per_tick ) );
			}
		}
	}
}

bool CLadspa::process_stereo(float **pin, float **pout, int numsamples, int mode) {
	if (!ladspaInitialized) return false;

	if (inertia) {
		const int n = info->global_parameters.size() - 1; //ladspa->getNumParams();

		if (n > 0) {
			for (int i = 0; i < n; ++i) {
				SParam &prm = paramInertia[i];

				if (prm.curParam > prm.newParam) {
					prm.curParam += ( prm.inertia * numsamples );

					if (prm.curParam<prm.newParam) {
						prm.curParam = float(prm.newParam);
					}

					set_parameter(i, prm.curParam / eMaxParam );
				}
				else if (prm.curParam < prm.newParam) {
					prm.curParam += ( prm.inertia * numsamples );

					if (prm.curParam > prm.newParam) {
						prm.curParam = float(prm.newParam);
					}					

					set_parameter(i , prm.curParam / eMaxParam );
				}					 			
			}
		}
	}	

	for (int i = 0; i < info->inputs; i++) {
		if (pin[i])
			memcpy(inbuffers[i].get(), pin[i], numsamples * sizeof(float));
		else
			memset(inbuffers[i].get(), 0, numsamples * sizeof(float));
	}

	info->lad_descriptor->run(handle, numsamples);

	bool result = false;
	for (int i = 0; i < info->outputs; i++) {
		if (pout[i]) {
			if (zzub::buffer_has_signals(outbuffers[i].get(), numsamples)) {
				memcpy(pout[i], outbuffers[i].get(), numsamples * sizeof(float));
				result = true;
			} else {
				pout[i] = 0;
			}
		}
	}
	return result;
}

void CLadspa::stop(void) {
}

void CLadspa::command(int index) {
	using std::endl;

	if (index == 0) {
		std::stringstream buffer;

		buffer << "Copyright (C) 2007 Frank Potulski (polac@gmx.de)" << endl;
		buffer << "Copyright (C) 2012 Armstrong Developers" << endl << endl;
		buffer << "Now wrapping..." << endl << endl;

		if (ladspaInitialized)  {
			buffer << "Name:\t\t" << info->name << endl;
			buffer << "ShortName:\t" << info->short_name << endl;
			buffer << "Author:\t\t" << info->author << endl;

			if (info->lad_descriptor->Copyright) {
				buffer << "Copyright:\t";
				buffer << info->lad_descriptor->Copyright << endl;
			}

			buffer << "Path:\t\t" << info->pluginfile << endl;
			buffer << "DescriptorInUse:\t" << info->lad_index << endl;
			//buffer += "/";
			//::sprintf(digits,"%d",info->lad_num_desc);
			//buffer += digits;
			buffer << endl << endl;
			buffer << "UniqueID:\t" << std::hex << info->lad_descriptor->UniqueID << endl;
			//buffer << "NumParams:\t" << (info->global_parameters.size() - 1) << endl;
			/*::sprintf(digits,"%.8X (%d)",ladspa->getUniqueID(),ladspa->getUniqueID());
			buffer += digits;
			buffer += "\n";
			buffer += "\n";
			buffer += "NumParams:\t";
			::sprintf(digits,"%d",ladspa->getNumParams());
			buffer += digits;
			buffer += "\n";		
			buffer += "NumInputs:\t";
			::sprintf(digits,"%d",ladspa->getNumInputs());
			buffer += digits;
			buffer += "\n";
			buffer += "NumOutputs:\t";
			::sprintf(digits,"%d",ladspa->getNumOutputs());
			buffer += digits;
			buffer += "\n";*/
		} else {
			buffer << "nothing";
		}

		::MessageBox(::GetForegroundWindow(), buffer.str().c_str(), "Polac Ladspa Loader v0.01a",MB_OK);
	}
}

const char *CLadspa::describe_value(int param,int value) {
	static char paramDescribe[64];

	paramDescribe[0] = 0;

	if (ladspaInitialized) {
		if (param >= 0) {
			const int n = info->global_parameters.size() - 1; //ladspa->getNumParams();
			
			if (param < n) {
				::sprintf( paramDescribe , "%.2f" , get_parameter(param, float(value)/eMaxParam));
			} else if (param==n) {
				::sprintf( paramDescribe , "%.3f Ticks" , float(value)/eInertiaTick );
			}
		}			
	} else {
		::strcpy(paramDescribe, "?");
	}

	return paramDescribe;
}
