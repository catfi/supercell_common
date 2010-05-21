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
 * @date May 21, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_THREADCOLLISIONDETECTOR_H_
#define ZILLIANS_THREADCOLLISIONDETECTOR_H_

#include <stdexcept>

#ifdef NDEBUG
	#define DETECTOR(obj)
	#define SCOPED_WATCH(obj)
	#define WATCH(obj)
#else
	#define DETECTOR(obj)		ThreadCollisionDetector _##obj;
	#define SCOPED_WATCH(obj)	ThreadCollisionDetector::ScopedWatcher _scoped_watcher_##obj(_##obj);
	#define WATCH(obj)			ThreadCollisionDetector::Watcher _watcher_##obj(_##obj);
#endif

namespace zillians {

/**
 * ThreadCollisionDetector is used to detect whether two thread accessing the same region of code or variable
 */
class ThreadCollisionDetector
{
public:
	ThreadCollisionDetector() : mActiveThread(0)
    { }

    ~ThreadCollisionDetector()
    { }

    class Watcher
    {
    public:
    	Watcher(ThreadCollisionDetector& _v) : v(_v)
    	{ v.enterSelf(); }

    	~Watcher()
    	{ }

	private:
    	ThreadCollisionDetector& v;
    };

    class ScopedWatcher {
	public:
    	ScopedWatcher(ThreadCollisionDetector& _v) : v(_v)
		{ v.enter(); }

    	~ScopedWatcher()
    	{ v.leave(); }

	private:
    	ThreadCollisionDetector& v;
    };

private:
    void enterSelf()
    {
        if(!__sync_bool_compare_and_swap(&mActiveThread, 0, pthread_self()))
        {
            if(!__sync_bool_compare_and_swap(&mActiveThread, pthread_self(), mActiveThread))
            {
                throw std::runtime_error("thread collision detected");
            }
        }
    }

    void enter()
    {
        if(!__sync_bool_compare_and_swap(&mActiveThread, 0, pthread_self()))
        {
            throw std::runtime_error("thread collision detected");
        }
    }

    void leave()
    {
        __sync_fetch_and_xor(&mActiveThread, mActiveThread);
    }

    pthread_t mActiveThread;
};

}
#endif /* ZILLIANS_THREADCOLLISIONDETECTOR_H_ */
