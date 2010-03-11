#include "ZNRefCount.h"

ZNRefCount::INTERLOCKED_LONG ZNRefCount::_global_ref = 0L;

ZNRefCount::ZNRefCount(void)
{
	_ref = 0L;
	++_global_ref;
	AddRef();
}

ZNRefCount::~ZNRefCount(void)
{
	--_global_ref;
}

long ZNRefCount::AddRef()
{
	++_ref;
	return _ref.value();
}

long ZNRefCount::Release()
{
	if((--_ref) == 0L)
	{
		delete this;
		return 0L;
	}
	return _ref.value();
}
