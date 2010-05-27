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


#include "core-api/Prerequisite.h"
#include "core-api/BiMap.h"
#include "util-api/UUIDUtil.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

using namespace zillians;
using namespace std;

#define NUM_THREAD	2
#define NUM_LOOKUP	10000

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("zillians.common.core-api.BiMapTest"));

volatile bool gTerminated = false;

void ThreadReader(volatile int* counter, shared_ptr<BiMap<int, UUID> > map)
{
	LOG4CXX_INFO(logger, "Thread ID: " << tbb::this_tbb_thread::get_id());

	while (*counter <= 0);

	int missed = 0;
	tbb::tick_count start, end;
	start = tbb::tick_count::now();
	for (int i=0; i<NUM_LOOKUP; i++)
	{
		int c = (*counter), r = rand();
		int random = r % c;
		try
		{
			//int tmp = map->mapLeft(UUID("e6cd9ec2-564a-11de-9c55-001d92648328"));
			UUID tmp = map->mapRight(random);
			//LOG4CXX_INFO(logger, tmp);
		}
		catch (...)
		{
			missed++;
		}
	}
	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	LOG4CXX_INFO(logger, "Thread " << tbb::this_tbb_thread::get_id() << ": " << total << " ms / " << total / NUM_LOOKUP << " ms");
	LOG4CXX_INFO(logger, "Thread " << tbb::this_tbb_thread::get_id() << ": found: " << NUM_LOOKUP - missed << " missed: " << missed);
}

void ThreadWriter(volatile int* counter, shared_ptr<BiMap<int, UUID> > map)
{
	while (!gTerminated)
	{
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif//WIN32
		UUID uuid;
		uuid.random();
		map->insert((*counter), uuid);
		(*counter)++;
	}

}

int main()
{
	srand(time(NULL));
	log4cxx::BasicConfigurator::configure();
	std::list<tbb::tbb_thread*> threads;
	shared_ptr<BiMap<int, UUID> > map = shared_ptr<BiMap<int, UUID> >(new BiMap<int, UUID>());
	volatile int counter = 0;

	LOG4CXX_INFO(logger, "NUM_THREAD: " << NUM_THREAD);

	tbb::tbb_thread* threadWriter = new tbb::tbb_thread(boost::bind(&ThreadWriter, &counter, map));
	for (int i=0; i<NUM_THREAD; i++)
	{
		tbb::tbb_thread* thread;
		thread = new tbb::tbb_thread(boost::bind(&ThreadReader, &counter, map));
		threads.push_back(thread);
	}

	for (std::list<tbb::tbb_thread*>::iterator i=threads.begin(); i!=threads.end(); i++)
	{
		(*i)->join();
	}

	gTerminated = true;
	threadWriter->join();

	for (std::list<tbb::tbb_thread*>::iterator i=threads.begin(); i!=threads.end(); i++)
	{
		SAFE_DELETE(*i);
	}
	threads.clear();

	SAFE_DELETE(threadWriter);

	return 0;
}
