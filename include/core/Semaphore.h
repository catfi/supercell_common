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

#ifndef ZILLIANS_SEMAPHORE_H_
#define ZILLIANS_SEMAPHORE_H_

#include "core/Common.h"
#include <semaphore.h>

namespace zillians {

class Semaphore
{
public:
	Semaphore()
	{
		int result = sem_init(&mSemaphore, 0 /* in-process semaphore */, 0 /* initial value */);
		BOOST_ASSERT(result != -1);
	}

	~Semaphore()
	{
		int result = sem_destroy(&mSemaphore);
		BOOST_ASSERT(result != -1);
	}

public:
	inline void wait()
	{
		int result = sem_wait(&mSemaphore);
		BOOST_ASSERT(result != -1);
	}

	inline void post()
	{
		int result = sem_post(&mSemaphore);
		BOOST_ASSERT(result != -1);
	}

private:
	sem_t mSemaphore;

	// forbid object copy constructor and copy operator
private:
	Semaphore(const Semaphore&);
	void operator = (const Semaphore&);
};

}

#endif /* ZILLIANS_SEMAPHORE_H_ */
