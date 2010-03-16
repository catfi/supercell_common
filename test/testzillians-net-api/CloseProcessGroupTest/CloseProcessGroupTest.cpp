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
 * @date Oct 7, 2009 rocet - Initial version created.
 */


#include "core-api/Prerequisite.h"
#include "net-api/group/CloseProcessGroup.h"
#include <tbb/tbb_thread.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#define MAX_TYPE	1024

using namespace zillians;
using namespace zillians::net::group;

log4cxx::LoggerPtr mLogger(log4cxx::Logger::getLogger("CloseProcessGroupTest"));

void RunThreadProc(CloseProcessGroup* cpg)
{
	bool result = cpg->dispatch();
}

void OnlineHandler(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{
	LOG4CXX_INFO(mLogger, memberInfo.nodeId << " joins group(" << groupName << "): " << reason);
}

void OfflineHandler(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{
	LOG4CXX_INFO(mLogger, memberInfo.nodeId << " leaves group(" << groupName << "): " << reason);
}

void ErrorHandler(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{

}

void BufferHandler(CloseProcessGroup::SourceInfo& source, uint32 type, SharedPtr<Buffer>& b, std::size_t size)
{

}

int main (int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <group>\n";
		return 1;
	}

	CloseProcessGroup cpg;
	std::vector<CloseProcessGroup::MemberInfo> members;

	CloseProcessGroup::MembershipDispatcher* membershipDispatcher = new CloseProcessGroup::MembershipDispatcher();
	membershipDispatcher->bindAny(
			boost::bind(OnlineHandler, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason),
			boost::bind(OfflineHandler, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason),
			boost::bind(ErrorHandler, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason)
	);

	CloseProcessGroup::DataDispatcher* dataDispatcher = new CloseProcessGroup::DataDispatcher(MAX_TYPE);
	dataDispatcher->bindAny(
			boost::bind(BufferHandler, placeholders::data::source_ref, placeholders::data::type, placeholders::data::buffer_ref, placeholders::data::size)
	);

	cpg.setMemberDispatcher(membershipDispatcher);
	cpg.setDataDispatcher(dataDispatcher);

	bool result = cpg.join(argv[1]);
	if (!result)
	{
		LOG4CXX_ERROR(mLogger, "join group(" << argv[1] << ") failed");
		return 1;
	}

	result = cpg.getMembership(argv[1], members);
	if (!result)
	{
		LOG4CXX_ERROR(mLogger, "get membership of group(" << argv[1] << ") failed");
		return 1;
	}

	LOG4CXX_INFO(mLogger, members.size() << " nodes in group(" << argv[1] << ")");

	//for (int i = 0; i < 10; i++)
	//{

	//}

	tbb::tbb_thread engineRun(boost::bind(RunThreadProc, &cpg));
	if(engineRun.joinable()) engineRun.join();

	return 0;
}
