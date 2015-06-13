/*******************************************************************************
 File: DKAllocatorChain.cpp
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

#include <new>
#include "DKAllocatorChain.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	namespace Private
	{
		void CreateAllocationTable(void);
		void DestroyAllocationTable(void);

		using ScopedSpinLock = DKCriticalSection<DKSpinLock>;
		struct Chain
		{
			using RefCount = unsigned int;

			DKAllocatorChain* first;
			DKSpinLock lock;
			RefCount refCount;
			static Chain* instance;

			Chain(void)
			{
				CreateAllocationTable();
				lock.Lock();
				first = NULL;
				instance = this;
				refCount = 0;
				lock.Unlock();
			}
			~Chain(void)
			{
				DestroyAllocationTable();
				while (true)
				{
					lock.Lock();
					DKAllocatorChain* p = first;
					lock.Unlock();
					if (p)
						delete p;
					else
						break;
				}

				lock.Lock();
				instance = NULL;
				first = NULL;
				lock.Unlock();
			}
			static Chain* Instance(void)
			{
				static Chain* p = new Chain();
				return p->instance;
			}
			RefCount IncrementRef(void)
			{
				ScopedSpinLock guard(lock);
				this->refCount++;
				return this->refCount;
			}
			RefCount DecrementRef(void)
			{
				ScopedSpinLock guard(lock);
				this->refCount--;
				return this->refCount;
			}
		};
		Chain* Chain::instance;

		// default Chain-Holder
		static DKAllocatorChain::StaticInitializer init;
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;


DKAllocatorChain::DKAllocatorChain(void)
: next(NULL)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	if (c->first)
	{
		DKAllocatorChain* last = c->first;
		while (last->next)
			last = last->next;
		last->next = this;
	}
	else
	{
		c->first = this;
	}
}

DKAllocatorChain::~DKAllocatorChain(void)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	if (c->first == this)
		c->first = this->next;
	else
	{
		for (DKAllocatorChain* chain = c->first; chain; chain = chain->next)
		{
			if (chain->next == this)
			{
				chain->next = this->next;
				break;
			}
		}
	}
}

size_t DKAllocatorChain::Cleanup(void)
{
	size_t purged = 0;
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	for (DKAllocatorChain* chain = c->first; chain; chain = chain->next)
	{
		purged += chain->Purge();
	}
	return purged;
}

DKAllocatorChain* DKAllocatorChain::FirstAllocator(void)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	return c->first;
}

DKAllocatorChain* DKAllocatorChain::NextAllocator(void)
{
	return next;
}

DKAllocatorChain::StaticInitializer::StaticInitializer(void)
{
	Chain* c = Chain::Instance();
	DKASSERT_MEM_DEBUG( c != NULL );
	Chain::RefCount ref = c->IncrementRef();
	DKASSERT_MEM_DEBUG( ref > 0);
	if (ref == 0)
	{
		DKLog("ERROR: DKAllocatorChain invalid reference.");
	}
}

DKAllocatorChain::StaticInitializer::~StaticInitializer(void)
{
	Chain* c = Chain::Instance();
	DKASSERT_MEM_DEBUG( c != NULL );
	Chain::RefCount ref = c->DecrementRef();
	DKASSERT_MEM_DEBUG( ref >= 0);
	if (ref == 0)
	{
		delete c;
	}
}

void* DKAllocatorChain::operator new (size_t s)
{
	return ::malloc(s);
}

void DKAllocatorChain::operator delete (void* p) NOEXCEPT
{
	::free(p);
}

