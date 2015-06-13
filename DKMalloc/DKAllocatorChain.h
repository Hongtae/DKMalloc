/*******************************************************************************
 File: DKAllocatorChain.h
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

////////////////////////////////////////////////////////////////////////////////
// DKAllocatorChain
// an abstract class, a memory allocator chain class.
// implemented as linked-list.
// subclass will be added to chain automatically when they are instantiated.
//
// Warning:
//  If you need to use DKAllocator with static (global) object initialization,
//  You have to hold StaticInitializer instance as static storage.
//
//  The StaticInitializer will postpone destruction of internal allocators and
//  allocation pools until all StaticInitializer instances are destroyed.
//  It is necessary to internal allocator become persistent even when main()
//  function has been finished. (even after atexit() called)
//
//  StaticInitializer Usage:
//    Just declare static instance before using any allocators in global scope.
//    not necessary for function scope inside of main() routine.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKAllocatorChain
	{
	public:
		DKAllocatorChain(void);
		virtual ~DKAllocatorChain(void);

		virtual void* Alloc(size_t) = 0;
		virtual void Dealloc(void*) = 0;

		virtual size_t Purge(void) { return 0; }
		virtual void Description(void) {}

		static size_t Cleanup(void);
		static DKAllocatorChain* FirstAllocator(void);
		DKAllocatorChain* NextAllocator(void);


		// To extend static-object life cycle, a static-object which own
		// DKAllocator, it should have StaticInitializer instance with static storage.
		struct StaticInitializer
		{
			StaticInitializer(void);
			~StaticInitializer(void);
		};

		void* operator new (size_t);
		void operator delete (void*) NOEXCEPT;
	private:
		DKAllocatorChain* next;
	};
}
