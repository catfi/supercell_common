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
 * @date May 2, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "utility/TemplateTricks.h"
#include <tr1/unordered_set>

#define BOOST_TEST_MODULE TemplateTricksTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace zillians;

BOOST_AUTO_TEST_SUITE( TemplateTricksTestSuite )

HAS_MEMBER_FUNCTION(f, (void, (char)))

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase1 )
{
	struct dummy_t
	{
		void f() { }
	};
}

BOOST_AUTO_TEST_SUITE_END()
