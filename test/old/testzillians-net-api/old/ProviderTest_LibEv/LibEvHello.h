#ifndef LIBEVHELLO_H_
#define LIBEVHELLO_H_

#include "core/Types.h"
#include "core/SharedCount.h"
#include "core/ObjectPool.h"
#include "networking/Message.h"
#include "networking/MessageFactory.h"

#include <cstring>
#include <string>

using namespace zillians;

class HelloMessage : public Message, public ObjectPool<HelloMessage>
{
public:
	HelloMessage() { data = new char[255]; }
	virtual ~HelloMessage() { delete[] data; }

public:
	uint32 id;
	char *data;

public:
	static const int MESSAGE_TYPE = 1;
	
	virtual int32 type()
	{
		return MESSAGE_TYPE;
	}
	
	virtual size_t length()
	{
		return ByteBuffer::probeSize(id) + 
		       ByteBuffer::probeSize(data);
	}
	
	virtual int32 decode(ByteBuffer &bb, size_t length)
	{
		bb >> id;
		bb >> data;
		return ZN_OK;
	}
	
	virtual int32 encode(ByteBuffer &bb)
	{
		bb << id;
		bb << data;
		return ZN_OK;
	}
	
public:
	static long getTotalAllocationCount()
	{
		//return (long)mAllocationCount;
		return 0L;
	}
};

class HelloMessageFactory : public MessageFactory
{
public:
	virtual int32 create(int32 type, Message **message)
	{
		*message = new HelloMessage();
		//printf("create => total allocation count = %d\n", HelloMessage::getTotalAllocationCount());
	}
	
	virtual int32 destroy(int32 type, Message *message)
	{
		SAFE_RELEASE(message);
		//printf("destroy => total allocation count = %d\n", HelloMessage::getTotalAllocationCount());
	}
};


#endif /*LIBEVHELLO_H_*/
