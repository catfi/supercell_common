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
 * @date Jun 9, 2010 zac - Initial version created.
 */

#ifndef ZILLIANS_DISPATCHER_DISPATCHERTHREADSIGNALER_H_
#define ZILLIANS_DISPATCHER_DISPATCHERTHREADSIGNALER_H_

#include "core/Prerequisite.h"
#include "core/Semaphore.h"
#include "core/Atomic.h"

namespace zillians { namespace threading {

class DispatcherThreadSignaler
{
public:
	DispatcherThreadSignaler() : mWaitSignal(sizeof (uint64) * 8 - 1)
	{ mBitmap = uint64(1) << mWaitSignal; }

	~DispatcherThreadSignaler()
	{ }

public:
	void signal(uint32 signal)
	{
		if(atomic::bitmap_btsr(mBitmap, signal, mWaitSignal))
			mSemaphore.post();
	}

	uint64 poll(uint32 id)
	{
		// TODO use ypollset style izte atomic op
		uint64 result = atomic::bitmap_xchg(mBitmap, 0);
		//result = mBitmap;

		if(!result)
		{
			mSemaphore.wait();
		}

		return result;
	}

	uint64 check()
	{ return atomic::bitmap_xchg(mBitmap, 0); }

	uint64 bitOr(uint64 bitmap)
	{ return atomic::bitmap_or(mBitmap, bitmap); }

	void bitReset(uint32 bit)
	{
		atomic::bitmap_btsr(mBitmap, bit, bit);
	}

private:
	Semaphore mSemaphore;
	uint64 mBitmap;
	const int mWaitSignal;
};

} }

#endif /* ZILLIANS_DISPATCHER_DISPATCHERTHREADSIGNALER_H_ */
