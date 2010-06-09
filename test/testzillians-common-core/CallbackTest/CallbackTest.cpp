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

#include "core/Callback.h"
#include <stdio.h>

using namespace zillians;

class MyClass
{
public:
	void test_method0() { }
	void test_method0_const() { }
	void test_method1(int XD) { }
	void test_method1_const(int XD) { }
};
int main(int argc, char** argv)
{
	MyClass c;
	
	Callback0 cb0; 
	cb0.bind<MyClass, &MyClass::test_method0>(&c);
	cb0.invoke();
	cb0.bind<MyClass, &MyClass::test_method0_const>(&c);
	cb0.invoke();
	
	Callback1 cb1;
	cb1.bind<MyClass, int, &MyClass::test_method1>(&c, 12);
	cb1.invoke();
	cb1.bind<MyClass, int, &MyClass::test_method1_const>(&c, 12);
	cb1.invoke();
	
	return 0;
}
