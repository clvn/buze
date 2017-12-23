/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>

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

#if defined(_WIN32)

#define NOMINMAX
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

namespace zzub {

#if defined(_WIN32)

struct timer {

	signed __int64 perf_start;
	signed __int64 perf_freq;
	double scale;

	timer() {
		QueryPerformanceFrequency((LARGE_INTEGER*)&perf_freq);
		scale = static_cast<double>(perf_freq);
	}
	void start() {
		QueryPerformanceCounter((LARGE_INTEGER*)&perf_start);
	}

	double frame() {
		__int64 perf_frame;
		QueryPerformanceCounter((LARGE_INTEGER*)&perf_frame);
		return double(perf_frame - perf_start) / scale;
	}

};

#else 

struct timer {
	timeval vStart;

	timer() {
		gettimeofday(&vStart, 0);
	}
	void start() {
		gettimeofday(&vStart, 0);
	}

	double frame() {
		timeval vEnd;
		gettimeofday(&vEnd, 0);
		double dstart = vStart.tv_sec + ((double)vStart.tv_usec / 1000000.0);
		double dend = vEnd.tv_sec + ((double)vEnd.tv_usec / 1000000.0);
		return dend - dstart;
	}

};

#endif

} // namespace zzub
