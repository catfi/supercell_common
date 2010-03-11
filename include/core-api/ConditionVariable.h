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
 * @date Aug 14, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_CONDITIONVARIABLE_H_
#define ZILLIANS_CONDITIONVARIABLE_H_

#include <tbb/concurrent_queue.h>

namespace zillians {

/**
 * ConditionVariable is a simple condition variable implementation
 * using TBB's concurrent_bounded_queue.
 *
 * By using TBB's concurrent_bounded_queue, the ConditionVariable
 * has a more reliable signal/wait schematics than pthread's
 * condition variable. No signal will be lost, all wait() calls
 * will succeed with corresponding signal() calls (unless we reset
 * before calling wait() )
 *
 * ConditionVariable has a template parameter used to specify the
 * primitive to be used when wait or signal the condition. This
 * can be served as a simple inter-thread communication pipe.
 *
 * @note The template parameter cannot be "bool" type which will
 * lead to memory corruption due to a bug in
 * tbb::concurrent_bounded_queue implementation.
 */
template <typename T>
class ConditionVariable
{
public:
	ConditionVariable()
	{ }

	void reset()
	{
		mQueue.clear();
	}

	void wait(T& result)
	{
		mQueue.pop(result);
	}

	void signal(const T& result)
	{
		mQueue.push(result);
	}

private:
	tbb::concurrent_bounded_queue<T> mQueue;
};

}

#endif/*ZILLIANS_CONDITIONVARIABLE_H_*/
