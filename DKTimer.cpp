/*******************************************************************************
 File: DKTimer.cpp
 Author: Hongtae Kim (tiff2766@gmail.com)

 Copyright (c) 2015 Hongtae Kim. All rights reserved.

 NOTE: This is simplified 'Memory Allocator' part of DKLib.
  Full version of DKLib: http://github.com/tiff2766/DKLib

 License: BSD-3
*******************************************************************************/
/*******************************************************************************

 Copyright (c) 2015, Hongtae Kim.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of DKMalloc nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#include "DKTimer.h"

#include <time.h>
#include <math.h>

#ifdef _WIN32
	#include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
	#include <mach/mach.h>
	#include <mach/mach_time.h>
#elif defined(__linux__)
	#include <time.h>
#else
	#include <sys/time.h>
	#warning High-Resolution Timer Unsupported. (Unknown OS)
#endif


DKTimer::DKTimer(void)
	: timeStamp(0)
{
	Reset();
}

DKTimer::~DKTimer(void)
{
}

double DKTimer::Reset(void)
{
	static double freq = static_cast<double>(SystemTickFrequency());
	Tick t = timeStamp;
	timeStamp = SystemTick();
	return static_cast<double>(timeStamp - t) / freq;
}

double DKTimer::Elapsed(void) const
{
	static double freq = static_cast<double>(SystemTickFrequency());
	return static_cast<double>(SystemTick() - timeStamp) / freq;
}

DKTimer::Tick DKTimer::SystemTick(void)
{
#ifdef _WIN32
	static LARGE_INTEGER frequency;
	static BOOL counterEnabled = ::QueryPerformanceFrequency(&frequency);
	if (counterEnabled)
	{
		LARGE_INTEGER count;
		::QueryPerformanceCounter(&count);
		return count.QuadPart;
	}
	return ::GetTickCount();
#elif defined(__APPLE__) && defined(__MACH__)
	return mach_absolute_time();
#elif defined(__linux__)
    struct timespec ts;
    ts.tv_sec = 0;
	ts.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC_HR, &ts);
	return static_cast<Tick>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
#else
	timeval tm;
	gettimeofday(&tm, NULL);
	return static_cast<Tick>(tm.tv_sec) * 1000000ULL + tm.tv_usec;
#endif
}

DKTimer::Tick DKTimer::SystemTickFrequency(void)
{
#ifdef _WIN32
	static LARGE_INTEGER frequency;
	static BOOL counterEnabled = ::QueryPerformanceFrequency(&frequency);
	if (counterEnabled)
	{
		return frequency.QuadPart;
	}
	return 1000ULL;
#elif defined(__APPLE__) && defined(__MACH__)
	static struct MachTimeBase
	{
		MachTimeBase(void)
		{
			mach_timebase_info_data_t base;
			mach_timebase_info(&base);
			frequency = 1000000000ULL * base.denom / base.numer;
		}
		uint64_t frequency;
	} machTime;
	return machTime.frequency;
#elif defined(__linux__)
	return 1000000000ULL;
#else
	return 1000000ULL;
#endif
}
