// zzub Types
// Copyright (C) 2006 Leonard Ritter (contact@leonard-ritter.com)
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

#if !defined(__ZZUBTYPES_H)
#define __ZZUBTYPES_H

#if defined(__GNUC__)
#ifdef cdecl
#undef cdecl
#endif
#ifdef stdcall
#undef stdcall
#endif
#	define ZZUB_CALLING_CONVENTION __attribute__((cdecl))
#else
#	define ZZUB_CALLING_CONVENTION __cdecl
#endif
#define ZZUB_EXTERN_C extern "C"

#endif // __ZZUBTYPES_H
