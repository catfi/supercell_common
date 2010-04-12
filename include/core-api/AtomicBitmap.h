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
 * @date Mar 17, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_ATOMICBITMAP_H_
#define ZILLIANS_ATOMICBITMAP_H_

#include "core-api/Common.h"

//#define ZILLIANS_ATOMICBITMAP_WINDOWS
//#define ZILLIANS_ATOMICBITMAP_X86
#define ZILLIANS_ATOMICBITMAP_MUTEX

#if defined ZILLIANS_ATOMICBITMAP_MUTEX
#include <tbb/mutex.h>
#include <tbb/spin_mutex.h>
#endif

namespace zillians {

class AtomicBitmap
{
public:
	typedef uint64 BitmapType;
	AtomicBitmap(BitmapType value = 0) : mValue(value)
	{ }

	~AtomicBitmap()
	{ }

public:
	inline bool btsr(int indexToSet, int indexToReset)
	{
		BitmapType valueOld;
		BitmapType valueNew;
#if defined ZILLIANS_PLATFORM_WINDOWS
		while(true)
		{
			valueOld = mValue;
			valueNew = (valueOld | BitmapType(1) << indexToSet) & ~(BitmapType(1) << indexToReset);
			if(InterlockedCompareExchange((volatile LONG*)&mValue, valueNew, valueOld) == (LONG)valueOld)
				return (bool) (valueOld & (BitmapType(1) << indexToReset));
		}
#elif defined ZILLIANS_ATOMICBITMAP_X86
		BitmapType dummy;
		__asm__ volatile (
				"mov %0, %1\n\t"
				"1:\n\t"
				"mov %1, %2\n\t"
				"bts %3, %2\n\t"
				"btr %4, %2\n\t"
				"lock cmpxchg %2, %0\n\t"
				"jnz 1b\n\t"
				: "+m" (mValue), "=&a" (valueOld), "=&r" (dummy)
				: "r" (BitmapType(indexToSet)), "r" (BitmapType(indexToReset))
				: "cc"
				);
		return (bool) (valueOld & (BitmapType(1) << indexToReset));
#elif defined ZILLIANS_ATOMICBITMAP_MUTEX
		mBarrier.lock();
		valueOld = mValue;
		valueNew = (valueOld | BitmapType(1) << indexToSet) & ~(BitmapType(1) << indexToReset);
		mBarrier.unlock();
		return (bool) (valueOld & (BitmapType(1) << indexToReset));
#endif
	}

	inline BitmapType xchg(BitmapType valueNew)
	{
		BitmapType valueOld;
#if defined ZILLIANS_PLATFORM_WINDOWS
		valueOld = InterlockedExchange((volatile LONG*)&mValue, valueNew);
#elif defined ZILLIANS_ATOMICBITMAP_X86
		valueOld = valueNew;
		__asm__ volatile (
				"lock; xchg %0, %1"
				: "=r" (valudOld)
				: "m" (mValue), "0" (valueOld)
				: "memory"
				);
#elif defined ZILLIANS_ATOMICBITMAP_MUTEX
		mBarrier.lock();
		valueOld = mValue;
		mValue = valueNew;
		mBarrier.unlock();
#endif
		return valueOld;
	}

	// itez => "if-zero-then-else" atomic operation
	// if the value is zero, then it's substituted by valueThen, otherwise by valueElse
	// and return the original value
	inline BitmapType izte(BitmapType valueThen, BitmapType valueElse)
	{
		BitmapType valueOld;
		BitmapType valueNew;
#if defined ZILLIANS_PLATFORM_WINDOWS
		while(true)
		{
			valueOld = mValue;
			valueNew = (valueOld == 0) ? valueThen : valueElse;
			if(InterlockedCompareExchange((volatile LONG*)&mValue, valueNew, valueOld) == (LONG)valueOld)
				return valueOld;
		}
#elif defined ZILLIANS_ATOMICBITMAP_X86
		BitmapType dummy;
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
				: "+m" (mValue), "=&a" (valueOld), "=&r" (dummy)
				: "r" (valueThen), "r" (valueElse)
				: "cc"
				);
#elif defined ZILLIANS_ATOMICBITMAP_MUTEX
		mBarrier.lock();
		valueOld = mValue;
		mValue = (valueOld == 0) ? valueThen : valueElse;
		mBarrier.unlock();
#endif
		return valueOld;
	}

private:
	volatile BitmapType mValue;

#if defined ZILLIANS_ATOMICBITMAP_MUTEX
	tbb::spin_mutex mBarrier;
#endif

	// forbid object copy constructor and copy operator
private:
	AtomicBitmap(const AtomicBitmap&);
	void operator = (const AtomicBitmap&);

};

}

#endif /* ZILLIANS_ATOMICBITMAP_H_ */
