/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
 * For more information see http://www.zillians.com
 *
 * Zillians MMO is the library and runtime for massive multiplayer online game
 * development in utility computing model, which runs as a service for every
 * developer to build their virtual world running on our GPU-assisted machines.
 *
 * This is a close source library intended to be used solely within Zillians.com
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/**
 * @date Jun 4, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_ATOMIC_H_
#define ZILLIANS_ATOMIC_H_

#include "core/Types.h"

#if defined(WIN32)
#include <intrin.h>
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange64)

#if defined(_WIN64)
#pragma intrinsic(_InterlockedCompareExchange128)
#endif
#endif

namespace zillians { namespace atomic {

template<typename T>
inline T inc(volatile T* ptr)
{
#if defined(__GNUC__)
	return __sync_add_and_fetch(ptr, static_cast<T> (1));
#elif defined(WIN32)
	BOOST_ASSERT(sizeof(T) == 4);
	return _InterlockedIncrement(reinterpret_cast<volatile long*>(ptr));
#endif
}

template<typename T>
inline T dec(volatile T* ptr)
{
#if defined(__GNUC__)
	return __sync_sub_and_fetch(ptr, static_cast<T> (1));
#elif defined(WIN32)
	BOOST_ASSERT(sizeof(T) == 4);
	return _InterlockedDecrement(reinterpret_cast<volatile long*>(ptr));
#endif
}

template<typename T>
inline T add(volatile T* ptr, const T val)
{
#if defined(__GNUC__)
	return __sync_fetch_and_add(ptr, val);
#elif defined(WIN32)
	BOOST_ASSERT(sizeof(T) == 4);
	return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(ptr), val);
#endif
}

template<typename T>
inline T cas(volatile T* ptr, const T val, const T cmp)
{
#if defined(__GNUC__)
	return __sync_val_compare_and_swap(ptr, cmp, val);
#elif defined(WIN32)
	if(sizeof(T) == 4)
		return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(ptr), val, cmp);
	else if(sizeof(T) == 8)
		return _InterlockedCompareExchange64(reinterpret_cast<volatile int64*>(ptr), val, cmp);
#endif
}

template<typename T>
inline bool b_cas(volatile T* ptr, const T val, const T cmp)
{
#if defined(__GNUC__)
	return __sync_bool_compare_and_swap(ptr, cmp, val);
#elif defined(WIN32)
	if(sizeof(T) == 4)
		return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(ptr), val, cmp) == cmp;
	else if(sizeof(T) == 8)
		return _InterlockedCompareExchange64(reinterpret_cast<volatile int64*>(ptr), val, cmp) == cmp;
#endif
}

inline void* cas_ptr(void* volatile* pdst, void* pval, void* pcmp)
{
#if defined(__GNUC__)
	return reinterpret_cast<void*> (__sync_val_compare_and_swap(
			reinterpret_cast<volatile ptrdiff_t*> (pdst),
			reinterpret_cast<ptrdiff_t> (pcmp),
			reinterpret_cast<ptrdiff_t> (pval)));
#elif defined(WIN32)
	return reinterpret_cast<void*>(__InterlockedCompareExchange(
					reinterpret_cast<volatile long *>(pdst),
					reinterpret_cast<long>(pval),
					reinterpret_cast<long>(pcmp)));
#endif
}

inline bool b_cas_ptr(void* volatile* pdst, void* pval, void* pcmp)
{
#if defined(__GNUC__)
	return __sync_bool_compare_and_swap(
			reinterpret_cast<volatile ptrdiff_t*> (pdst),
			reinterpret_cast<ptrdiff_t> (pcmp),
			reinterpret_cast<ptrdiff_t> (pval));
#elif defined(WIN32)
	return __InterlockedCompareExchange(
			reinterpret_cast<volatile long *>(pdst),
			reinterpret_cast<long>(pval),
			reinterpret_cast<long>(pcmp)) == reinterpret_cast<long>(pcmp);
#endif
}

inline bool bitmap_btsr(uint64 bitmap, int index_to_set, int index_to_reset)
{
	uint64 bitmap_old;
#if defined(__GNUC__)
	uint64 dummy;
	__asm__ volatile (
			"mov %0, %1\n\t"
			"1:\n\t"
			"mov %1, %2\n\t"
			"bts %3, %2\n\t"
			"btr %4, %2\n\t"
			"lock cmpxchg %2, %0\n\t"
			"jnz 1b\n\t"
			: "+m" (bitmap), "=&a" (bitmap_old), "=&r" (dummy)
			: "r" (uint64(index_to_set)), "r" (uint64(index_to_reset))
			: "cc"
			);
	return (bool) (bitmap_old & (uint64(1) << index_to_reset));
#elif defined(WIN32)
	while(true)
	{
		bitmap_old = bitmap;
		bitmap_new = (bitmap_old | uint64(1) << index_to_set) & ~(uint64(1) << index_to_reset);
		if(_InterlockedCompareExchange((volatile LONG*)&bitmap, bitmap_new, bitmap_old) == (LONG)bitmap_old)
			return (bool) (bitmap_old & (uint64(1) << index_to_reset));
	}
#endif
}

inline uint64 bitmap_xchg(uint64 bitmap, uint64 bitmap_new)
{
	uint64 bitmap_old;
#if defined(__GNUC__)
	bitmap_old = bitmap_new;
	__asm__ volatile (
			"lock; xchg %0, %1"
			: "=r" (bitmap_old)
			: "m" (bitmap), "0" (bitmap_old)
			: "memory"
			);
#elif defined(WIN32)
	bitmap_old = _InterlockedExchange((volatile LONG*)&bitmap, bitmap_new);
#endif
	return bitmap_old;
}

// itez => "if-zero-then-else" atomic operation
// if the value is zero, then it's substituted by valueThen, otherwise by valueElse
// and return the original value
inline uint64 bitmap_izte(uint64 bitmap, uint64 bitmap_then, uint64 bitmap_else)
{
	uint64 bitmap_old;
#if defined(__GNUC__)
	uint64 dummy;
	__asm__ volatile (
			"mov %0, %1\n\t"
			"1:\n\t"
			"mov %3, %2\n\t"
			"test %1, %1\n\t"
			"jz 2f\n\t"
			"mov %4, %2\n\t"
			"2:\n\t"
			"lock cmpxchg %2, %0\n\t"
			"jnz 1b\n\t"
			: "+m" (bitmap), "=&a" (bitmap_old), "=&r" (dummy)
			: "r" (bitmap_then), "r" (bitmap_else)
			: "cc"
			);
#elif defined(WIN32)
	while(true)
	{
		bitmap_old = bitmap;
		bitmap_new = (bitmap_old == 0) ? bitmap_then : bitmap_else;
		if(_InterlockedCompareExchange((volatile LONG*)&bitmap, bitmap_new, bitmap_old) == (LONG)bitmap_old)
			return bitmap_old;
	}
#endif
	return bitmap_old;
}

} }

#endif /* ZILLIANS_ATOMIC_H_ */
