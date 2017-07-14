/*******************************************************************************
 File: DKDef.h
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


#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <sys/select.h>
#include <sched.h>		// to using sched_yield() in DKThread::Yield()
#include <errno.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <libkern/OSAtomic.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(DEBUG) || defined(_DEBUG)
#define DKGL_DEBUG_ENABLED 1
#include <stdexcept>
#define DKERROR_THROW_DEBUG(desc)			throw std::runtime_error(desc)
#define DKASSERT_DESC_DEBUG(expr, desc)		{if (!(expr)) throw std::runtime_error(desc);}
#define DKASSERT_DEBUG(expr)				{if (!(expr)) throw std::runtime_error("error");}
#define DKASSERT_STD_DESC(expr, desc)		{if (!(expr)) throw std::runtime_error(desc);}
#define DKASSERT_STD(expr)					{if (!(expr)) throw std::runtime_error("");}
#define DKASSERT_STD_DESC_DEBUG(expr, desc)	DKASSERT_STD_DESC(expr, desc)
#define DKASSERT_STD_DEBUG(expr)			DKASSERT_STD(expr)
#else
#define DKERROR_THROW_DEBUG(desc)			(void)0
#define DKASSERT_DESC_DEBUG(expr, desc)		(void)0
#define DKASSERT_DEBUG(expr)				(void)0
#define DKASSERT_STD_DESC(expr, desc)		(void)0
#define DKASSERT_STD(expr)					(void)0
#define DKASSERT_STD_DESC_DEBUG(expr, desc)	(void)0
#define DKASSERT_STD_DEBUG(expr)			(void)0
#endif

#ifndef DKGL_MEMORY_DEBUG
#ifdef DKGL_DEBUG_ENABLED
#define DKGL_MEMORY_DEBUG 1
#else
#define DKGL_MEMORY_DEBUG 0
#endif
#endif /* DKGL_MEMORY_DEBUG */

#if DKGL_MEMORY_DEBUG
#define DKASSERT_MEM_DESC(expr, desc)		{if (!(expr)) throw std::runtime_error(desc);}
#define DKASSERT_MEM(expr)					{if (!(expr)) throw std::runtime_error("");}
#define DKASSERT_MEM_DESC_DEBUG(expr, desc)	DKASSERT_STD_DESC(expr, desc)
#define DKASSERT_MEM_DEBUG(expr)			DKASSERT_STD(expr)
#else
#define DKASSERT_MEM_DESC(expr, desc)		(void)0
#define DKASSERT_MEM(expr)					(void)0
#define DKASSERT_MEM_DESC_DEBUG(expr, desc)	(void)0
#define DKASSERT_MEM_DEBUG(expr)			(void)0
#endif


// Inline macros
#ifndef FORCEINLINE
#ifdef DKGL_DEBUG_ENABLED
#define FORCEINLINE inline
#else
#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE inline __attribute__((always_inline))
#endif
#endif
#endif
#ifndef NOINLINE
#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif
#endif

#ifndef NOEXCEPT
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif
#endif

#ifndef DKGL_API
#define DKGL_API
#endif

#define DKLog(...)	printf(__VA_ARGS__)

#ifdef __cplusplus
#include <new>
#include <algorithm>
#include <initializer_list>
#include <type_traits>
namespace DKFoundation
{
#pragma pack(push, 1)
	struct DKDummyLock
	{
		FORCEINLINE void Lock(void) const {}
		FORCEINLINE bool TryLock(void) const { return true; }
		FORCEINLINE void Unlock(void) const {}
	};

	template <typename T> class DKCriticalSection // scoped lock.
	{
	public:
		FORCEINLINE DKCriticalSection(const T& lockObject)
		: lock(lockObject)
		{
			lock.Lock();
		}
		FORCEINLINE ~DKCriticalSection(void)
		{
			lock.Unlock();
		}
	private:
		DKCriticalSection(const DKCriticalSection&);
		DKCriticalSection& operator = (const DKCriticalSection&);
		const T& lock;
	};
#pragma pack(pop)

	template <typename T> using _UnRef = typename std::remove_reference<T>::type;
	template <typename T> using _UnCV = typename std::remove_cv<T>::type;
	template <typename T> using _UnRefCV = _UnCV<_UnRef<T>>;

	// Min
	template <typename T> FORCEINLINE auto Min(T&& lhs, T&& rhs)->T&&
	{
		return std::forward<T>((lhs < rhs) ? lhs : rhs);
	}
	template <typename T, typename U> FORCEINLINE auto Min(T&& lhs, U&& rhs)->_UnRefCV<T>
	{
		return static_cast<_UnRef<T>>((lhs < rhs) ? lhs : rhs);
	}
	template <typename T, typename U, typename... V> FORCEINLINE auto Min(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
	{
		return Min(std::forward<T>(v1), Min(std::forward<U>(v2), std::forward<V>(rest)...));
	}

	// Max
	template <typename T> FORCEINLINE auto Max(T&& lhs, T&& rhs)->T&&
	{
		return std::forward<T>((lhs > rhs) ? lhs : rhs);
	}
	template <typename T, typename U> FORCEINLINE auto Max(T&& lhs, U&& rhs)->_UnRefCV<T>
	{
		return static_cast<_UnRef<T>>(((lhs > rhs) ? lhs : rhs));
	}
	template <typename T, typename U, typename... V> FORCEINLINE auto Max(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
	{
		return Max(std::forward<T>(v1), Max(std::forward<U>(v2), std::forward<V>(rest)...));
	}

	// Clamp
	template <typename T> FORCEINLINE auto Clamp(T&& v, T&& _min, T&& _max)->T&&
	{
		return Min(Max(std::forward<T>(v), std::forward<T>(_min)), std::forward<T>(_max));
	}
	template <typename T, typename MinT, typename MaxT> FORCEINLINE auto Clamp(T&& v, MinT&& _min, MaxT&& _max)->_UnRefCV<T>
	{
		return Min(Max(std::forward<T>(v), std::forward<MinT>(_min)), std::forward<MaxT>(_max));
	}

}
#endif
