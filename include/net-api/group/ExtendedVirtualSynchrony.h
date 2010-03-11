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

#ifndef ZILLIANS_NET_GROUP_EXTENDEDVIRTUALSYNCHRONY_H_
#define ZILLIANS_NET_GROUP_EXTENDEDVIRTUALSYNCHRONY_H_

#include "core-api/Prerequisite.h"
#include "net-api/group/DataDispatcher.h"
#include "net-api/group/MembershipDispatcher.h"
#include <corosync/corotypes.h>
#include <corosync/evs.h>

#define ZILLIANS_EXTENDEDVIRTUALSYNCHRONY_MAX_PROCESSOR_COUNT 128

namespace zillians { namespace net { namespace group {

class ExtendedVirtualSynchrony
{
public:
	struct SourceInfo
	{
		uint32 nodeId;
	};
	typedef DataDispatcherT<SourceInfo> DataDispatcher;

	struct MemberInfo
	{
		uint32 nodeId;
	};
	typedef MembershipDispatcherT<MemberInfo> MembershipDispatcher;

	ExtendedVirtualSynchrony();
	~ExtendedVirtualSynchrony();

public:
	bool send(uint32 type, SharedPtr<Buffer> buffer);
	bool join(const std::string& group);
	bool leave(const std::string& group);
	bool run(bool blocking = true);

public:
	void getLocalIdentifier(/*OUT*/ uint32& id);
	bool getLocalMembership(/*OUT*/ std::vector<MemberInfo>& members);

public:
	void setDataDispatcher(ExtendedVirtualSynchrony::DataDispatcher* dispatcher);
	ExtendedVirtualSynchrony::DataDispatcher* getDataDispatcher();

	void setMemberDispatcher(ExtendedVirtualSynchrony::MembershipDispatcher* dispatcher);
	ExtendedVirtualSynchrony::MembershipDispatcher* getMembershipDispatcher();

private:
	uint32 mLocalIdentifier;
	evs_handle_t mHandle;
	ExtendedVirtualSynchrony::DataDispatcher* mDataDispatcher;
	ExtendedVirtualSynchrony::MembershipDispatcher* mMembershipDispatcher;

};

} } }

#endif/*ZILLIANS_NET_GROUP_EXTENDEDVIRTUALSYNCHRONY_H_*/
