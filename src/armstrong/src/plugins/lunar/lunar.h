// zzub/lua bridge
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

#if defined(__cplusplus)
extern "C" {
#endif

zzub::plugincollection *lunar_get_plugincollection();

// registers a plugin directory to lunar
// the directory must contain a manifest.xml
// which describes the plugin appropriately.
void lunar_register_plugin(const char *path);
	
// sets the plugins local storage directory,
// which is required for loading plugin content
// from modules.
void lunar_set_local_storage_dir(const char *path);

#if defined(__cplusplus)
} // extern "C"
#endif
