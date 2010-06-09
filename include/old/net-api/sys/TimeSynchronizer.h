/**
 * Zillians MMO
 * Copyright (C) 2007-2009 Zillians.com, Inc.
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
 * @date Sep 30, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SYS_TIMESYNCHRONIZER_H_
#define ZILLIANS_NET_SYS_TIMESYNCHRONIZER_H_

#include "protocol/TimeSynchronizationRequestMsg.h"
#include "protocol/TimeSynchronizationResponseMsg.h"
#include "networking/sys/Placeholders.h"
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>
#include <tbb/spin_rw_mutex.h>

namespace zillians { namespace net { namespace sys {

template<typename SessionEngine, typename Session>
class TimeSynchronizer
{
	struct SynchronizationContext
	{
		SynchronizationContext(uint32 updateInterval = 0) :
			mUpdateInterval(updateInterval),
			mSynchronized( updateInterval > 0 ? false : true ),
			mBaseTick(tbb::tick_count::now()), mBaseTime(0.0),
			mAdjustmentFactor(1.0),
			mDriftHistorySize(0)
		{ }

		uint32             mUpdateInterval;

		tbb::spin_rw_mutex mUpdateLock;
		tbb::tick_count    mUpdateStartTick;

		volatile bool      mSynchronized;

		tbb::tick_count    mBaseTick;
		double             mBaseTime;

		double             mAdjustmentFactor;

		uint32             mDriftHistorySize;
		std::list<double>  mDriftHistory;
	};

public:
	TimeSynchronizer(SessionEngine& engine) :
		mUpdateRunning(false), mContext(new SynchronizationContext())
	{
		engine.getDispatcher().template bind<TimeSynchronizationRequestMsg>(
				boost::bind(
						&TimeSynchronizer::handleRequest, this,
						placeholders::dispatch::source_ref,
						placeholders::dispatch::message_ref)
				);
	}

	TimeSynchronizer(SessionEngine& engine, Session& session, uint32 updateInterval = DEFAULT_SYNCHRONIZATION_INTERVAL) :
		mUpdateRunning(true)
	{
		mContext = new SynchronizationContext(updateInterval);
		mUpdateThread = new tbb::tbb_thread(boost::bind(&TimeSynchronizer::update, this, &session));

		engine.getDispatcher().template bind<TimeSynchronizationRequestMsg>(
				boost::bind(
						&TimeSynchronizer::handleRequest, this,
						placeholders::dispatch::source_ref,
						placeholders::dispatch::message_ref)
				);

		engine.getDispatcher().template bind<TimeSynchronizationResponseMsg>(
				boost::bind(
						TimeSynchronizer::handleResponse,
						placeholders::dispatch::source_ref,
						placeholders::dispatch::message_ref)
				);

		session.template setContext<SynchronizationContext>(mContext);
	}

	~TimeSynchronizer()
	{
		if(mUpdateRunning)
		{
			mUpdateRunning = false;
			if(mUpdateThread->joinable())
				mUpdateThread->join();
		}
		else
		{
			SAFE_DELETE(mContext);
		}
	}

public:
	double current()
	{
		if(UNLIKELY(!mContext->mSynchronized))
		{
			return 0;
		}
		// NOTE: This while may block under single threaded program
		//while(UNLIKELY(!mContext->mSynchronized))
		//{
		//	tbb::tick_count::interval_t interval(0.5);
		//	tbb::this_tbb_thread::sleep(interval);
		//}

		tbb::spin_rw_mutex::scoped_lock lock(mContext->mUpdateLock, false);
		tbb::tick_count now = tbb::tick_count::now();

		return (now - mContext->mBaseTick).seconds() * mContext->mAdjustmentFactor + mContext->mBaseTime;
	}

private:
	static double averageDriftTime(SynchronizationContext* context)
	{
		if(context->mDriftHistorySize < 3)
		{
			return 0.0;
		}

		double maxDrift = std::numeric_limits<double>::min();
		double minDrift = std::numeric_limits<double>::max();
		double sum = 0.0;
		for(std::list<double>::const_iterator it = context->mDriftHistory.begin(); it != context->mDriftHistory.end(); ++it)
		{
			if(*it > maxDrift) maxDrift = *it;
			if(*it < minDrift) minDrift = *it;

			sum += *it;
		}

		return (sum - maxDrift - minDrift) / (double)(context->mDriftHistorySize - 2);
	}

	void handleRequest(Session& session, TimeSynchronizationRequestMsg& dummy)
	{
		TimeSynchronizationResponseMsg message;
		message.currentTime = current();
		session.writeAsync(message, boost::bind(TimeSynchronizer::writeAsyncCompleted, placeholders::error));
	}

	static void handleResponse(Session& session, TimeSynchronizationResponseMsg& message)
	{
		SynchronizationContext* context = session.template getContext<SynchronizationContext>();

		tbb::tick_count now = tbb::tick_count::now();

		double elapse = (now - context->mUpdateStartTick).seconds();
		//double drift_adjusted = ((now - context->mBaseTick).seconds()*context->mAdjustmentFactor + context->mBaseTime) - (message.currentTime + elapse * 0.5);
		double drift = ((now - context->mBaseTick).seconds() + context->mBaseTime) - (message.currentTime + elapse * 0.5);

		//printf("tick drifting %f ms\n", drift*1000.0);
		//printf("tick drifting (adjusted) %f ms\n", drift_adjusted*1000.0);

		context->mDriftHistory.push_back(drift);
		if(++context->mDriftHistorySize > MAX_DRIFT_HISTORY_SIZE)
		{
			context->mDriftHistory.pop_front();
			--context->mDriftHistorySize;
		}

		{
			tbb::spin_rw_mutex::scoped_lock lock(context->mUpdateLock, true);
			context->mAdjustmentFactor = 1.0 - ( averageDriftTime(context) / (double)context->mUpdateInterval );
			context->mBaseTick = now;
			context->mBaseTime = message.currentTime + elapse * 0.5;
		}

		context->mSynchronized = true;
	}

	void update(Session* session)
	{
		while(true)
		{
			// perform update
			mContext->mUpdateStartTick = tbb::tick_count::now();

			TimeSynchronizationRequestMsg message;
			session->writeAsync(message, boost::bind(TimeSynchronizer::writeAsyncCompleted, placeholders::error));

			for(int i=0;i<mContext->mUpdateInterval*2;++i)
			{
				tbb::tick_count::interval_t interval(0.5);
				tbb::this_tbb_thread::sleep(interval);

				if(!mUpdateRunning)
					return;
			}
		}
	}

	static void writeAsyncCompleted(const boost::system::error_code& ec)
	{ }

	bool mUpdateRunning;
	tbb::tbb_thread* mUpdateThread;

	static const uint32 DEFAULT_SYNCHRONIZATION_INTERVAL = 5;
	static const uint32 MAX_DRIFT_HISTORY_SIZE = 10;

	SynchronizationContext* mContext;
};

} } }

#endif/*ZILLIANS_NET_SYS_TIMESYNCHRONIZER_H_*/
