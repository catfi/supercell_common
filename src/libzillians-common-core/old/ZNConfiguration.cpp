#include "ZNConfiguration.h"

//////////////////////////////////////////////////////////////////////////
ZNConfiguration::ZNConfiguration()
{
}

ZNConfiguration::~ZNConfiguration()
{
}


//////////////////////////////////////////////////////////////////////////
bool ZNConfiguration::setValue(string key, int    value)
{
	mIntMap[key] = value;
	return true;
}

bool ZNConfiguration::setValue(string key, long   value)
{
	mLongMap[key] = value;
	return true;
}

bool ZNConfiguration::setValue(string key, float  value)
{
	mFloatMap[key] = value;
	return true;
}

bool ZNConfiguration::setValue(string key, string value)
{
	mStringMap[key] = value;
	return true;
}

bool ZNConfiguration::setValue(string key, void*  value)
{
	mPtrMap[key] = value;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ZNConfiguration::getValue(string key, int*    value)
{
	return true;
}

bool ZNConfiguration::getValue(string key, long*   value)
{
	return true;
}

bool ZNConfiguration::getValue(string key, float*  value)
{
	return true;
}

bool ZNConfiguration::getValue(string key, string* value)
{
	return true;
}

bool ZNConfiguration::getValue(string key, void*   value)
{
	return true;
}
