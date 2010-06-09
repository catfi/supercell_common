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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "core/Types.h"
#include "core/SharedCount.h"
#include "core/ByteBuffer.h"
#include <boost/shared_ptr.hpp>

namespace zillians {

class Message : public SharedCount
{
public:
	Message() { }
	virtual ~Message() { }
	
public:
	virtual int32 type() = 0;
	virtual size_t length() = 0;
	virtual int32 decode(ByteBuffer &bb, size_t length) = 0;
	virtual int32 encode(ByteBuffer &bb) = 0;
};

}

#endif /*MESSAGE_H_*/
