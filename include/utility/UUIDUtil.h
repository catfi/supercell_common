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

#include "core/Common.h"
#include <apr_uuid.h>
#include <hash_set>

namespace zillians {

#define ENABLE_CASE_INSENSITIVE_UUID

struct UUID
{
public:
	UUID()
	{ invalidate(); }

	UUID(const UUID& id)
	{
#if __WORDSIZE == 32
		data.u32[0] = id.data.u32[0];
		data.u32[1] = id.data.u32[1];
		data.u32[2] = id.data.u32[2];
		data.u32[3] = id.data.u32[3];
#else
		data.u64[0] = id.data.u64[0];
		data.u64[1] = id.data.u64[1];
#endif
	}

	UUID(const char* text)
	{
		// parse the data
		apr_uuid_parse(&data.raw, text);
	}

	UUID(const std::string& text)
	{
		// parse the data
		apr_uuid_parse(&data.raw, text.c_str());
	}

	~UUID()
	{ }

public:
	inline bool valid() const
	{
#if __WORDSIZE == 32
		return  ( data.u32[0] != 0 ) ||
				( data.u32[1] != 0 ) ||
				( data.u32[2] != 0 ) ||
				( data.u32[3] != 0 );
#else
		return  ( data.u64[0] != 0 ) ||
				( data.u64[1] != 0 );
#endif
	}

	inline bool invalid() const
	{
#if __WORDSIZE == 32
		return  ( data.u32[0] == 0 ) &&
				( data.u32[1] == 0 ) &&
				( data.u32[2] == 0 ) &&
				( data.u32[3] == 0 );
#else
		return  ( data.u64[0] == 0 ) &&
				( data.u64[1] == 0 );
#endif
	}

public:
	inline static void random(UUID& id)
	{
		apr_uuid_get(&id.data.raw);
	}

	inline static void invalidate(UUID& id)
	{
#if __WORDSIZE == 32
		id.data.u32[0] = 0;
		id.data.u32[1] = 0;
		id.data.u32[2] = 0;
		id.data.u32[3] = 0;
#else
		id.data.u64[0] = 0;
		id.data.u64[1] = 0;
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
		data.u32[0] = id.data.u32[0];
		data.u32[1] = id.data.u32[1];
		data.u32[2] = id.data.u32[2];
		data.u32[3] = id.data.u32[3];
#else
		data.u64[0] = id.data.u64[0];
		data.u64[1] = id.data.u64[1];
#endif

		return *this;
	}
	inline UUID& operator = (const char* text)
	{
		// parse the data
		apr_uuid_parse(&data.raw, text);
		return *this;
	}
	inline UUID& operator = (const std::string& text)
	{
		// parse the data
		apr_uuid_parse(&data.raw, text.c_str());
		return *this;
	}
	inline bool operator == (const UUID& b) const
	{
#if __WORDSIZE == 32
		return  ( data.u32[0] == b.data.u32[0] ) &&
				( data.u32[1] == b.data.u32[1] ) &&
				( data.u32[2] == b.data.u32[2] ) &&
				( data.u32[3] == b.data.u32[3] );
#else
		return  ( data.u64[0] == b.data.u64[0] ) &&
				( data.u64[1] == b.data.u64[1] ) ;
#endif
	}
	inline bool operator == (const std::string& b) const
	{
		char temp[APR_UUID_FORMATTED_LENGTH+1];
		apr_uuid_format(temp, &data.raw);

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
		if( data.u32[0] == b.data.u32[0] )
		{
			if( data.u32[1] == b.data.u32[1] )
			{
				if( data.u32[2] == b.data.u32[2] )
				{
					if( data.u32[3] == b.data.u32[3] )
					{
						return false;
					}
					else
					{
						if( data.u32[3] < b.data.u32[3] )
							return true;
						else
							return false;
					}
				}
				else
				{
					if( data.u32[2] < b.data.u32[2] )
						return true;
					else
						return false;
				}
			}
			else
			{
				if( data.u32[1] < b.data.u32[1] )
					return true;
				else
					return false;
			}
		}
		else
		{
			if( data.u32[0] < b.data.u32[0] )
				return true;
			else
				return false;
		}
#else
		if( data.u64[0] == b.data.u64[0] )
		{
			if( data.u64[1] == b.data.u64[1] )
			{
				return false;
			}
			else
			{
				if( data.u64[1] < b.data.u64[1] )
					return true;
				else
					return false;
			}
		}
		else
		{
			if( data.u64[0] < b.data.u64[0] )
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
		apr_uuid_format(temp, &data.raw);

		std::string to_string= temp;
		return to_string;
	}

public:
	union
	{
		apr_uuid_t raw;
		uint64 u64[2];
		uint32 u32[4];
		uint16 u16[8];
	} data;
};

inline std::ostream& operator << (std::ostream &stream, const UUID& id)
{
	char temp[APR_UUID_FORMATTED_LENGTH+1];
	apr_uuid_format(temp, &id.data.raw);
	stream << temp;
	return stream;
}

inline std::istream& operator >> (std::istream& stream, const UUID& id)
{
	char temp[APR_UUID_FORMATTED_LENGTH+1];
	stream >> temp;
	apr_uuid_parse((apr_uuid_t*)&id.data.raw, temp);
	return stream;
}

struct UUIDHasher
{
    static size_t hash( const UUID& x )
    {
#if PLATFORM_BIT_WIDTH == 32
    	return x.data.u32[0] + x.data.u32[1] + x.data.u32[2] + x.data.u32[3];
#else
    	return x.data.u64[0] + x.data.u64[1];
#endif
    }
    static bool equal( const UUID& x, const UUID& y )
    {
        return (x == y);
    }
};

static UUID INVALID_UUID;

}

namespace boost {

extern std::size_t hash_value(const zillians::UUID& __x);

}

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
