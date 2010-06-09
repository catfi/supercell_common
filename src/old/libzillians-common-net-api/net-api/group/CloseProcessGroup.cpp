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

#include "networking/group/CloseProcessGroup.h"

namespace zillians { namespace net { namespace group {

namespace detail {

static void CloseProcessGroupDeliveryCallback(
		cpg_handle_t handle,
		const struct cpg_name *group_name,
		uint32_t nodeid,
		uint32_t pid,
		void *msg,
		size_t msg_len)
{
	cs_error_t result;

	CloseProcessGroup* cpg = NULL;
	result = cpg_context_get(handle, (void**)&cpg);
	BOOST_ASSERT(result == CS_OK);

	// get the dispatcher
	CloseProcessGroup::DataDispatcher* dispatcher = cpg->getDataDispatcher();
	if(!dispatcher)
		return;

	// populate the source info structure (will be used in dispatcher)
	CloseProcessGroup::SourceInfo info;
	info.groupName.append(group_name->value, group_name->length);
	info.nodeId = nodeid;
	info.processId = pid;

	// create buffer object from given data pointer
	shared_ptr<Buffer> buffer(new Buffer((byte*)msg, (std::size_t)msg_len));

	uint32 type; *buffer >> type;
	dispatcher->dispatch(info, type, buffer, msg_len - sizeof(uint32));
}

static void CloseProcessGroupConfigChangeCallback(
	    cpg_handle_t handle,
	    const struct cpg_name *group_name,
	    const struct cpg_address *member_list, size_t member_list_entries,
	    const struct cpg_address *left_list, size_t left_list_entries,
	    const struct cpg_address *joined_list, size_t joined_list_entries)
{
	cs_error_t result;

	CloseProcessGroup* cpg = NULL;
	result = cpg_context_get(handle, (void**)&cpg);
	BOOST_ASSERT(result == CS_OK);

	// get the dispatcher
	CloseProcessGroup::MembershipDispatcher* dispatcher = cpg->getMembershipDispatcher();
	if(!dispatcher)
		return;

	std::string group(group_name->value, group_name->length);

	for(int i=0;i<left_list_entries;++i)
	{
		CloseProcessGroup::MemberInfo info;
		info.nodeId = left_list[i].nodeid;
		info.processId = left_list[i].pid;

		MembershipChangeReason reason;
		switch(left_list[i].reason)
		{
		case CPG_REASON_JOIN:     reason = MEMBER_JOIN; break;
		case CPG_REASON_LEAVE:    reason = MEMBER_LEAVE; break;
		case CPG_REASON_NODEDOWN: reason = MEMBER_NODEDOWN; break;
		case CPG_REASON_NODEUP:   reason = MEMBER_NDOEUP; break;
		case CPG_REASON_PROCDOWN: reason = MEMBER_PROCDOWN; break;
		default:
			BOOST_ASSERT(false);
		}

		dispatcher->dispatch(group, info, reason);
	}

	for(int i=0;i<joined_list_entries;++i)
	{
		CloseProcessGroup::MemberInfo info;
		info.nodeId = joined_list[i].nodeid;
		info.processId = joined_list[i].pid;

		MembershipChangeReason reason;
		switch(joined_list[i].reason)
		{
		case CPG_REASON_JOIN:     reason = MEMBER_JOIN; break;
		case CPG_REASON_LEAVE:    reason = MEMBER_LEAVE; break;
		case CPG_REASON_NODEDOWN: reason = MEMBER_NODEDOWN; break;
		case CPG_REASON_NODEUP:   reason = MEMBER_NDOEUP; break;
		case CPG_REASON_PROCDOWN: reason = MEMBER_PROCDOWN; break;
		default:
			BOOST_ASSERT(false);
		}

		dispatcher->dispatch(group, info, reason);
	}
}

}

CloseProcessGroup::CloseProcessGroup() :
	mDataDispatcher(NULL), mMembershipDispatcher(NULL)
{
	cs_error_t result;

	static cpg_callbacks_t callbacks = {
	    detail::CloseProcessGroupDeliveryCallback,
	    detail::CloseProcessGroupConfigChangeCallback
	};

	result = cpg_initialize(&mHandle, &callbacks);
	if(result != CS_OK)
		throw std::runtime_error("failed to initialize close process group");

	result = cpg_local_get(mHandle, &mLocalIdentifier);
	if(result != CS_OK)
		throw std::runtime_error("failed to determine the local identifier");

	result = cpg_context_set(mHandle, (void*)this);
	BOOST_ASSERT(result == CS_OK);
}

CloseProcessGroup::~CloseProcessGroup()
{
	cs_error_t result = cpg_finalize(mHandle);
	if(result != CS_OK)
		throw std::runtime_error("failed to finalize close process group");
}

bool CloseProcessGroup::send(uint32 type, shared_ptr<Buffer> buffer)
{
	iovec iov[2];
	iov[0].iov_base = &type;
	iov[0].iov_len = sizeof(type);
	iov[1].iov_base = buffer->rptr();
	iov[1].iov_len = buffer->dataSize();

	cs_error_t result = cpg_mcast_joined(mHandle, CPG_TYPE_AGREED, (iovec*)iov, 2);
	if(result != CS_OK)
		return false;

	return true;
}

bool CloseProcessGroup::join(const std::string& group)
{
	BOOST_ASSERT(group.length() < CPG_MAX_NAME_LENGTH - 1);

	cpg_name group_name;
	group_name.length = group.length();
	strncpy(group_name.value, group.c_str(), group.length());
	group_name.value[group_name.length] = 0;

	cs_error_t result = cpg_join(mHandle, &group_name);
	if(result != CS_OK)
		return false;

	return true;
}

bool CloseProcessGroup::leave(const std::string& group)
{
	BOOST_ASSERT(group.length() < CPG_MAX_NAME_LENGTH - 1);

	cpg_name group_name;
	group_name.length = group.length();
	strncpy(group_name.value, group.c_str(), group.length());
	group_name.value[group_name.length] = 0;

	cs_error_t result = cpg_leave(mHandle, &group_name);
	if(result != CS_OK)
		return false;

	return true;
}

bool CloseProcessGroup::dispatch(bool blocking, bool one_or_all)
{
	if(blocking)
	{
		cs_error_t result = cpg_dispatch(mHandle, CS_DISPATCH_BLOCKING);
		if(result != CS_OK)
		{
			BOOST_ASSERT(result == CS_OK);
			return false;
		}
	}
	else
	{
		cs_error_t result = cpg_dispatch(mHandle, (one_or_all) ? CS_DISPATCH_ONE : CS_DISPATCH_ALL);
		if(result != CS_OK)
		{
			return false;
		}
	}

	return true;
}

void CloseProcessGroup::getLocalIdentifier(/*OUT*/ uint32& id)
{
	id = mLocalIdentifier;
}

bool CloseProcessGroup::getMembership(const std::string& group, /*OUT*/ std::vector<MemberInfo>& members)
{
	cpg_name group_name;
	group_name.length = group.length();
	strncpy(group_name.value, group.c_str(), group.length());
	group_name.value[group_name.length] = 0;

	cpg_address entries[ZILLIANS_CLOSEPROCESSGROUP_MAX_PROCESSOR_COUNT];
	int entry_count = ZILLIANS_CLOSEPROCESSGROUP_MAX_PROCESSOR_COUNT;
	cs_error_t result = cpg_membership_get(mHandle, &group_name, (cpg_address*)entries, &entry_count);
	if(result != CS_OK)
		return false;

	for(int i=0;i<entry_count;++i)
	{
		MemberInfo info;
		info.nodeId = entries[i].nodeid;
		info.processId = entries[i].pid;
		members.push_back(info);
	}

	return true;
}

void CloseProcessGroup::setDataDispatcher(CloseProcessGroup::DataDispatcher* dispatcher)
{
	mDataDispatcher = dispatcher;
}

CloseProcessGroup::DataDispatcher* CloseProcessGroup::getDataDispatcher()
{
	return mDataDispatcher;
}

void CloseProcessGroup::setMemberDispatcher(CloseProcessGroup::MembershipDispatcher* dispatcher)
{
	mMembershipDispatcher = dispatcher;
}

CloseProcessGroup::MembershipDispatcher* CloseProcessGroup::getMembershipDispatcher()
{
	return mMembershipDispatcher;
}

} } }
