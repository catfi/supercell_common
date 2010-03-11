//
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Contact Information: info@zillians.com
//

#include "LibEvHello.h"
#include "LibEvHelloClient.h"
#include "LibEvHelloServer.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/scalable_allocator.h>

//using namespace log4cxx;
//using namespace log4cxx::helpers;


int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	if(argc != 3)
		return ZN_ERROR;

	if(strcmp(argv[1], "server") == 0)
	{
		tbb::tbb_thread server_thread(boost::bind(serverThreadProc, std::string(argv[2])));
		server_thread.join();
	}
	else if(strcmp(argv[1], "client") == 0)
	{
		tbb::tbb_thread client_thread(boost::bind(clientThreadProc, std::string(argv[2])));
		client_thread.join();
	}
	else
	{
		return ZN_ERROR;
	}

	return ZN_OK;
}
