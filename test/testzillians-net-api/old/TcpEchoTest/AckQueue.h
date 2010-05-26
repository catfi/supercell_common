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
 * @date Jul 2, 2009 sdk - Initial version created.
 */

#ifndef ACKQUEUE_H_
#define ACKQUEUE_H_

#include "core-api/Prerequisite.h"
#include "net-api/Channel.h"
#include <boost/type_traits.hpp>
#include <tbb/tbb_thread.h>

namespace zillians { namespace net {

class AckSlot
{
public:
	AckSlot()		{ reset(); }
	~AckSlot()		{ }
public:
	void wait()		{ while(mCount == 0) tbb::this_tbb_thread::yield(); }
	void signal()	{ mCount++; }
	void reset()	{ mCount = 0; }
private:
	tbb::atomic<uint32> mCount;
};

class AckQueue
{
public:
	AckQueue()	{ }
	~AckQueue()	{ }

public:
	typedef uint32 ack_key_type;

public:
	void wait(ack_key_type key)
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, false);

		AckMap::iterator it = mAckMap.map.find(key);
		BOOST_ASSERT(it != mAckMap.map.end());

		try
		{
			it->second.first->wait();
		}
		catch(...)
		{
			printf("exception catched while waiting\n");
		}

		while(!lock.upgrade_to_writer())
		{
			printf("failed to upgrade to writer\n");
		}

		shared_ptr<AckSlot> cond = it->second.first;
		cond->reset();

		mAckMap.map.erase(it);
		mAckSlotQueue.push(cond);
	}

	void signal(ack_key_type key, shared_ptr<Buffer>* value)
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, false);

		AckMap::iterator it = mAckMap.map.find(key);
		BOOST_ASSERT(it != mAckMap.map.end());

		*it->second.second = *value;

		it->second.first->signal();
	}

	ack_key_type next(shared_ptr<Buffer>* value)
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, true);

		ack_key_type key = ++mCounter;

		shared_ptr<AckSlot> cond;
		if(!mAckSlotQueue.try_pop(cond))
		{
			cond = shared_ptr<AckSlot>(new AckSlot);
		}

		mAckMap.map[key] = std::make_pair(cond, value);

		return key;
	}

private:
	tbb::atomic<ack_key_type> mCounter;

	typedef std::map<ack_key_type, std::pair< shared_ptr<AckSlot>, shared_ptr<Buffer>* > > AckMap;
	struct
	{
		AckMap map;
		tbb::spin_rw_mutex lock;
	} mAckMap;

	typedef tbb::concurrent_queue< shared_ptr<AckSlot> > AckSlotQueue;
	AckSlotQueue mAckSlotQueue;
};

} }

#endif/*ACKQUEUE_H_*/
