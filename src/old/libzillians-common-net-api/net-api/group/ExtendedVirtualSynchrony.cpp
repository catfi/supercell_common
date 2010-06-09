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
 * @date Oct 8, 2009 sdk - Initial version created.
 */

#include "net-api/group/ExtendedVirtualSynchrony.h"

namespace zillians { namespace net { namespace group {

namespace detail {

static void ExtendedVirtualSynchronyDeliveryCallback(
		hdb_handle_t h,
		unsigned int nodeid,
		const void *msg,
		size_t msg_len)
{
	cs_error_t result;
	evs_handle_t handle = h;

	ExtendedVirtualSynchrony* evs = NULL;
	result = evs_context_get(handle, (void**)&evs);
	BOOST_ASSERT(result == CS_OK);

	// get the dispatcher
	ExtendedVirtualSynchrony::DataDispatcher* dispatcher = evs->getDataDispatcher();
	if(!dispatcher)
		return;

	// populate the source info structure (will be used in dispatcher)
	ExtendedVirtualSynchrony::SourceInfo info;
	info.nodeId = nodeid;

	// create buffer object from given data pointer
	shared_ptr<Buffer> buffer(new Buffer((const byte*)msg, (std::size_t)msg_len));

	uint32 type; *buffer >> type;

	dispatcher->dispatch(info, type, buffer, msg_len - sizeof(uint32));
}

static void ExtendedVirtualSynchronyConfigChangeCallback(
	    hdb_handle_t h,
	    const unsigned int *member_list, size_t member_list_entries,
	    const unsigned int *left_list, size_t left_list_entries,
	    const unsigned int *joined_list, size_t joined_list_entries,
	    const struct evs_ring_id* ring_id)
{
	cs_error_t result;
	evs_handle_t handle = h;

	ExtendedVirtualSynchrony* evs = NULL;
	result = evs_context_get(handle, (void**)&evs);
	BOOST_ASSERT(result == CS_OK);

	// get the dispatcher
	ExtendedVirtualSynchrony::MembershipDispatcher* dispatcher = evs->getMembershipDispatcher();
	if(!dispatcher)
		return;

	const std::string dummy_group("evs_local");

	for(int i=0;i<left_list_entries;++i)
	{
		ExtendedVirtualSynchrony::MemberInfo info;
		info.nodeId = left_list[i];

		MembershipChangeReason reason = MEMBER_LEAVE;

		dispatcher->dispatch(dummy_group, info, reason);
	}

	for(int i=0;i<joined_list_entries;++i)
	{
		ExtendedVirtualSynchrony::MemberInfo info;
		info.nodeId = joined_list[i];

		MembershipChangeReason reason = MEMBER_JOIN;

		dispatcher->dispatch(dummy_group, info, reason);
	}
}

}

ExtendedVirtualSynchrony::ExtendedVirtualSynchrony() :
	mDataDispatcher(NULL), mMembershipDispatcher(NULL)
{
	cs_error_t result;

	static evs_callbacks_t callbacks = {
	    detail::ExtendedVirtualSynchronyDeliveryCallback,
	    detail::ExtendedVirtualSynchronyConfigChangeCallback
	};

	result = evs_initialize(&mHandle, &callbacks);
	if(result != CS_OK)
		throw std::runtime_error("failed to initialize extended virtual synchrony");

	std::size_t size = 128;
	uint32 lists[128];

	result = evs_membership_get(mHandle, &mLocalIdentifier, (uint32*)lists, &size);
	if(result != CS_OK)
		throw std::runtime_error("failed to determine the local identifier");

	result = evs_context_set(mHandle, (void*)this);
	BOOST_ASSERT(result == CS_OK);
}

ExtendedVirtualSynchrony::~ExtendedVirtualSynchrony()
{
	cs_error_t result = evs_finalize(mHandle);
	if(result != CS_OK)
		throw std::runtime_error("failed to finalize extended virtual synchrony");
}

bool ExtendedVirtualSynchrony::send(uint32 type, shared_ptr<Buffer> buffer)
{
	iovec iov[2];
	iov[0].iov_base = &type;
	iov[0].iov_len = sizeof(type);
	iov[1].iov_base = buffer->rptr();
	iov[1].iov_len = buffer->dataSize();

	cs_error_t result = evs_mcast_joined(mHandle, EVS_TYPE_AGREED, (iovec*)iov, 2);
	if(result != CS_OK)
		return false;

	return true;
}

bool ExtendedVirtualSynchrony::join(const std::string& group)
{
	BOOST_ASSERT(group.length() <= 32);

	evs_group eg;
	strncpy(eg.key, group.c_str(), group.length());

	cs_error_t result = evs_join(mHandle, &eg, 1);
	if(result != CS_OK)
		return false;

	return true;
}

bool ExtendedVirtualSynchrony::leave(const std::string& group)
{
	BOOST_ASSERT(group.length() <= 32);

	evs_group eg;
	strncpy(eg.key, group.c_str(), group.length());

	cs_error_t result = evs_leave(mHandle, &eg, 1);
	if(result != CS_OK)
		return false;

	return true;
}

bool ExtendedVirtualSynchrony::run(bool blocking)
{
	if(blocking)
	{
		cs_error_t result = evs_dispatch(mHandle, CS_DISPATCH_BLOCKING);
		if(result != CS_OK)
		{
			BOOST_ASSERT(result == CS_OK);
			return false;
		}
	}
	else
	{
		cs_error_t result = evs_dispatch(mHandle, CS_DISPATCH_ALL);
		if(result != CS_OK)
		{
			return false;
		}
	}

	return true;
}

void ExtendedVirtualSynchrony::getLocalIdentifier(/*OUT*/ uint32& id)
{
	id = mLocalIdentifier;
}

bool ExtendedVirtualSynchrony::getLocalMembership(/*OUT*/ std::vector<MemberInfo>& members)
{
	uint32 dummyId;
	uint32 entries[ZILLIANS_EXTENDEDVIRTUALSYNCHRONY_MAX_PROCESSOR_COUNT];
	std::size_t entry_count = ZILLIANS_EXTENDEDVIRTUALSYNCHRONY_MAX_PROCESSOR_COUNT;
	cs_error_t result = evs_membership_get(mHandle, &dummyId, (unsigned int*)entries, &entry_count);
	if(result != CS_OK)
		return false;

	BOOST_ASSERT(dummyId == mLocalIdentifier);

	for(int i=0;i<entry_count;++i)
	{
		MemberInfo info;
		info.nodeId = entries[i];
		members.push_back(info);
	}

	return true;
}

void ExtendedVirtualSynchrony::setDataDispatcher(ExtendedVirtualSynchrony::DataDispatcher* dispatcher)
{
	mDataDispatcher = dispatcher;
}

ExtendedVirtualSynchrony::DataDispatcher* ExtendedVirtualSynchrony::getDataDispatcher()
{
	return mDataDispatcher;
}

void ExtendedVirtualSynchrony::setMemberDispatcher(ExtendedVirtualSynchrony::MembershipDispatcher* dispatcher)
{
	mMembershipDispatcher = dispatcher;
}

ExtendedVirtualSynchrony::MembershipDispatcher* ExtendedVirtualSynchrony::getMembershipDispatcher()
{
	return mMembershipDispatcher;
}

} } }
