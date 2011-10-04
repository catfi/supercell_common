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

#include "core/Prerequisite.h"
#include "core/ContextHub.h"
#include "core/ContextHubSerialization.h"
#include <iostream>
#include <string>
#include <limits>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#define BOOST_TEST_MODULE ContextHubSerializationTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( ContextHubSerializationTest )

struct Context1
{
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & data;
    }

    std::string data;
};

struct Context2
{
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & a;
    	ar & b;
    }

    int a;
    int b;
};

BOOST_AUTO_TEST_CASE( ContextHubSerializationTestCase1 )
{
	const char * testfile = "hello.txt";

	// serialize
	{
		ContextHub<ContextOwnership::transfer> hub;

		// prepare data
		Context1* ctx1 = new Context1;
		ctx1->data = "orz";
		hub.set<Context1>(ctx1);

		Context2* ctx2 = new Context2;
		ctx2->a = 0xFFFFABAB; ctx2->b = 0xAAAABBBB;
		hub.set<Context2>(ctx2);

		std::ofstream ofs(testfile);
		boost::archive::text_oarchive oa(ofs);

		ContextHubSerialization<boost::mpl::vector<Context1, Context2> > serializer(hub);
		oa << serializer;
	}

	// deserialize
	{
		ContextHub<ContextOwnership::transfer> hub;

		std::ifstream ifs(testfile);
		boost::archive::text_iarchive ia(ifs);

		ContextHubSerialization<boost::mpl::vector<Context1, Context2> > serializer(hub);
		ia >> serializer;

		Context1* ctx1 = hub.get<Context1>();
		BOOST_CHECK(ctx1 != NULL);
		if(ctx1)
		{
			BOOST_CHECK(ctx1->data == "orz");
		}

		Context2* ctx2 = hub.get<Context2>();
		BOOST_CHECK(ctx2 != NULL);
		if(ctx2)
		{
			BOOST_CHECK(ctx2->a == 0xFFFFABAB);
			BOOST_CHECK(ctx2->b == 0xAAAABBBB);
		}

	}
}

BOOST_AUTO_TEST_SUITE_END()
