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

#ifndef ZILLIANS_NET_GROUP_CLOSEPROCESSGROUP_H_
#define ZILLIANS_NET_GROUP_CLOSEPROCESSGROUP_H_

#include "core/Prerequisite.h"
#include "networking/group/DataDispatcher.h"
#include "networking/group/MembershipDispatcher.h"
#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/swab.h>

#define ZILLIANS_CLOSEPROCESSGROUP_MAX_PROCESSOR_COUNT	CPG_MEMBERS_MAX

namespace zillians { namespace net { namespace group {

class CloseProcessGroup
{
	friend class CloseProcessGroupManager;
public:
	struct SourceInfo
	{
		uint32 nodeId;
		uint32 processId;
		std::string groupName;
	};
	typedef DataDispatcherT<SourceInfo> DataDispatcher;

	struct MemberInfo
	{
		uint32 nodeId;
		uint32 processId;
	};
	typedef MembershipDispatcherT<MemberInfo> MembershipDispatcher;

	struct MemberInfoHasher
	{
	    static size_t hash( const MemberInfo& x )
	    {
	    	return *((uint32*)(&x.nodeId)) + *((uint32*)(&x.processId));
	    }
	    static bool equal( const MemberInfo& x, const MemberInfo& y )
	    {
	        return (x.nodeId == y.nodeId && x.processId == y.processId);
	    }
	};

	CloseProcessGroup();
	~CloseProcessGroup();

public:
	bool send(uint32 type, shared_ptr<Buffer> buffer);
	bool join(const std::string& group);
	bool leave(const std::string& group);
	bool dispatch(bool blocking = true, bool one_or_all = true);

public:
	void getLocalIdentifier(/*OUT*/ uint32& id);
	bool getMembership(const std::string& group, /*OUT*/ std::vector<MemberInfo>& members);

public:
	void setDataDispatcher(CloseProcessGroup::DataDispatcher* dispatcher);
	CloseProcessGroup::DataDispatcher* getDataDispatcher();

	void setMemberDispatcher(CloseProcessGroup::MembershipDispatcher* dispatcher);
	CloseProcessGroup::MembershipDispatcher* getMembershipDispatcher();

private:
	uint32 mLocalIdentifier;
	cpg_handle_t mHandle;
	CloseProcessGroup::DataDispatcher* mDataDispatcher;
	CloseProcessGroup::MembershipDispatcher* mMembershipDispatcher;
};

} } }

#endif/*ZILLIANS_NET_GROUP_CLOSEPROCESSGROUP_H_*/
