/*******************************************************************************
 File: DKSpinLock.h
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

#pragma once
#include "DKDef.h"

namespace DKFoundation
{
	class DKSpinLock
	{
		enum SpinLockState
		{
			SpinLockStateFree = 0,
			SpinLockStateLocked = 1,
		};

	public:
		DKSpinLock(void): state(SpinLockStateFree) {}
		~DKSpinLock(void) {}
		void Lock(void) const
		{
#ifdef _WIN32
			while (!TryLock())
			{
				if (SwitchToThread() == 0)
					::Sleep(0);
			}
#else
			timespec req = {0, 0};
			while (!TryLock())
			{
				if (sched_yield() != 0)
				{
					while (nanosleep(&req, &req) != 0)
					{
						if (errno != EINTR)
							break;
					}
				}
			}
#endif
		}
		bool TryLock(void) const
		{
#ifdef _WIN32
			return ::InterlockedCompareExchange((LONG*)&state, SpinLockStateLocked, SpinLockStateFree) == SpinLockStateFree;
#elif defined(__APPLE__) && defined(__MACH__)
			return ::OSAtomicCompareAndSwap32(SpinLockStateFree, SpinLockStateLocked, &state);
#else
			return __sync_bool_compare_and_swap(&state, SpinLockStateFree, SpinLockStateLocked);
#endif
		}
		void Unlock(void) const
		{
			state = SpinLockStateFree;
		}

	private:
		DKSpinLock(const DKSpinLock&);
		DKSpinLock& operator = (const DKSpinLock&);
		mutable volatile int state;
	};
}
