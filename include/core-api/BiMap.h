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
 * @date Jun 10, 2009 rocet - Initial version created.
 */

#ifndef ZILLIANS_BIMAP_H_
#define ZILLIANS_BIMAP_H_

#include "core-api/Prerequisite.h"
#include <boost/thread/tss.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/spin_rw_mutex.h>

namespace zillians {

template <class TypeLeft, class TypeRight>
class BiMap
{
public:
	BiMap();

public:
	typedef boost::bimaps::bimap< typename boost::bimaps::unordered_set_of< TypeLeft >, typename boost::bimaps::unordered_set_of< TypeRight > > MapType;

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
	void initThreadData();
	static void cleanupThreadData(ThreadData* data);
	void mergeMap();
	void insert(TypeLeft left, TypeRight right);
	TypeLeft mapLeft(TypeRight lookup);
	TypeRight mapRight(TypeLeft lookup);

private:
	static shared_ptr<ThreadPool> mThreadPool;
	static tbb::spin_rw_mutex mLockThreadPool;
	boost::thread_specific_ptr<ThreadData> mThreadData;

	tbb::tick_count tick;
	float mMergeInterval;
};

///////////////////////////////////////////////////////////////////////////////

template<class TypeLeft, class TypeRight>
BiMap<TypeLeft, TypeRight>::BiMap()
: mMergeInterval(100.0), //100ms
mThreadData(&BiMap::cleanupThreadData)
{
	mThreadPool = shared_ptr<ThreadPool>(new ThreadPool());
}

template<class TypeLeft, class TypeRight>
shared_ptr<typename BiMap<TypeLeft, TypeRight>::ThreadPool> BiMap<TypeLeft, TypeRight>::mThreadPool;

template<class TypeLeft, class TypeRight>
tbb::spin_rw_mutex BiMap<TypeLeft, TypeRight>::mLockThreadPool;

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::initThreadData()
{
	ThreadData* threadData = new ThreadData();
	threadData->mMainMap = new MapType();
	threadData->mBufMap = new MapType();
	threadData->mLastMerge = tbb::tick_count::now();
	mThreadData.reset(threadData);


	tbb::spin_rw_mutex::scoped_lock lockThreadPool(mLockThreadPool, true);
	mThreadPool->addThread(threadData);
}

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::cleanupThreadData(ThreadData* threadData)
{
	{
		tbb::spin_rw_mutex::scoped_lock lockThreadPool(mLockThreadPool, true);
		mThreadPool->removeThread();
	}

	SAFE_DELETE(threadData->mMainMap);
	SAFE_DELETE(threadData->mBufMap);
	SAFE_DELETE(threadData);
}

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::mergeMap()
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
		newMap->insert(typename MapType::value_type(i->left, i->right));
	}
	mThreadData->mBufMap->clear();

	mThreadData->mMainMap = newMap;
	SAFE_DELETE(mainMap);
}

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::insert(TypeLeft left, TypeRight right)
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
		i->second->mBufMap->insert(typename MapType::value_type(left, right));
	}
}

template<class TypeLeft, class TypeRight>
TypeLeft BiMap<TypeLeft, TypeRight>::mapLeft(TypeRight lookup)
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

	typename MapType::right_const_iterator iter;

	//lookup in mMainMap
	iter = mThreadData->mMainMap->right.find(lookup);
	if (iter == mThreadData->mMainMap->right.end())
	{
		//lookup in mBufMap
		iter = mThreadData->mBufMap->right.find(lookup);
		if (iter == mThreadData->mBufMap->right.end())
		{
			throw std::out_of_range("out_of_range");
		}
	}

	return iter->second;
}

template<class TypeLeft, class TypeRight>
TypeRight BiMap<TypeLeft, TypeRight>::mapRight(TypeLeft lookup)
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

	typename MapType::left_const_iterator iter;

	//lookup in mMainMap
	iter = mThreadData->mMainMap->left.find(lookup);
	if (iter == mThreadData->mMainMap->left.end())
	{
		//lookup in mBufMap
		iter = mThreadData->mBufMap->left.find(lookup);
		if (iter == mThreadData->mBufMap->left.end())
		{
			throw std::out_of_range("out_of_range");
		}
	}

	return iter->second;
}

///////////////////////////////////////////////////////////////////////////////

template<class TypeLeft, class TypeRight>
BiMap<TypeLeft, TypeRight>::ThreadPool::ThreadPool()
{
}

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::ThreadPool::addThread(ThreadData* threadData)
{
	this->insert(std::pair<tbb::tbb_thread::id , ThreadData*>(tbb::this_tbb_thread::get_id(), threadData));
}

template<class TypeLeft, class TypeRight>
void BiMap<TypeLeft, TypeRight>::ThreadPool::removeThread()
{
	typename BiMap<TypeLeft, TypeRight>::ThreadPool::iterator i = this->find(tbb::this_tbb_thread::get_id());
	if (i != this->end())
	{
		this->erase(i);
	}
}

}

#endif/*ZILLIANS_BIMAP_H_*/
