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
 * @date Aug 13, 2009 rocet - Initial version created.
 */

#include "core/Prerequisite.h"
#include "networking/scylla/ScyllaChannelEngine.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/bind.hpp>

#include <tbb/tbb_thread.h>

using namespace zillians;
using namespace zillians::networking;
using namespace zillians::networking::group;

uint32 test_message_count = 10;

log4cxx::LoggerPtr mLogger(log4cxx::Logger::getLogger("ScyllaBasicTest"));

////////////////////////////////////////////////////////////////////////////
struct MyMessage
{
	enum { TYPE = 11 };

	std::string data;
	int message_no;

	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & data;
		ar & message_no;
	}
};

bool gPassiveMode = false;
tbb::atomic<int> gReceiveCount;

void handle_my_message(shared_ptr<Channel> channel, uint32 type, shared_ptr<Buffer> buffer, shared_ptr<ScyllaChannelEngine> engine)
{
	shared_ptr<MyMessage> message = shared_ptr<MyMessage>(new MyMessage());
	buffer->readSerializable(*message);

	LOG4CXX_INFO(mLogger, "from " << channel->getIdentifier() << ": " << message->data << " (#" << message->message_no << ")");

	if(gPassiveMode)
	{
		message->data = "Hi!!";
		shared_ptr<Buffer> buf(new Buffer(Buffer::probeSize(*message)));
		buf->writeSerializable(*message);
		channel->send(message->TYPE, buf);
	}

	++gReceiveCount;
}

void handle_async_complete(int i)
{
	LOG4CXX_INFO(mLogger, "message #" << i << " sent");
}

//////////////////////////////////////////////////////////////////////////
void test_send(shared_ptr<Channel> channel)
{
	LOG4CXX_INFO(mLogger, "test send");
	for(int i=0;i<test_message_count;++i)
	{
		LOG4CXX_INFO(mLogger, "send message #" << i);

		shared_ptr<MyMessage> message(new MyMessage);
		message->data = "HELLO!!!";
		message->message_no = i;

		shared_ptr<Buffer> buf(new Buffer(Buffer::probeSize(*message)));
		buf->writeSerializable(*message);

		channel->send(message->TYPE, buf);
	}
}

void test_acksend(shared_ptr<Channel> channel)
{
	LOG4CXX_INFO(mLogger, "test acksend");
	for(int i=0;i<test_message_count;++i)
	{
		LOG4CXX_INFO(mLogger, "send message #" << i);

		shared_ptr<MyMessage> message(new MyMessage);
		message->data = "HELLO!!!";
		message->message_no = i;

		shared_ptr<Buffer> buf(new Buffer(Buffer::probeSize(*message)));
		buf->writeSerializable(*message);

		channel->acksend(message->TYPE, buf);
	}
}

void test_acksendasync(shared_ptr<Channel> channel)
{
	LOG4CXX_INFO(mLogger, "test acksendasync");
	for(int i=0;i<test_message_count;++i)
	{
		LOG4CXX_INFO(mLogger, "send message #" << i);

		shared_ptr<MyMessage> message(new MyMessage);
		message->data = "HELLO!!!";
		message->message_no = i;

		shared_ptr<Buffer> buf(new Buffer(Buffer::probeSize(*message)));
		buf->writeSerializable(*message);

		channel->acksendAsync(message->TYPE, buf, boost::bind(handle_async_complete, i));
	}
}

//////////////////////////////////////////////////////////////////////////
void EngineRunThreadProc(shared_ptr<ChannelEngine> engine)
{
	engine->run();
}

void CheckThreadProc(shared_ptr<ChannelEngine> engine)
{
	while(gReceiveCount < test_message_count)
	{
		sleep(1);
	}

	engine->terminate();
}

//////////////////////////////////////////////////////////////////////////
void printHelp(const boost::program_options::options_description& desc)
{
	std::stringstream ss;
	desc.print(ss);
	fprintf(stdout, "Usage: %s", ss.str().c_str());
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	boost::program_options::options_description desc("ScyllaBasicTest");

    desc.add_options()
        ("help,h", "help message")
        ("local,l", boost::program_options::value<std::string>(), "local node name")
        ("target,t", boost::program_options::value<std::string>(), "target node name")
        ("method,m", boost::program_options::value<std::string>(), "possible value: send/acksend/acksendasync")
        ("count,n", boost::program_options::value<uint32>(), "test message count");

    boost::program_options::variables_map vm;
    try
    {
    	// parse the command line
    	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
    	boost::program_options::notify(vm);

        // for -h or --help
        if(vm.count("help"))
        {
        	printHelp(desc);
        	return 1;
        }

        if(vm.count("local") == 0)
        {
        	throw std::invalid_argument("local node id is not specified");
        }

        if(vm.count("count") > 0)
		{
        	test_message_count = vm["count"].as<uint32>();
		}

        if(vm.count("target") > 0)
		{
			if(vm.count("method") == 0)
			{
				throw std::invalid_argument("method is not specified");
			}

			gPassiveMode = false;
		}
		else
		{
			gPassiveMode = true;
		}

		gReceiveCount = 0;

		shared_ptr<ScyllaNodeDB> engineNodeDB(new ScyllaNodeDB());
		shared_ptr<ScyllaChannelEngine> engine(new ScyllaChannelEngine(vm["local"].as<std::string>(), engineNodeDB));
    	engine->registerDefaultDataHandler(
				boost::bind(handle_my_message,
				placeholders::data::source_ref,
				placeholders::data::type,
				placeholders::data::buffer_ref,
				engine));

    	tbb::tbb_thread engineRun(boost::bind(EngineRunThreadProc, engine));
    	tbb::tbb_thread checker(boost::bind(CheckThreadProc, engine));

    	if(vm.count("target") > 0)
    	{
    		UUID target = vm["target"].as<std::string>();
    		shared_ptr<Channel> channel;
    		try
    		{
    			channel = engine->findChannel(target);
    		}
    		catch(...)
    		{
    			fprintf(stderr, "Error: failed to connect to target\n");
    			return -1;
    		}

    		std::string method = vm["method"].as<std::string>();

    		if (method == "acksend")
    		{
    			test_acksend(channel);
    		}
    		else if (method == "acksendasync")
    		{
    			test_acksendasync(channel);
    		}
    		else
    		{
    			test_send(channel);
    		}
    	}



    	if(engineRun.joinable()) engineRun.join();
    	if(checker.joinable()) checker.join();

    }
    catch(boost::program_options::invalid_command_line_syntax&)
    {
    	fprintf(stderr, "Error: invalid command line syntax!\n");
    	printHelp(desc);
    	return -1;
    }
    catch(boost::bad_lexical_cast&)
    {
    	fprintf(stderr, "Error: invalid lexical value!\n");
    	printHelp(desc);
    	return -1;
    }
    catch(std::invalid_argument&)
    {
    	fprintf(stderr, "Error: invalid arguments\n");
    	printHelp(desc);
    	return -1;
    }

	return 0;
}
