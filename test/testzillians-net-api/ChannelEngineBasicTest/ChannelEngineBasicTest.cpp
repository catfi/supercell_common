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
 * @date Feb 26, 2009 sdk - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "net-api/scylla/ScyllaConfiguration.h"
#include "net-api/scylla/ScyllaChannelEngine.h"
#include "net-api/Message.h"
#include "net-api/MessageFactory.h"
#include "net-api/MessageHandler.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/bind.hpp>

#include <tbb/tbb_thread.h>

#define TIXML_USE_TICPP
#include <ticpp/ticpp.h>

using namespace zillians;
using namespace zillians::net;
using namespace ticpp;

#define TEST_MESSAGE_COUNT	1000

//////////////////////////////////////////////////////////////////////////
class MyMessage : public Message
{
public:
	enum { TYPE = 0x0001 };

	virtual uint32 type()
	{
		return TYPE;
	}

	virtual uint32 size()
	{
		Buffer::probeSize(data);
	}

	virtual void decode(Buffer &buffer)
	{
		buffer >> data;
	}

	virtual void encode(Buffer &buffer)
	{
		buffer << data;
	}

public:
	std::string data;
};

class MyMessageFactory : public MessageFactory
{
public:
	virtual SharedPtr<Message> create(uint32 type)
	{
		BOOST_ASSERT(type == MyMessage::TYPE);
		return SharedPtr<Message>(new MyMessage);
	}
};

bool gPassiveMode = false;
tbb::atomic<int> gReceiveCount;

class MyMessageHandler : public MessageHandler
{
public:
	virtual void onMessage(uint32 type, SharedPtr<Channel> channel, SharedPtr<Message> message)
	{
		SharedPtr<MyMessage> mymsg = boost::static_pointer_cast<MyMessage>(message);
		LOG4CXX_INFO(mLogger, "from " << channel->getIdentifier() << ": " << mymsg->data);
		if(gPassiveMode)
		{
			channel->send(message);
		}
		++gReceiveCount;
	}

private:
	static log4cxx::LoggerPtr mLogger;

};

log4cxx::LoggerPtr MyMessageHandler::mLogger(log4cxx::Logger::getLogger("MyMessageHandler"));


//////////////////////////////////////////////////////////////////////////
void ConnectorHandler(SharedPtr<RdmaConnection> connection, int err)
{
	printf("ConnectorHandler: err = %d\n", err);
}

void AcceptorHandler(SharedPtr<RdmaConnection> connection, int err)
{
	printf("AcceptorHandler: err = %d\n", err);
}

void EngineRunThreadProc(SharedPtr<ChannelEngine> engine)
{
	engine->run();
}

void CheckThreadProc(SharedPtr<ChannelEngine> engine)
{
	while(gReceiveCount < TEST_MESSAGE_COUNT)
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

	boost::program_options::options_description desc("ChannelEngineBasicTest");

    desc.add_options()
        ("help,h", "help message")
        ("config,c", boost::program_options::value<std::string>(), "node configuration file")
        ("local,l", boost::program_options::value<std::string>(), "local node name")
        ("target,t", boost::program_options::value<std::string>(), "target node name")
        ("port,p", boost::program_options::value<std::string>(), "daemon port");

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

        if(vm.count("config") == 0)
        {
        	throw std::invalid_argument("node configuration is not specified");
        }

        if(vm.count("local") == 0)
        {
        	throw std::invalid_argument("local node id is not specified");
        }

    	ScyllaConfiguration engineConfig;
    	{
			engineConfig.bufferManagerPoolSize = 32*1024*1024;
			engineConfig.dispatcherThreadCount = 4;
			engineConfig.receiverThreadCount = 1;
			engineConfig.localIdentifier = vm["local"].as<std::string>();
    	}

		gReceiveCount = 0;

    	SharedPtr<ScyllaNodeDB> engineNodeDB(new ScyllaNodeDB());
    	{
			std::string nodeConfig = vm["config"].as<std::string>();
			std::ifstream stream(nodeConfig.c_str());

			// parse the node configuration XML
			ticpp::Document* doc = new ticpp::Document(nodeConfig.c_str());
			doc->LoadFile();

			ticpp::Element* baseElem = doc->FirstChildElement("node-configuration");
			ticpp::Iterator<ticpp::Element> itNodes;
			for(itNodes = itNodes.begin(baseElem); itNodes != itNodes.end(); itNodes++)
			{
				UUID nodeId = itNodes->GetAttribute("uuid");
				ticpp::Iterator<ticpp::Element> itTransports;
				for(itTransports = itTransports.begin(itNodes.Get()); itTransports != itTransports.end(); itTransports++)
				{
					std::string bindAddr = itTransports->GetAttribute("bind");
					engineNodeDB->setLink(nodeId, bindAddr);
				}
			}
    	}

    	SharedPtr<ScyllaChannelEngine> engine(new ScyllaChannelEngine(engineConfig, engineNodeDB));
    	{
    		SharedPtr<MyMessageFactory> fact(new MyMessageFactory);
    		SharedPtr<MyMessageHandler> handler(new MyMessageHandler);
    		engine->registerMessageHandler(MyMessage::TYPE, fact, handler);
    	}

    	tbb::tbb_thread engineRun(boost::bind(EngineRunThreadProc, engine));
    	tbb::tbb_thread checker(boost::bind(CheckThreadProc, engine));

    	if(vm.count("target") > 0)
    	{
    		gPassiveMode = false;
    		UUID target = vm["target"].as<std::string>();
    		SharedPtr<Channel> channel;
    		try
    		{
    			channel = engine->createChannel(target);
    		}
    		catch(...)
    		{
    			fprintf(stderr, "Error: failed to connect to target\n");
    			return -1;
    		}

    		for(int i=0;i<TEST_MESSAGE_COUNT;++i)
    		{
				SharedPtr<MyMessage> message(new MyMessage);
				message->data = "HELLO!!!";
				channel->send(message);
    		}
    	}
    	else
    	{
    		gPassiveMode = true;
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
