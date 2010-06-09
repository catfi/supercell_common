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
 * @date Feb 14, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "networking/rdma/infiniband/IBDeviceResourceManager.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

using namespace zillians;
using namespace zillians::networking::rdma;

struct SingletonPool
{
	IBDeviceResourceManager *device_resource_manager;
} gSingletonTool;

void initSingleton()
{
	gSingletonTool.device_resource_manager = new IBDeviceResourceManager();
}

void finiSingleton()
{
	SAFE_DELETE(gSingletonTool.device_resource_manager);
}

int main(int argc, char** argv)
{
	// configure the log4cxx to default
	log4cxx::BasicConfigurator::configure();

	// initialize singleton classes
	initSingleton();

	// finalize singleton classes
	finiSingleton();

	return 0;
}
