// lunar fx standard library
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

#define LUNAR_NO_FX_STDLIB_DEFINES
#include "include/lunar/fx.h"

namespace zzub {
	struct host;
};

struct _lunar_voice {
	zzub::host *cb;
	
	int instr;
	float note;
	float speed;
	int offset;
	float frac;
	const zzub::wave_info *wi;
	const zzub::wave_level *wl;
};

struct _lunar_host {
	zzub::host *cb;
	std::vector<lunar_voice_t> voices;
};

