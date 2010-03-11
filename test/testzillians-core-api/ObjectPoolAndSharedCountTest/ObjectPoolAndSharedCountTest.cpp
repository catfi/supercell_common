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

#include "core-api/SharedCount.h"
#include "core-api/ObjectPool.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <stdio.h>

using namespace zillians;

class ManagedObject : public SharedCount, public ObjectPool<ManagedObject>
{
public:
	ManagedObject()
	{
		data = 0;
	}
	~ManagedObject()
	{
	}
	
public:
	int data;
};

#define ELEMENT_COUNT 10
int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();
	
	std::vector<ManagedObject*> objs;
	for(int i=0;i<ELEMENT_COUNT;++i)
	{
		ManagedObject *obj = new ManagedObject();
		objs.push_back(obj);
	}
	
	for(int i=0;i<ELEMENT_COUNT;++i)
	{
		ManagedObject *obj = objs[i];
		SAFE_RELEASE(obj);
	}
	
	ManagedObject::purge();
	
	return 0;
}
