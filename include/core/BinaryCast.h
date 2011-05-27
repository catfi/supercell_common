/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date May 27, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_BINARYCAST_H_
#define ZILLIANS_BINARYCAST_H_

#include "core/Types.h"

namespace zillians {

namespace {

union __binary_cast_helper_union_1
{
	int8 i8;
	uint8 u8;
};

union __binary_cast_helper_union_2
{
	int16 i16;
	uint16 u16;
};

union __binary_cast_helper_union_4
{
	int32 i32;
	uint32 u32;
	float f32;
};

union __binary_cast_helper_union_8
{
	int64 i64;
	long long int ll64;
	uint64 u64;
	unsigned long long int ull64;
	double f64;
};

template<typename T, typename Intermediate>
struct converter;

template<>
struct converter<int8, __binary_cast_helper_union_1>
{
	static inline int8 get(const __binary_cast_helper_union_1& intermediate)
	{
		return intermediate.i8;
	}

	static inline void set(__binary_cast_helper_union_1& intermediate, const int8& value)
	{
		intermediate.i8 = value;
	}
};

template<>
struct converter<uint8, __binary_cast_helper_union_1>
{
	static inline uint8 get(const __binary_cast_helper_union_1& intermediate)
	{
		return intermediate.u8;
	}

	static inline void set(__binary_cast_helper_union_1& intermediate, const uint8& value)
	{
		intermediate.u8 = value;
	}
};

template<>
struct converter<int16, __binary_cast_helper_union_2>
{
	static inline int16 get(const __binary_cast_helper_union_2& intermediate)
	{
		return intermediate.i16;
	}

	static inline void set(__binary_cast_helper_union_2& intermediate, const int16& value)
	{
		intermediate.i16 = value;
	}
};

template<>
struct converter<uint16, __binary_cast_helper_union_2>
{
	static inline uint16 get(const __binary_cast_helper_union_2& intermediate)
	{
		return intermediate.u16;
	}

	static inline void set(__binary_cast_helper_union_2& intermediate, const uint16& value)
	{
		intermediate.u16 = value;
	}
};

template<>
struct converter<int32, __binary_cast_helper_union_4>
{
	static inline int32 get(const __binary_cast_helper_union_4& intermediate)
	{
		return intermediate.i32;
	}

	static inline void set(__binary_cast_helper_union_4& intermediate, const int32& value)
	{
		intermediate.i32 = value;
	}
};

template<>
struct converter<uint32, __binary_cast_helper_union_4>
{
	static inline uint32 get(const __binary_cast_helper_union_4& intermediate)
	{
		return intermediate.u32;
	}

	static inline void set(__binary_cast_helper_union_4& intermediate, const uint32& value)
	{
		intermediate.u32 = value;
	}
};

template<>
struct converter<float, __binary_cast_helper_union_4>
{
	static inline float get(const __binary_cast_helper_union_4& intermediate)
	{
		return intermediate.f32;
	}

	static inline void set(__binary_cast_helper_union_4& intermediate, const float& value)
	{
		intermediate.f32 = value;
	}
};

template<>
struct converter<int64, __binary_cast_helper_union_8>
{
	static inline int64 get(const __binary_cast_helper_union_8& intermediate)
	{
		return intermediate.i64;
	}

	static inline void set(__binary_cast_helper_union_8& intermediate, const int64& value)
	{
		intermediate.i64 = value;
	}
};

template<>
struct converter<uint64, __binary_cast_helper_union_8>
{
	static inline uint64 get(const __binary_cast_helper_union_8& intermediate)
	{
		return intermediate.u64;
	}

	static inline void set(__binary_cast_helper_union_8& intermediate, const uint64& value)
	{
		intermediate.u64 = value;
	}
};

template<>
struct converter<double, __binary_cast_helper_union_8>
{
	static inline double get(const __binary_cast_helper_union_8& intermediate)
	{
		return intermediate.f64;
	}

	static inline void set(__binary_cast_helper_union_8& intermediate, const double& value)
	{
		intermediate.f64 = value;
	}
};

template<>
struct converter<long long int, __binary_cast_helper_union_8>
{
	static inline long long int get(const __binary_cast_helper_union_8& intermediate)
	{
		return intermediate.ll64;
	}

	static inline void set(__binary_cast_helper_union_8& intermediate, const long long int& value)
	{
		intermediate.ll64 = value;
	}
};

template<>
struct converter<unsigned long long int, __binary_cast_helper_union_8>
{
	static inline unsigned long long int get(const __binary_cast_helper_union_8& intermediate)
	{
		return intermediate.ull64;
	}

	static inline void set(__binary_cast_helper_union_8& intermediate, const unsigned long long int& value)
	{
		intermediate.ull64 = value;
	}
};

template<typename T, typename U, std::size_t Size>
struct binary_cast_impl;

template<typename T, typename U>
struct binary_cast_impl<T,U,1>
{
	static T cast(const U& value)
	{
		__binary_cast_helper_union_1 intermediate;
		converter<U, __binary_cast_helper_union_1>::set(intermediate, value);
		return converter<T, __binary_cast_helper_union_1>::get(intermediate);
	}
};

template<typename T, typename U>
struct binary_cast_impl<T,U,2>
{
	static T cast(const U& value)
	{
		__binary_cast_helper_union_2 intermediate;
		converter<U, __binary_cast_helper_union_2>::set(intermediate, value);
		return converter<T, __binary_cast_helper_union_2>::get(intermediate);
	}
};

template<typename T, typename U>
struct binary_cast_impl<T,U,4>
{
	static T cast(const U& value)
	{
		__binary_cast_helper_union_4 intermediate;
		converter<U, __binary_cast_helper_union_4>::set(intermediate, value);
		return converter<T, __binary_cast_helper_union_4>::get(intermediate);
	}
};

template<typename T, typename U>
struct binary_cast_impl<T,U,8>
{
	static T cast(const U& value)
	{
		__binary_cast_helper_union_8 intermediate;
		converter<U, __binary_cast_helper_union_8>::set(intermediate, value);
		return converter<T, __binary_cast_helper_union_8>::get(intermediate);
	}
};

}

template<typename T, typename U>
T binary_cast(const U& value)
{
	return binary_cast_impl<T,U,sizeof(T)>::cast(value);
}

}

#endif /* ZILLIANS_BINARYCAST_H_ */
