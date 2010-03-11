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
 * @date May 1, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_UUID_H_
#define ZILLIANS_UUID_H_

#include "core-api/Common.h"
#include <apr_uuid.h>
#include <hash_set>

namespace zillians {

#define ENABLE_CASE_INSENSITIVE_UUID

class UUID : public apr_uuid_t
{
public:
	UUID()
	{ invalidate(); }
	UUID(const UUID& id)
	{
#if __WORDSIZE == 32
		*((uint32*)(&data[0])) = *((uint32*)(&id.data[0]));
		*((uint32*)(&data[4])) = *((uint32*)(&id.data[4]));
		*((uint32*)(&data[8])) = *((uint32*)(&id.data[8]));
		*((uint32*)(&data[12])) = *((uint32*)(&id.data[12]));
#else
		*((uint64*)(&data[0])) = *((uint64*)(&id.data[0]));
		*((uint64*)(&data[8])) = *((uint64*)(&id.data[8]));
#endif
	}
	UUID(const char* text)
	{
		// parse the data
		apr_uuid_parse((apr_uuid_t*)this, text);
	}
	UUID(const std::string& text)
	{
		// parse the data
		apr_uuid_parse((apr_uuid_t*)this, text.c_str());
	}
	~UUID()
	{ }

public:
	inline bool valid() const
	{
#if __WORDSIZE == 32
		return  ( *((uint32*)(&data[0])) != 0 ) ||
				( *((uint32*)(&data[4])) != 0 ) ||
				( *((uint32*)(&data[8])) != 0 ) ||
				( *((uint32*)(&data[12])) != 0 );
#else
		return  ( *((uint64*)(&data[0])) != 0 ) ||
				( *((uint64*)(&data[8])) != 0 );
#endif
	}
	inline bool invalid() const
	{
#if __WORDSIZE == 32
		return  ( *((uint32*)(&data[0])) == 0 ) &&
				( *((uint32*)(&data[4])) == 0 ) &&
				( *((uint32*)(&data[8])) == 0 ) &&
				( *((uint32*)(&data[12])) == 0 );
#else
		return  ( *((uint64*)(&data[0])) == 0 ) &&
				( *((uint64*)(&data[8])) == 0 );
#endif
	}
public:
	inline static void random(UUID& id)
	{
		apr_uuid_get((apr_uuid_t*)&id);
	}

	inline static void invalidate(UUID& id)
	{
#if __WORDSIZE == 32
		*((uint32*)(&id.data[0])) = 0;
		*((uint32*)(&id.data[4])) = 0;
		*((uint32*)(&id.data[8])) = 0;
		*((uint32*)(&id.data[12])) = 0;
#else
		*((uint64*)(&id.data[0])) = 0;
		*((uint64*)(&id.data[8])) = 0;
#endif
	}

	inline void invalidate()
	{
		UUID::invalidate(*this);
	}

	inline void random()
	{
		UUID::random(*this);
	}

public:
	inline UUID& operator = (const UUID& id)
	{
		// check for self-assignment
		if(this == &id)
			return *this;

		// copy the data
#if __WORDSIZE == 32
		*((uint32*)(&data[0])) = *((uint32*)(&id.data[0]));
		*((uint32*)(&data[4])) = *((uint32*)(&id.data[4]));
		*((uint32*)(&data[8])) = *((uint32*)(&id.data[8]));
		*((uint32*)(&data[12])) = *((uint32*)(&id.data[12]));
#else
		*((uint64*)(&data[0])) = *((uint64*)(&id.data[0]));
		*((uint64*)(&data[8])) = *((uint64*)(&id.data[8]));
#endif

		return *this;
	}
	inline UUID& operator = (const char* text)
	{
		// parse the data
		apr_uuid_parse((apr_uuid_t*)this, text);
		return *this;
	}
	inline UUID& operator = (const std::string& text)
	{
		// parse the data
		apr_uuid_parse((apr_uuid_t*)this, text.c_str());
		return *this;
	}
	inline bool operator == (const UUID& b) const
	{
#if __WORDSIZE == 32
		return  ( *((uint32*)(&data[0])) == *((uint32*)(&b.data[0])) ) &&
				( *((uint32*)(&data[4])) == *((uint32*)(&b.data[4])) ) &&
				( *((uint32*)(&data[8])) == *((uint32*)(&b.data[8])) ) &&
				( *((uint32*)(&data[12])) == *((uint32*)(&b.data[12])) );
#else
		return  ( *((uint64*)(&data[0])) == *((uint64*)(&b.data[0])) ) &&
				( *((uint64*)(&data[8])) == *((uint64*)(&b.data[8])) ) ;
#endif
	}
	inline bool operator == (const std::string& b) const
	{
		char temp[APR_UUID_FORMATTED_LENGTH+1];
		apr_uuid_format(temp, (apr_uuid_t*)this);

		// perform case insensitive comparison while comparing UUID to string
		for(int i=0;i<APR_UUID_FORMATTED_LENGTH;++i)
		{
#ifdef ENABLE_CASE_INSENSITIVE_UUID
			if(tolower(temp[i]) != tolower(b.c_str()[i]))
#else
			if(temp[i] != b.c_str()[i])
#endif
				return false;

		}

		return true;
	}
	inline bool operator != (const UUID& b) const
	{
		return  !((*this) == b);
	}
	inline bool operator < (const UUID& b) const
	{
		// TODO find a more efficient implementation here...
#if __WORDSIZE == 32
		if( *((uint32*)(&data[0])) == *((uint32*)(&b.data[0])) )
		{
			if( *((uint32*)(&data[4])) == *((uint32*)(&b.data[4])) )
			{
				if( *((uint32*)(&data[8])) == *((uint32*)(&b.data[8])) )
				{
					if( *((uint32*)(&data[12])) == *((uint32*)(&b.data[12])) )
					{
						return false;
					}
					else
					{
						if( *((uint32*)(&data[12])) < *((uint32*)(&b.data[12])) )
							return true;
						else
							return false;
					}
				}
				else
				{
					if( *((uint32*)(&data[8])) < *((uint32*)(&b.data[8])) )
						return true;
					else
						return false;
				}
			}
			else
			{
				if( *((uint32*)(&data[4])) < *((uint32*)(&b.data[4])) )
					return true;
				else
					return false;
			}
		}
		else
		{
			if( *((uint32*)(&data[0])) < *((uint32*)(&b.data[0])) )
				return true;
			else
				return false;
		}
#else
		if( *((uint64*)(&data[0])) == *((uint64*)(&b.data[0])) )
		{
			if( *((uint64*)(&data[8])) == *((uint64*)(&b.data[8])) )
			{
				return false;
			}
			else
			{
				if( *((uint64*)(&data[8])) < *((uint64*)(&b.data[8])) )
					return true;
				else
					return false;
			}
		}
		else
		{
			if( *((uint64*)(&data[0])) < *((uint64*)(&b.data[0])) )
				return true;
			else
				return false;
		}
#endif
	}
	inline bool operator > (const UUID& b) const
	{
		return  !((*this) < b);
	}
	inline bool operator <= (const UUID& b) const
	{
		return  ((*this) < b || (*this) == b);
	}
	inline bool operator >= (const UUID& b) const
	{
		return  ((*this) > b || (*this) == b);
	}

public:
	inline operator std::string() const
	{
		char temp[APR_UUID_FORMATTED_LENGTH+1];
		apr_uuid_format(temp, (apr_uuid_t*)this);

		std::string to_string= temp;
		return to_string;
	}

};

inline std::ostream& operator << (std::ostream &stream, const UUID& id)
{
	char temp[APR_UUID_FORMATTED_LENGTH+1];
	apr_uuid_format(temp, (apr_uuid_t*)&id);
	stream << temp;
	return stream;
}

inline std::istream& operator >> (std::istream& stream, const UUID& id)
{
	char temp[APR_UUID_FORMATTED_LENGTH+1];
	stream >> temp;
	apr_uuid_parse((apr_uuid_t*)&id, temp);
	return stream;
}

struct UUIDHasher
{
    static size_t hash( const UUID& x )
    {
#if PLATFORM_BIT_WIDTH == 32
    	return *((uint32*)(&x.data[0])) + *((uint32*)(&x.data[4])) + *((uint32*)(&x.data[8])) + *((uint32*)(&x.data[12]));
#else
    	return *((uint64*)(&x.data[0])) + *((uint64*)(&x.data[8]));
#endif
    }
    static bool equal( const UUID& x, const UUID& y )
    {
        return (x == y);
    }
};

static UUID INVALID_UUID;

}

extern std::size_t hash_value(const zillians::UUID& __x);

#ifdef __PLATFORM_LINUX__
namespace __gnu_cxx
{
	template<>
	struct hash<zillians::UUID>
	{
		size_t operator()(const zillians::UUID& __x) const
		{
			return zillians::UUIDHasher::hash(__x);
		}
	};
}
#else
namespace stdext
{
	template<>
	struct hash_compare<zillians::UUID>
	{
		size_t operator()(const zillians::UUID& __x) const
		{
			return zillians::UUIDHasher::hash(__x);
		}

		bool operator()(const zillians::UUID& v1, const zillians::UUID& v2) const
		{
			return (v1 == v2) ? true : false;
		}
	};
}
#endif

#endif/*ZILLIANS_UUID_H_*/
