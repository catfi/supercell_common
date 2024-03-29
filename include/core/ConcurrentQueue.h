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

#ifndef ZILLIANS_CONCURRENTQUEUE_H_
#define ZILLIANS_CONCURRENTQUEUE_H_

#include "core/Prerequisite.h"

namespace zillians {

/**
 * @brief ConcurrentQueue is a fast concurrent queue supporting multiple producers multiple consumers
 *
 * ConcurrentQueue is built entirely on boost thread library, so it's basically 100% cross-platform compatible
 */
template<typename T>
class ConcurrentQueue : public boost::noncopyable
{
	struct prevent_spurious_wakeup_predicate
	{
		prevent_spurious_wakeup_predicate(std::queue<T>& q) : _q(q) { }
		bool operator () () const { return !_q.empty(); }
		typename std::queue<T>& _q;
	};

public:
	ConcurrentQueue()
	{ }

	~ConcurrentQueue()
	{ }

public:
    void push(T const& data)
    {
        boost::mutex::scoped_lock lock(mMutex);
        mQueue.push(data);
        lock.unlock();
        mConditionVariable.notify_one();
    }

    void clear()
    {
        boost::mutex::scoped_lock lock(mMutex);
        while(!mQueue.empty())
        	mQueue.pop();
    }

    bool empty() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mQueue.empty();
    }

    bool try_peek(T& value)
    {
        boost::mutex::scoped_lock lock(mMutex);
        if(mQueue.empty())
        {
            return false;
        }

        value = mQueue.front();
        return true;
    }

    bool try_pop(T& value)
    {
        boost::mutex::scoped_lock lock(mMutex);
        if(mQueue.empty())
        {
            return false;
        }

        value = mQueue.front();
        mQueue.pop();
        return true;
    }

    void wait_and_pop(T& value)
    {
        boost::mutex::scoped_lock lock(mMutex);
        prevent_spurious_wakeup_predicate p(mQueue);
    	mConditionVariable.wait(lock, p);

        value = mQueue.front();
        mQueue.pop();
    }

    bool timed_wait_and_pop(T& value, const boost::system_time& absolute)
    {
    	boost::mutex::scoped_lock lock(mMutex);
    	prevent_spurious_wakeup_predicate p(mQueue);
    	if(!mConditionVariable.timed_wait(lock, absolute, p))
    		return false;

    	value = mQueue.front();
        mQueue.pop();

        return true;
    }

    template<typename DurationType>
    bool timed_wait_and_pop(T& value, const DurationType& relative)
    {
    	boost::mutex::scoped_lock lock(mMutex);
    	prevent_spurious_wakeup_predicate p(mQueue);
    	if(!mConditionVariable.timed_wait(lock, relative, p))
    		return false;

    	value = mQueue.front();
        mQueue.pop();

        return true;
    }

private:
    std::queue<T> mQueue;
    mutable boost::mutex mMutex;
    boost::condition_variable mConditionVariable;
};

}

#endif /* ZILLIANS_CONCURRENTQUEUE_H_ */
