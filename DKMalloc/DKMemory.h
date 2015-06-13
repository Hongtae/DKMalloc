/*******************************************************************************
 File: DKMemory.h
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
	// heap memory
	DKLIB_API void* DKMemoryHeapAlloc(size_t);
	DKLIB_API void* DKMemoryHeapRealloc(void*, size_t);
	DKLIB_API void  DKMemoryHeapFree(void*);

	// virtual-address, can commit, decommit.
	// data will be erased when decommit.
	DKLIB_API void* DKMemoryVirtualAlloc(size_t);
	DKLIB_API void* DKMemoryVirtualRealloc(void*, size_t);
	DKLIB_API void  DKMemoryVirtualFree(void*);
	DKLIB_API size_t  DKMemoryVirtualSize(void*);

	// system-paing functions.
	DKLIB_API size_t DKMemoryPageSize(void); // default allocation size
	DKLIB_API void* DKMemoryPageReserve(void*, size_t);
	DKLIB_API void DKMemoryPageRelease(void*);
	DKLIB_API void DKMemoryPageCommit(void*, size_t);
	DKLIB_API void DKMemoryPageDecommit(void*, size_t);

	// Pre-allocated pool.
	DKLIB_API void* DKMemoryPoolAlloc(size_t);
	DKLIB_API void* DKMemoryPoolRealloc(void*, size_t);
	DKLIB_API void  DKMemoryPoolFree(void*);
	// Optional pool management functions.
	DKLIB_API size_t DKMemoryPoolPurge(void);
	DKLIB_API size_t DKMemoryPoolSize(void);


	enum DKMemoryLocation
	{
		DKMemoryLocationCustom = 0,
		DKMemoryLocationHeap,
		DKMemoryLocationVirtual,
		DKMemoryLocationPool,
	};

	// simple allocator types for template classes.
	// you can provide your own allocator.
	struct DKMemoryHeapAllocator
	{
		enum { Location = DKMemoryLocationHeap };
		static void* Alloc(size_t s)			{ return DKMemoryHeapAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryHeapRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryHeapFree(p); }
	};
	struct DKMemoryVirtualAllocator
	{
		enum { Location = DKMemoryLocationVirtual };
		static void* Alloc(size_t s)			{ return DKMemoryVirtualAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryVirtualRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryVirtualFree(p); }
	};
	struct DKMemoryPoolAllocator
	{
		enum { Location = DKMemoryLocationPool };
		static void* Alloc(size_t s)			{ return DKMemoryPoolAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryPoolRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryPoolFree(p); }
	};

	using DKMemoryDefaultAllocator = DKMemoryHeapAllocator;
}
