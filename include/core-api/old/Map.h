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
 * @date Jun 30, 2009 rocet - Initial version created.
 */

#ifndef ZILLIANS_MAP_H_
#define ZILLIANS_MAP_H_

#include "core-api/Prerequisite.h"
#include <boost/thread/tss.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/spin_rw_mutex.h>
#include <boost/tr1/unordered_map.hpp>
#include <boost/functional/hash.hpp>

namespace zillians {

struct ReadOverWrite;
struct WriteOverRead;
struct EqualReadWrite;

template <class TypeKey, class TypeValue, class AccessTrait>
class Map
{
public:
	typedef std::tr1::unordered_map<TypeKey, TypeValue, std::tr1::hash<TypeKey> > MapType;

public:
	void insert(TypeKey left, TypeValue right);
	const TypeValue& operator[] ( const TypeKey& x );

private:
	MapType mMap;
	tbb::spin_rw_mutex mLock;
};

template <class TypeKey, class TypeValue>
class Map<TypeKey, TypeValue, ReadOverWrite>
{
public:
	Map();

public:
	typedef std::tr1::unordered_map<TypeKey, TypeValue, std::tr1::hash<TypeKey> > MapType;

public:
	struct ThreadData
	{
		MapType* mMainMap;
		MapType* mBufMap;
		tbb::spin_rw_mutex mLock;
		tbb::tick_count mLastMerge;
	};

	class ThreadPool : public std::map<tbb::tbb_thread::id, ThreadData*>
	{
	public:
		ThreadPool();

	public:
		void addThread(ThreadData* threadData);
		void removeThread();
	};

public:
	void insert(TypeKey left, TypeValue right);
	const TypeValue& operator[] ( const TypeKey& x );
	void clear();

private:
	void mergeMap();
	void initThreadData();
	static void cleanupThreadData(ThreadData* data);

private:
	static SharedPtr<ThreadPool> mThreadPool;
	static tbb::spin_rw_mutex mLockThreadPool;
	boost::thread_specific_ptr<ThreadData> mThreadData;

	tbb::tick_count tick;
	float mMergeInterval;
};

///////////////////////////////////////////////////////////////////////////////

template<class TypeKey, class TypeValue, class AccessTrait>
void Map<TypeKey, TypeValue, AccessTrait>::insert(const TypeKey key, const TypeValue value)
{
	tbb::spin_rw_mutex::scoped_lock lock(mLock, true);
	mMap.insert(std::pair<TypeKey, TypeValue>(key, value));
}

template<class TypeKey, class TypeValue, class AccessTrait>
const TypeValue& Map<TypeKey, TypeValue, AccessTrait>::operator[] (const TypeKey& lookup)
{
	tbb::spin_rw_mutex::scoped_lock lock(mLock, false);
	return mMap.operator[](lookup);
}

///////////////////////////////////////////////////////////////////////////////

template<class TypeKey, class TypeValue>
Map<TypeKey, TypeValue, ReadOverWrite>::Map()
: mMergeInterval(100.0), //100ms
mThreadData(&Map::cleanupThreadData)
{
	mThreadPool = SharedPtr<ThreadPool>(new ThreadPool());
}

template<class TypeKey, class TypeValue>
SharedPtr<typename Map<TypeKey, TypeValue, ReadOverWrite>::ThreadPool> Map<TypeKey, TypeValue, ReadOverWrite>::mThreadPool;

template<class TypeKey, class TypeValue>
tbb::spin_rw_mutex Map<TypeKey, TypeValue, ReadOverWrite>::mLockThreadPool;

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::initThreadData()
{
	ThreadData* threadData = new ThreadData();
	threadData->mMainMap = new MapType();
	threadData->mBufMap = new MapType();
	threadData->mLastMerge = tbb::tick_count::now();
	mThreadData.reset(threadData);


	tbb::spin_rw_mutex::scoped_lock lockThreadPool(mLockThreadPool, true);
	mThreadPool->addThread(threadData);
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::cleanupThreadData(ThreadData* threadData)
{
	{
		tbb::spin_rw_mutex::scoped_lock lockThreadPool(mLockThreadPool, true);
		mThreadPool->removeThread();
	}

	SAFE_DELETE(threadData->mMainMap);
	SAFE_DELETE(threadData->mBufMap);
	SAFE_DELETE(threadData);
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::mergeMap()
{
	tbb::spin_rw_mutex::scoped_lock lockMap(mThreadData->mLock, true);
	if (mThreadData->mBufMap->size() <= 0)
	{
		//no data to merge
		return;
	}

	//merge mBufMap to mMainMap
	MapType* mainMap = mThreadData->mMainMap;
	MapType* newMap = new MapType(*mainMap);

	for (typename MapType::iterator i = mThreadData->mBufMap->begin(); i != mThreadData->mBufMap->end(); i++)
	{
		newMap->insert(std::pair<TypeKey, TypeValue>(i->first, i->second));
	}
	mThreadData->mBufMap->clear();

	mThreadData->mMainMap = newMap;
	SAFE_DELETE(mainMap);
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::insert(const TypeKey key, const TypeValue value)
{
	if (mThreadData.get() == NULL)
	{
		initThreadData();
	}

	tbb::spin_rw_mutex::scoped_lock lockThreadPool(mLockThreadPool, false);

	//insert data into mBufMap
	for (typename ThreadPool::iterator i = mThreadPool->begin(); i != mThreadPool->end(); i++)
	{
		tbb::spin_rw_mutex::scoped_lock lockMap(i->second->mLock, true);
		i->second->mBufMap->insert(std::pair<TypeKey, TypeValue>(key, value));
	}
}

template<class TypeKey, class TypeValue>
const TypeValue& Map<TypeKey, TypeValue, ReadOverWrite>::operator[] (const TypeKey& lookup)
{
	if (mThreadData.get() == NULL)
	{
		initThreadData();
	}

	if ((tbb::tick_count::now() - mThreadData->mLastMerge).seconds() * 1000.0 >= mMergeInterval)
	{
		mThreadData->mLastMerge = tbb::tick_count::now();
		mergeMap();
	}

	typename MapType::iterator iter;

	//lookup in mMainMap
	iter = mThreadData->mMainMap->find(lookup);
	if (iter == mThreadData->mMainMap->end())
	{
		//lookup in mBufMap
		tbb::spin_rw_mutex::scoped_lock lockMap(mThreadData->mLock, false);
		iter = mThreadData->mBufMap->find(lookup);
		if (iter == mThreadData->mBufMap->end())
		{
			throw std::out_of_range("out_of_range");
		}
	}

	return iter->second;
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::clear()
{
	if (mThreadData.get() == NULL)
	{
		initThreadData();
	}

	mThreadData->mMainMap->clear();
	tbb::spin_rw_mutex::scoped_lock lockMap(mThreadData->mLock, true);
	mThreadData->mBufMap->clear();
}

///////////////////////////////////////////////////////////////////////////////

template<class TypeKey, class TypeValue>
Map<TypeKey, TypeValue, ReadOverWrite>::ThreadPool::ThreadPool()
{
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::ThreadPool::addThread(ThreadData* threadData)
{
	this->insert(std::pair<tbb::tbb_thread::id , ThreadData*>(tbb::this_tbb_thread::get_id(), threadData));
}

template<class TypeKey, class TypeValue>
void Map<TypeKey, TypeValue, ReadOverWrite>::ThreadPool::removeThread()
{
	typename Map<TypeKey, TypeValue, ReadOverWrite>::ThreadPool::iterator i = this->find(tbb::this_tbb_thread::get_id());
	if (i != this->end())
	{
		this->erase(i);
	}
}

}
#endif/*ZILLIANS_MAP_H_*/
