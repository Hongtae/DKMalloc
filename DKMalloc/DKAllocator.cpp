/*******************************************************************************
 File: DKAllocator.cpp
 Author: Hongtae Kim (tiff2766@gmail.com)

 Copyright (c) 2015,2017 Hongtae Kim. All rights reserved.

 NOTE: This is simplified 'Memory Allocator' part of DKGL.
 Full version of DKGL: https://github.com/DKGL/DKGL

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

#include <new>
#include <stdexcept>
#include "DKAllocator.h"
#include "DKSpinLock.h"

using namespace DKFoundation;

DKAllocator::DKAllocator(void)
{
}

DKAllocator::~DKAllocator(void) noexcept(!DKGL_MEMORY_DEBUG)
{
}

DKAllocator& DKAllocator::DefaultAllocator(DKMemoryLocation loc)
{
	static Maintainer init;

	struct HeapAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryHeapAlloc(s);}
		void Dealloc(void* p)					{DKMemoryHeapFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationHeap;}
	};
	struct VMemAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryVirtualAlloc(s);}
		void Dealloc(void* p)					{DKMemoryVirtualFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationVirtual;}
	};
	struct PoolAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryPoolAlloc(s);}
		void Dealloc(void* p)					{DKMemoryPoolFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationPool;}
	};

	static bool initialized = false;
	static DKSpinLock lock;

	static HeapAllocator* hma = NULL;
	static VMemAllocator* vma = NULL;
	static PoolAllocator* pma = NULL;

	if (!initialized)
	{
		lock.Lock();
		if (!initialized)
		{
			hma = new HeapAllocator();
			vma = new VMemAllocator();
			pma = new PoolAllocator();
			initialized = true;
		}
		lock.Unlock();
	}

	switch (loc)
	{
	case DKMemoryLocationHeap:
		return *hma;
		break;
	case DKMemoryLocationVirtual:
		return *vma;
		break;
	case DKMemoryLocationPool:
		return *pma;
		break;
	default:		// custom?
		break;
	}
	return *hma;
}
