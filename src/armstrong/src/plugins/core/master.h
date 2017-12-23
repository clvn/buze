/*
Copyright (C) 2003-2008 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

struct master_values {
	unsigned short volume;
};

struct master_plugin : zzub::plugin {
	master_values* gvals;
	master_values dummy;

	int master_volume;

	master_plugin();
	virtual void init(zzub::archive*);
	virtual void destroy();
	void process_events();
	bool process_stereo(float** pin, float** pout, int numsamples, int mode);

	void get_midi_output_names(zzub::outstream* pout);
	void process_midi_events(zzub::midi_message* pin, int nummessages);
};

struct master_plugin_info : zzub::info {
	master_plugin_info();	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive*) const;
};
