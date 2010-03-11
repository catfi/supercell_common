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
 * @date Oct 6, 2009 sdk - Initial version created.
 */

#ifndef MEMBERSHIPDISPATCHER_H_
#define MEMBERSHIPDISPATCHER_H_

#include "core-api/Prerequisite.h"
#include "net-api/group/Placeholders.h"
#include <boost/tr1/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>

#define ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY		1

#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
#include <tbb/queuing_rw_mutex.h>
#endif

namespace zillians { namespace net { namespace group {

enum MembershipChangeReason
{
	MEMBER_JOIN,
	MEMBER_LEAVE,
	MEMBER_NODEDOWN,
	MEMBER_NDOEUP,
	MEMBER_PROCDOWN,
};

template<typename Source>
struct MembershipDispatchT
{
	virtual void dispatch(const std::string& group, Source& source, MembershipChangeReason reason) = 0;
};

/**
 * MembershipDispatchBinderT is one type of dispatch implementation to dispatch a
 * buffer completion to user-provided handler (in order to support functor).
 */
template <typename MemberInfo>
struct MembershipDispatchBinderT : public MembershipDispatchT<MemberInfo>
{
	typedef typename boost::function< void (const char*, MemberInfo&, MembershipChangeReason) > handler_type;

	MembershipDispatchBinderT(handler_type h) : handler(h) { }

	//virtual void dispatch(const std::string& group_name, MemberInfo& info, MembershipChangeReason reason)
	virtual void dispatch(const std::string& group_name, MemberInfo& info, MembershipChangeReason reason)
	{
		handler(group_name.c_str(), info, reason);
	}
	handler_type handler;
};

template <typename MemberInfo>
class MembershipDispatcherT
{
	typedef typename boost::tuple<MembershipDispatchT<MemberInfo>*, MembershipDispatchT<MemberInfo>*, MembershipDispatchT<MemberInfo>*> DispatchInfo;
	typedef boost::unordered_map<std::string, DispatchInfo> DispatchMap;

public:
	MembershipDispatcherT()
	{
	}

	~MembershipDispatcherT()
	{
		if(true)
		{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
			tbb::queuing_rw_mutex::scoped_lock lock(mDispatchByGroup.lock, true);
#endif
			for(typename DispatchMap::iterator it = mDispatchByGroup.begin(); it != mDispatchByGroup.end(); ++it)
			{
				SAFE_DELETE(boost::get<0>(it->second));
				SAFE_DELETE(boost::get<1>(it->second));
				SAFE_DELETE(boost::get<2>(it->second));
			}

			mDispatchByGroup.map.clear();
		}

		if(true)
		{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
			tbb::queuing_rw_mutex::scoped_lock lock(mDispatchAny.lock, true);
#endif
			SAFE_DELETE(boost::get<0>(mDispatchAny.any));
			SAFE_DELETE(boost::get<1>(mDispatchAny.any));
			SAFE_DELETE(boost::get<2>(mDispatchAny.any));
		}
	}

	void bindAny(MembershipDispatchT<MemberInfo>* online, MembershipDispatchT<MemberInfo>* offline, MembershipDispatchT<MemberInfo>* error)
	{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex::scoped_lock lock(mDispatchAny.lock, true);
#endif
		SAFE_DELETE(boost::get<0>(mDispatchAny.any));
		SAFE_DELETE(boost::get<1>(mDispatchAny.any));
		SAFE_DELETE(boost::get<2>(mDispatchAny.any));

		boost::get<0>(mDispatchAny.any) = online;
		boost::get<1>(mDispatchAny.any) = offline;
		boost::get<2>(mDispatchAny.any) = error;
	}

	void bindAny(typename MembershipDispatchBinderT<MemberInfo>::handler_type online, typename MembershipDispatchBinderT<MemberInfo>::handler_type offline, typename MembershipDispatchBinderT<MemberInfo>::handler_type error)
	{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex::scoped_lock lock(mDispatchAny.lock, true);
#endif
		SAFE_DELETE(boost::get<0>(mDispatchAny.any));
		SAFE_DELETE(boost::get<1>(mDispatchAny.any));
		SAFE_DELETE(boost::get<2>(mDispatchAny.any));

		boost::get<0>(mDispatchAny.any) = new MembershipDispatchBinderT<MemberInfo>(online);
		boost::get<1>(mDispatchAny.any) = new MembershipDispatchBinderT<MemberInfo>(offline);
		boost::get<2>(mDispatchAny.any) = new MembershipDispatchBinderT<MemberInfo>(error);
	}

	void bind(const std::string& group, MembershipDispatchT<MemberInfo>* online, MembershipDispatchT<MemberInfo>* offline, MembershipDispatchT<MemberInfo>* error)
	{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex::scoped_lock lock(mDispatchByGroup.lock, true);
#endif
		typename DispatchMap::iterator it = mDispatchByGroup.map.find(group);
		if(it != mDispatchByGroup.map.end())
		{
			SAFE_DELETE(boost::get<0>(it->second));
			SAFE_DELETE(boost::get<1>(it->second));
			SAFE_DELETE(boost::get<2>(it->second));

			boost::get<0>(it->second) = online;
			boost::get<1>(it->second) = offline;
			boost::get<2>(it->second) = error;
		}
		else
		{
			DispatchInfo info;
			boost::get<0>(info) = online;
			boost::get<1>(info) = offline;
			boost::get<2>(info) = error;

			bool result;
			boost::tie(it, result) = mDispatchByGroup.map.insert(std::make_pair(group, info));
			if(!result)
			{
				throw std::runtime_error("failed to insert membership dispatcher");
			}
		}
	}

	void bind(const std::string& group, typename MembershipDispatchBinderT<MemberInfo>::handler_type online, typename MembershipDispatchBinderT<MemberInfo>::handler_type offline, typename MembershipDispatchBinderT<MemberInfo>::handler_type error)
	{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex::scoped_lock lock(mDispatchByGroup.lock, true);
#endif
		typename DispatchMap::iterator it = mDispatchByGroup.map.find(group);
		if(it != mDispatchByGroup.map.end())
		{
			SAFE_DELETE(boost::get<0>(it->second));
			SAFE_DELETE(boost::get<1>(it->second));
			SAFE_DELETE(boost::get<2>(it->second));

			boost::get<0>(it->second) = new MembershipDispatchBinderT<MemberInfo>(online);
			boost::get<1>(it->second) = new MembershipDispatchBinderT<MemberInfo>(offline);
			boost::get<2>(it->second) = new MembershipDispatchBinderT<MemberInfo>(error);
		}
		else
		{
			DispatchInfo info;
			boost::get<0>(info) = new MembershipDispatchBinderT<MemberInfo>(online);;
			boost::get<1>(info) = new MembershipDispatchBinderT<MemberInfo>(offline);
			boost::get<2>(info) = new MembershipDispatchBinderT<MemberInfo>(error);

			bool result;
			boost::tie(it, result) = mDispatchByGroup.map.insert(std::make_pair(group, info));
			if(!result)
			{
				throw std::runtime_error("failed to insert membership dispatcher");
			}
		}
	}

	void dispatch(const std::string& group_name, MemberInfo& info, MembershipChangeReason reason)
	{
		uint8 mode = 0;
		switch(reason)
		{
		case MEMBER_JOIN:
		case MEMBER_NDOEUP:
			mode = 0; break;
		case MEMBER_LEAVE:
			mode = 1; break;
		case MEMBER_NODEDOWN:
		case MEMBER_PROCDOWN:
			mode = 2; break;
		}

		MembershipDispatchT<MemberInfo>* dispatch_by_group = NULL;
		if(true)
		{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
			tbb::queuing_rw_mutex::scoped_lock lock(mDispatchByGroup.lock, false);
#endif
			typename DispatchMap::iterator it = mDispatchByGroup.map.find(group_name);
			if(it != mDispatchByGroup.map.end())
			{
				dispatch_by_group = (mode == 0) ? boost::get<0>(it->second) : (mode == 1) ? boost::get<1>(it->second) : boost::get<2>(it->second);
			}
		}

		if(dispatch_by_group)
		{
			dispatch_by_group->dispatch(group_name, info, reason);
		}

		MembershipDispatchT<MemberInfo>* dispatch_any = NULL;
		if(true)
		{
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
			tbb::queuing_rw_mutex::scoped_lock lock(mDispatchAny.lock, false);
#endif
			dispatch_any = (mode == 0) ? boost::get<0>(mDispatchAny.any) : (mode == 1) ? boost::get<1>(mDispatchAny.any) : boost::get<2>(mDispatchAny.any);
		}

		if(dispatch_any)
		{
			dispatch_any->dispatch(group_name, info, reason);
		}
	}

private:
	struct
	{
		DispatchMap map;
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex lock;
#endif
	} mDispatchByGroup;

	struct
	{
		DispatchInfo any;
#if ZILLIANS_MEMBERSHIPDISPATCHER_THREAD_SAFETY
		tbb::queuing_rw_mutex lock;
#endif
	} mDispatchAny;
};

} } }

#endif/*MEMBERSHIPDISPATCHER_H_*/
