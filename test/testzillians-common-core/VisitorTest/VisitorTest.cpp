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
 * @date Mar 3, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "core/Visitor.h"
#include <iostream>
#include <string>
#include <limits>
#include <tbb/tick_count.h>

#define BOOST_TEST_MODULE VisitorTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

class Shape : public VisitableBase<Shape> {
public:
	DEFINE_VISITABLE()
};

class Circle : public Shape {
public:
	DEFINE_VISITABLE()
};

class Renderer: public Visitor<const Shape, void>
{
public:
	Renderer()
	{
		visits(*this, boost::mpl::vector<Shape, Circle>(), DrawInvoker());
	}

	void draw(const Shape&)
	{
		printf("draw Shape\n");
	}

	void draw(const Circle&)
	{
		printf("draw Circle\n");
	}

	// other draw implementations

	typedef VISIT_INVOKER( draw ) DrawInvoker;
};

BOOST_AUTO_TEST_SUITE( VisitorTestSuite )

BOOST_AUTO_TEST_CASE( VisitorTestCase1 )
{
	Circle s;
	Renderer r;
	r(s);
}

BOOST_AUTO_TEST_SUITE_END()
