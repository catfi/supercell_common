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
 * @date Jan 28, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_THREADING_ADAPTIVEWAIT_H_
#define ZILLIANS_THREADING_ADAPTIVEWAIT_H_

#include "core/Prerequisite.h"

namespace zillians { namespace threading {

template<uint32 NumberOfSignalsBeforeEnteringWaitState, uint32 NumberOfSignalsBeforeLeavingWaitState,
		uint32 MinWait, uint32 MaxWait,
		uint32 SlowDownStep, uint32 SpeedUpStep>
struct AdaptiveWait
{
	AdaptiveWait() : mWaiting(false), mSignalsBeforeEnter(0), mSignalsBeforeLeave(0), mTimeToWait(MinWait)
	{ }

	void slowdown()
	{
		if(!mWaiting)
		{
			if(++mSignalsBeforeEnter >= NumberOfSignalsBeforeEnteringWaitState)
			{
				mWaiting = true; mTimeToWait = MinWait;
				mSignalsBeforeEnter = 0;
			}
		}
		else
		{
			mTimeToWait += SlowDownStep;
			if(mTimeToWait >= MaxWait) mTimeToWait = MaxWait;
		}
	}

	void speedup()
	{
		if(!mWaiting)
			return;

		if(mTimeToWait > MinWait + SpeedUpStep)
		{
			mTimeToWait -= SpeedUpStep;
		}
		else
		{
			mTimeToWait = MinWait;
			if(++mSignalsBeforeLeave >= NumberOfSignalsBeforeLeavingWaitState)
			{
				mWaiting = false;
				mSignalsBeforeLeave = 0;
			}
		}
	}

	bool is_waiting()
	{
		return mWaiting;
	}

	uint32 time_to_wait()
	{
		return mWaiting ? mTimeToWait : 0;
	}

	void wait()
	{
		if(mWaiting) ::usleep(mTimeToWait);
	}

public:
	bool mWaiting;
	uint32 mSignalsBeforeEnter;
	uint32 mSignalsBeforeLeave;
	uint32 mTimeToWait;
};

} }

#endif /* ZILLIANS_THREADING_ADAPTIVEWAIT_H_ */
