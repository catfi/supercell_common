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

#include "networking/group/CloseProcessGroupManager.h"
#include <tbb/tbb_thread.h>

namespace zillians { namespace net { namespace group {

CloseProcessGroupManager::CloseProcessGroupManager()
{
	mTerminated = false;
}

CloseProcessGroupManager::~CloseProcessGroupManager()
{
	mTerminated = true;
}

void CloseProcessGroupManager::attach(CloseProcessGroup* group)
{
	tbb::queuing_rw_mutex::scoped_lock lock(mGroupsLock, true);
	mGroups.insert(group);
}

void CloseProcessGroupManager::detach(CloseProcessGroup* group)
{
	tbb::queuing_rw_mutex::scoped_lock lock(mGroupsLock, true);
	mGroups.erase(group);
}

void CloseProcessGroupManager::run()
{
	fd_set read_set;

	while(!mTerminated)
	{
		FD_ZERO(&read_set);

		tbb::queuing_rw_mutex::scoped_lock lock(mGroupsLock, false);
		int max_fd = 0;
		for(boost::unordered_set<CloseProcessGroup*>::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
		{
			int fd = 0;
			cpg_fd_get((*it)->mHandle, &fd);
			FD_SET(fd, &read_set);
			if(fd > max_fd) max_fd = fd;
		}

		int available = select(max_fd + 1, &read_set, 0, 0, 0);
		if(available == -1)
		{
			// select failed, log here
			continue;
		}

		if(available > 0)
		{
			for(boost::unordered_set<CloseProcessGroup*>::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
			{
				int fd = 0;
				cpg_fd_get((*it)->mHandle, &fd);
				if(FD_ISSET(fd, &read_set))
				{
					(*it)->dispatch(false /*NON-BLOCKING*/, false /*ALL*/);
				}
			}
		}
		else
		{
			tbb::this_tbb_thread::yield();
		}
	}
}

void CloseProcessGroupManager::stop()
{
	mTerminated = true;
}

} } }
