
#ifdef _WIN32
#define _CRT_RAND_S
#include "targetver.h"
#include <tchar.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string>

#define TEST_SYSTEM_DEFAULT_MALLOC 0  // set 1 to test system default 'malloc', 'free'
#define TEST_TBB_SCALABLE_ALLOCATOR 0 // set 1 to test TBB-scalable allocator'

// before testing TBB, you need to download and install TBB
// TBB official web page: https://www.threadingbuildingblocks.org

#if TEST_TBB_SCALABLE_ALLOCATOR
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#endif

#include "DKMalloc/DKMalloc.h"
#include "DKTimer.h"


#ifndef TEST_LARGE_BLOCKS
#define TEST_LARGE_BLOCKS 0	 // set 1 to test large-blocks
#endif


#ifdef _WIN32
unsigned int DKRandom(void)
{
	unsigned int value;
	if (rand_s(&value) == 0)
	{
		return value;
	}
	// rand_s error??
	static struct _Rand_init
	{
		_Rand_init(void)
		{
			DWORD tick = ::GetTickCount();
			srand((unsigned int)tick);
		}
	} init;

	unsigned int h = rand();
	unsigned int l = rand();
	return ((h << 16) & 0xffff0000) | (l & 0x0000ffff);
}

void DKSleep(double d)
{
	if (d < 0.0)
		d = 0.0;
	DWORD dwTime = static_cast<DWORD>(d * 1000.0f);
	::Sleep(dwTime);
}
#else
unsigned int DKRandom(void)
{
	return arc4random();
}

void DKSleep(double d)
{
	if (d < 0.0)
		d = 0.0;

	long sec = static_cast<long>(d);
	long usec = (d - sec) * 1000000;
	struct timespec req = {sec, usec * 1000};
	while ( nanosleep(&req, &req) != 0 )
	{
		// internal error! (except for signal, intrrupt)
		if (errno != EINTR)
			break;
	}
}
#endif


std::string FormatNumber(size_t num)
{
	if (num == 0)
		return "0";
	static char buff[1024];
	buff[1023] = NULL;
	int pos = 1022;
	int grp = 0;
	while (num > 0)
	{
		if (grp > 0 && grp % 3 == 0)
			buff[pos--] = ',';
		char n = num % 10;
		num = num / 10;
		buff[pos--] = '0' + n;
		grp++;
	}
	return &buff[pos+1];
};

template <typename T, size_t Size> size_t NumArrayItems(T(&)[Size])
{
	return Size;
}

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, const char * argv[])
#endif
{
#ifndef NDEBUG
	printf("Run-test debug-mode.\n");
#else
	printf("Run-test release-mode.\n");
#endif

#if TEST_TBB_SCALABLE_ALLOCATOR
	scalable_allocation_mode(TBBMALLOC_USE_HUGE_PAGES, 1);
#endif

	DKMemPurge();

	enum { Alignment = 16 };

#if TEST_LARGE_BLOCKS
	const size_t allocSizeArray[] =
	{
		128, 256, 1024, 2048, 4096, 8192, 131072
	};
	const size_t numAllocs = 1000000;
#else
	const size_t allocSizeArray[] =
	{
        8, 16, 24, 32, 48, 64, 128, 192, 256, 512, 1024, 2048
	};
    const size_t numAllocs = 20000000;
#endif

	struct AllocInfo
	{
		size_t size;
		void* ptr;
	};
	AllocInfo* allocArray = new AllocInfo[numAllocs];

	size_t totalSize = 0;
	printf("Generating random size... (%s units, TEST_LARGE_BLOCKS: %d)\n",
		FormatNumber(numAllocs).c_str(), TEST_LARGE_BLOCKS);
	size_t smallest = (size_t)-1;
	size_t largest = 0;
	for (size_t i = 0; i < numAllocs; ++i)
	{
		auto n = DKRandom() % NumArrayItems(allocSizeArray);
		allocArray[i].size = (DKRandom() % allocSizeArray[n]) + 1;
		allocArray[i].ptr = NULL;
		totalSize += allocArray[i].size;
		if (smallest > allocArray[i].size)
			smallest = allocArray[i].size;
		if (largest < allocArray[i].size)
			largest = allocArray[i].size;
	}
	printf("  -> %s units generated. (%s bytes, %s ~ %s)\n",
		   FormatNumber(numAllocs).c_str(),
		   FormatNumber(totalSize).c_str(),
		   FormatNumber(smallest).c_str(),
		   FormatNumber(largest).c_str());

	struct AllocatorFunc
	{
		const char* desc;
		void* (*alloc)(size_t);
		void* (*realloc)(void*, size_t);
		void (*free)(void*);
		bool checkAlignment;
		double allocTime;
		double deallocTime;
	};

	AllocatorFunc allocators[] = {
#if TEST_SYSTEM_DEFAULT_MALLOC
		{ "malloc", malloc, realloc, free, false },
#endif
#if TEST_TBB_SCALABLE_ALLOCATOR
		{ "TBB", scalable_malloc, scalable_realloc, scalable_free, false },
#endif
		{ "DKMalloc", DKMalloc, DKRealloc, DKFree, false },
	};

	auto runAlloc = [&](AllocatorFunc& alloc)
	{
		size_t bytesAlloc = 0;
		printf("  allocating %s units...\n", FormatNumber(numAllocs).c_str());
		DKTimer timer;
		timer.Reset();
		for (size_t i = 0; i < numAllocs; ++i)
		{
			allocArray[i].ptr = alloc.alloc(allocArray[i].size);
			if (allocArray[i].ptr)
			{
				::memset(allocArray[i].ptr, 0xf0, allocArray[i].size);
				bytesAlloc += allocArray[i].size;
			}
		}
		alloc.allocTime = timer.Elapsed();
		printf("  -> time elapsed: %f (%s bytes allocated.)\n", alloc.allocTime, FormatNumber(bytesAlloc).c_str());
	};
	auto runDealloc = [&](AllocatorFunc& alloc)
	{
		printf("  deallocating: %s units...\n", FormatNumber(numAllocs).c_str());
		DKTimer timer;
		timer.Reset();
		for (size_t i = 0; i < numAllocs; ++i)
		{
			alloc.free(allocArray[i].ptr);
		}
		alloc.deallocTime = timer.Elapsed();
		printf("  -> time elapsed: %f\n", alloc.deallocTime);
	};
	auto checkAlignment = [&]()
	{
		printf("  checking %d alignment: %s units...\n", Alignment, FormatNumber(numAllocs).c_str());
		for (size_t i = 0; i < numAllocs; ++i)
		{
			if (reinterpret_cast<uintptr_t>(allocArray[i].ptr) % Alignment)
			{
				printf("Invalid alignment!!\n");
			}
		}
	};

	auto shuffle = [&]()
	{
		for (size_t i = 0; i < numAllocs; ++i)
		{
			auto k = DKRandom() % numAllocs;
			AllocInfo tmp = allocArray[i];
			allocArray[i] = allocArray[k];
			allocArray[k] = tmp;
		}
	};

	size_t numAllocators = NumArrayItems(allocators);

	for (int i = 0; i < numAllocators; ++i)
	{
		AllocatorFunc& alloc = allocators[i];
		printf("\nTesting allocator[%d]... (%s)\n", i, alloc.desc);
		alloc.allocTime = 0.0;
		alloc.deallocTime = 0.0;
		if (alloc.alloc && alloc.free)
		{
			DKSleep(0.01);
			runAlloc(alloc);
			if (alloc.checkAlignment)
				checkAlignment();
			shuffle();
			DKSleep(0.01);
			runDealloc(alloc);
		}
		else
		{
			printf("  -> passed.");
		}
	}

	delete[] allocArray;

	printf("\n---------------------------------------\n");
	for (int i = 0; i < numAllocators; ++i)
	{
		printf("    alloc: %f, free: %f - [%s]\n",
			allocators[i].allocTime, allocators[i].deallocTime, allocators[i].desc);
	}

	printf("DKMemoryPoolSize: %s bytes.\n", FormatNumber(DKMemPoolSize()).c_str());

	//system("pause");
	return 0;
}
