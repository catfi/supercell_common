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
 * @date Feb 12, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_SINGLETON_H_
#define ZILLIANS_SINGLETON_H_

#include "core-api/Common.h"
#include <boost/assert.hpp>

namespace zillians {

/**
 * @brief Simple implementation of Singleton Pattern
 *
 * Inspired by OGRE, the Singleton class can be created once, and access through instance() method
 * NOTE: have potential problem when linking against dynamic library (nothing said that...)
 * NOTE: this is NOT thread-safe because the Singleton object can be created in two different threads in the same time
 */
template <class T, bool AUTO = false>
class Singleton
{
public:
	Singleton()
	{
		BOOST_ASSERT(!mInstance);

#if defined( _MSC_VER ) && _MSC_VER < 1200
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		mInstance = (T*)((int)this + offset);
#else
		mInstance = static_cast<T*>(this);
#endif
	}

	~Singleton() { }

public:
	static T* instance()
	{
		if(AUTO && !mInstance)
		{
			mInstance = new T;
		}

		return mInstance;
	}

private:
	static T* mInstance;
};

template <typename T, bool AUTO> T* Singleton<T, AUTO>::mInstance = NULL;

}

#endif/*ZILLIANS_SINGLETON_H_*/
