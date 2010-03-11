#include "ZNAttrTable.h"

/////////////////////////////////////////////////////////////////////////////////////////
ZNAttrTable::ZNAttrTable(void)
{
}

ZNAttrTable::~ZNAttrTable(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void ZNAttrTable::set_attribute (int key, void *val)
{
	ACE_WRITE_GUARD (ACE_Thread_Mutex, mon, _mutex_attributes);
	_attributes[key] = val;
}

void ZNAttrTable::get_attribute (int key, void **val)
{
	ACE_READ_GUARD (ACE_Thread_Mutex, mon, _mutex_attributes);
	std::map<int, void*>::iterator it = _attributes.find(key);
	if(it != _attributes.end())
		*val = (void*)it->second;
}

void ZNAttrTable::del_attribute (int key)
{
	ACE_WRITE_GUARD (ACE_Thread_Mutex, mon, _mutex_attributes);
	std::map<int, void*>::iterator it = _attributes.find(key);
	if(it != _attributes.end())
		_attributes.erase(it);
}
void ZNAttrTable::clear_attributes()
{
	ACE_WRITE_GUARD (ACE_Thread_Mutex, mon, _mutex_attributes);
	_attributes.clear();
}
