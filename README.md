# DKMalloc
Memory pool allocator. C / C++
Optimized for quick alloc, realloc, free.

You can use DKMalloc for your project.
Build DKMalloc as a shared library / DLL for C projects that can not use C\++.
(essential functions are defined as C-linkage)


This is simplified 'Memory Allocator' part of DKGL.
 Full version of DKGL: https://github.com/DKGL/DKGL


## Performance Test
1. Choose which allocator to test.
  - To test system default malloc with DKMalloc
    1. Set 1 to TEST_SYSTEM_DEFAULT_MALLOC at **main.cpp** line **14**
```c_cpp
#define TEST_SYSTEM_DEFAULT_MALLOC 1
```
  - To test **Intel TBB*** with DKMalloc
    1. You have to install Intel TBB and set up header path
    1. Set 1 to TEST_TBB_SCALABLE_ALLOCATOR at **main.cpp** line **15**
```c_cpp
#define TEST_TBB_SCALABLE_ALLOCATOR 1
```
1. Decide testing for small blocks or large blocks at **main.cpp** line **17**
```
#define TEST_LARGE_BLOCKS 0	 // set 1 to test large-blocks
```
  - Set 1 to test large blocks
1. Adjust allocation unit count at **main.cpp** line **19** or **20**
  - for testing small blocks (line 19)
```c_cpp
#define NUM_ALLOCATIONS_SMALL_BLOCKS 20000000	// num allocs for small blocks
```
  - for testing large blocks (line 20)
```c_cpp
#define NUM_ALLOCATIONS_LARGE_BLOCKS 1000000	// num allocs for large blocks
```
1. Build and Run with **RELEASE MODE**

## Platform
- Windows (x86, x64)
- Mac OS X
- iOS
- Android, Linux (NDK required)

## Build environment
- Visual Studio 2015 or later
- Xcode 5 or later
- C++11 compiler

## License
BSD 3
