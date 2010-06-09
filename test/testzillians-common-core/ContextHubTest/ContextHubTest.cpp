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
#include "core/ContextHub.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE ContextHubTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( ContextHubTest )

class A { };
class B { };
class C { };
class D { };
class E { };
class F { };

BOOST_AUTO_TEST_CASE( ContextHubTestCase1 )
{
	ContextHub<ContextOwnership::transfer> hub;

	BOOST_CHECK_NO_THROW(hub.set<A>(new A));
	BOOST_CHECK_NO_THROW(hub.set<B>(new B));
	BOOST_CHECK_NO_THROW(hub.set<C>(new C));
	BOOST_CHECK_NO_THROW(hub.set<D>(new D));
	BOOST_CHECK_NO_THROW(hub.set<E>(new E));
	BOOST_CHECK_NO_THROW(hub.set<F>(new F));

	A* a = NULL; BOOST_CHECK_NO_THROW(a = hub.get<A>()); BOOST_CHECK(a != NULL);
	B* b = NULL; BOOST_CHECK_NO_THROW(b = hub.get<B>()); BOOST_CHECK(b != NULL);
	C* c = NULL; BOOST_CHECK_NO_THROW(c = hub.get<C>()); BOOST_CHECK(c != NULL);
	D* d = NULL; BOOST_CHECK_NO_THROW(d = hub.get<D>()); BOOST_CHECK(d != NULL);
	E* e = NULL; BOOST_CHECK_NO_THROW(e = hub.get<E>()); BOOST_CHECK(e != NULL);
	F* f = NULL; BOOST_CHECK_NO_THROW(f = hub.get<F>()); BOOST_CHECK(f != NULL);
}

template<typename T>
class Counter
{
public:
	Counter(int& ctor_counter, int& dtor_counter) : ctor_counter_(ctor_counter), dtor_counter_(dtor_counter)
	{ ++ctor_counter_; }

	~Counter()
	{ ++dtor_counter_; }

public:
	int& ctor_counter_;
	int& dtor_counter_;
};

typedef Counter<A> CA;
typedef Counter<B> CB;
typedef Counter<C> CC;
typedef Counter<D> CD;
typedef Counter<E> CE;
typedef Counter<F> CF;

BOOST_AUTO_TEST_CASE( ContextHubTestCase2 )
{
	ContextHub<ContextOwnership::transfer> hub;

	int ctor_counter = 0;
	int dtor_counter = 0;

	BOOST_CHECK_NO_THROW(hub.set<CA>(new CA(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CB>(new CB(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CC>(new CC(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CD>(new CD(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CE>(new CE(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CF>(new CF(ctor_counter, dtor_counter)));

	BOOST_CHECK(ctor_counter == 6);
	BOOST_CHECK(dtor_counter == 0);

	CA* a = NULL; BOOST_CHECK_NO_THROW(a = hub.get<CA>()); BOOST_CHECK(a != NULL);
	CB* b = NULL; BOOST_CHECK_NO_THROW(b = hub.get<CB>()); BOOST_CHECK(b != NULL);
	CC* c = NULL; BOOST_CHECK_NO_THROW(c = hub.get<CC>()); BOOST_CHECK(c != NULL);
	CD* d = NULL; BOOST_CHECK_NO_THROW(d = hub.get<CD>()); BOOST_CHECK(d != NULL);
	CE* e = NULL; BOOST_CHECK_NO_THROW(e = hub.get<CE>()); BOOST_CHECK(e != NULL);
	CF* f = NULL; BOOST_CHECK_NO_THROW(f = hub.get<CF>()); BOOST_CHECK(f != NULL);

	BOOST_CHECK(ctor_counter == 6);
	BOOST_CHECK(dtor_counter == 0);

	BOOST_CHECK_NO_THROW(hub.set<CA>(new CA(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CB>(new CB(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CC>(new CC(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CD>(new CD(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CE>(new CE(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CF>(new CF(ctor_counter, dtor_counter)));

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 6);

	BOOST_CHECK_NO_THROW(hub.reset<CA>());
	BOOST_CHECK_NO_THROW(hub.reset<CB>());
	BOOST_CHECK_NO_THROW(hub.reset<CC>());
	BOOST_CHECK_NO_THROW(hub.reset<CD>());
	BOOST_CHECK_NO_THROW(hub.reset<CE>());
	BOOST_CHECK_NO_THROW(hub.reset<CF>());

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 12);
}

BOOST_AUTO_TEST_CASE( ContextHubTestCase3 )
{
	NamedContextHub<ContextOwnership::transfer> hub;

	int ctor_counter = 0;
	int dtor_counter = 0;

	CA* a = new CA(ctor_counter, dtor_counter);
	CB* b = new CB(ctor_counter, dtor_counter);
	CC* c = new CC(ctor_counter, dtor_counter);
	CD* d = new CD(ctor_counter, dtor_counter);
	CE* e = new CE(ctor_counter, dtor_counter);
	CF* f = new CF(ctor_counter, dtor_counter);

	BOOST_CHECK(ctor_counter == 6);
	BOOST_CHECK(dtor_counter == 0);

	hub.set(a, "CA");
	hub.set(b, "CB");
	hub.set(c, "CC");
	hub.set(d, "CD");
	hub.set(e, "CE");
	hub.set(f, "CF");

	CA* ta = NULL; BOOST_CHECK_NO_THROW(ta = hub.get<CA>("CA")); BOOST_CHECK(a == ta);
	CB* tb = NULL; BOOST_CHECK_NO_THROW(tb = hub.get<CB>("CB")); BOOST_CHECK(b == tb);
	CC* tc = NULL; BOOST_CHECK_NO_THROW(tc = hub.get<CC>("CC")); BOOST_CHECK(c == tc);
	CD* td = NULL; BOOST_CHECK_NO_THROW(td = hub.get<CD>("CD")); BOOST_CHECK(d == td);
	CE* te = NULL; BOOST_CHECK_NO_THROW(te = hub.get<CE>("CE")); BOOST_CHECK(e == te);
	CF* tf = NULL; BOOST_CHECK_NO_THROW(tf = hub.get<CF>("CF")); BOOST_CHECK(f == tf);

	BOOST_CHECK_NO_THROW(hub.set<CA>(new CA(ctor_counter, dtor_counter), "CA"));
	BOOST_CHECK_NO_THROW(hub.set<CB>(new CB(ctor_counter, dtor_counter), "CB"));
	BOOST_CHECK_NO_THROW(hub.set<CC>(new CC(ctor_counter, dtor_counter), "CC"));
	BOOST_CHECK_NO_THROW(hub.set<CD>(new CD(ctor_counter, dtor_counter), "CD"));
	BOOST_CHECK_NO_THROW(hub.set<CE>(new CE(ctor_counter, dtor_counter), "CE"));
	BOOST_CHECK_NO_THROW(hub.set<CF>(new CF(ctor_counter, dtor_counter), "CF"));

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 6);

	BOOST_CHECK_NO_THROW(hub.reset<CA>("CA"));
	BOOST_CHECK_NO_THROW(hub.reset<CB>("CB"));
	BOOST_CHECK_NO_THROW(hub.reset<CC>("CC"));
	BOOST_CHECK_NO_THROW(hub.reset<CD>("CD"));
	BOOST_CHECK_NO_THROW(hub.reset<CE>("CE"));
	BOOST_CHECK_NO_THROW(hub.reset<CF>("CF"));

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 12);
}

BOOST_AUTO_TEST_CASE( ContextHubTestCase4 )
{
	NamedContextHub<ContextOwnership::transfer> hub;

	int ctor_counter = 0;
	int dtor_counter = 0;

	CA* a = new CA(ctor_counter, dtor_counter);
	CB* b = new CB(ctor_counter, dtor_counter);
	CC* c = new CC(ctor_counter, dtor_counter);
	CD* d = new CD(ctor_counter, dtor_counter);
	CE* e = new CE(ctor_counter, dtor_counter);
	CF* f = new CF(ctor_counter, dtor_counter);

	BOOST_CHECK(ctor_counter == 6);
	BOOST_CHECK(dtor_counter == 0);

	hub.set(a);
	hub.set(b);
	hub.set(c);
	hub.set(d);
	hub.set(e);
	hub.set(f);

	CA* ta = NULL; BOOST_CHECK_NO_THROW(ta = hub.get<CA>()); BOOST_CHECK(a == ta);
	CB* tb = NULL; BOOST_CHECK_NO_THROW(tb = hub.get<CB>()); BOOST_CHECK(b == tb);
	CC* tc = NULL; BOOST_CHECK_NO_THROW(tc = hub.get<CC>()); BOOST_CHECK(c == tc);
	CD* td = NULL; BOOST_CHECK_NO_THROW(td = hub.get<CD>()); BOOST_CHECK(d == td);
	CE* te = NULL; BOOST_CHECK_NO_THROW(te = hub.get<CE>()); BOOST_CHECK(e == te);
	CF* tf = NULL; BOOST_CHECK_NO_THROW(tf = hub.get<CF>()); BOOST_CHECK(f == tf);

	BOOST_CHECK_NO_THROW(hub.set<CA>(new CA(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CB>(new CB(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CC>(new CC(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CD>(new CD(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CE>(new CE(ctor_counter, dtor_counter)));
	BOOST_CHECK_NO_THROW(hub.set<CF>(new CF(ctor_counter, dtor_counter)));

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 6);

	BOOST_CHECK_NO_THROW(hub.reset<CA>());
	BOOST_CHECK_NO_THROW(hub.reset<CB>());
	BOOST_CHECK_NO_THROW(hub.reset<CC>());
	BOOST_CHECK_NO_THROW(hub.reset<CD>());
	BOOST_CHECK_NO_THROW(hub.reset<CE>());
	BOOST_CHECK_NO_THROW(hub.reset<CF>());

	BOOST_CHECK(ctor_counter == 12);
	BOOST_CHECK(dtor_counter == 12);
}

BOOST_AUTO_TEST_SUITE_END()
