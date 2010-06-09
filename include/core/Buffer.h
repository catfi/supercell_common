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

#ifndef ZILLIANS_BUFFER_H_
#define ZILLIANS_BUFFER_H_

#include "core/Common.h"
#include "core/ObjectPool.h"
#include "core/SharedPtr.h"
#include "utility/UUIDUtil.h"
#include "utility/BitTrickUtil.h"

#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/or.hpp>
#include <boost/system/error_code.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>

#ifndef ZILLIANS_BUFFER_DEFAULT_SIZE
#define ZILLIANS_BUFFER_DEFAULT_SIZE	128
#endif

namespace zillians {

namespace detail {

template<typename T>
struct is_std_vector
{
	enum { value = false };
};

template<typename T>
struct is_std_vector<std::vector<T> >
{
	enum { value = true };
};

template<typename T>
struct is_std_list
{
	enum { value = false };
};

template<typename T>
struct is_std_list<std::list<T> >
{
	enum { value = true };
};

template<typename T>
struct is_std_map
{
	enum { value = false };
};

template<typename K, typename V>
struct is_std_map< std::map<K,V> >
{
	enum { value = true };
};

template<typename T>
struct is_boost_array
{
	enum { value = false };
};

template<typename T, std::size_t N>
struct is_boost_array< boost::array<T,N> >
{
	enum { value = true };
};

template<typename T>
struct is_std_string
{
	enum { value = false };
};

template<typename _CharT, typename _Traits, typename _Alloc>
struct is_std_string< std::basic_string<_CharT, _Traits, _Alloc> >
{
	enum { value = true };
};

}

/**
 * @brief BufferContext allows user structure to be associated with (or
 * super-imposed on) Buffer object.
 */
typedef shared_ptr<void> BufferContext;

/**
 * @brief Buffer is a generic buffer implementation with super-imposed context
 * object that allows other structure to be related to Buffer class.
 *
 * Buffer provides a very high performance implementation of generic
 * buffered array. We can read from or write to buffers through various
 * methods it provides or simply use the overloaded << or >> operator to
 * append or extract data from the internal data array. Buffer supports
 * context object to let to impose some relationship among buffer classes.
 */
class Buffer : public ConcurrentObjectPool<Buffer>
{
	template <typename T>
	struct is_buffer
	{
		enum { value = boost::is_same<T, Buffer>::value };
	};

	/**
	 * @brief Helper class to identify whether a given type is variable length
	 */
	template <typename T>
	struct is_variable_types
	{
		enum { value =
			boost::is_same<T, std::string>::value ||
			boost::is_same<T, char*>::value ||
			detail::is_std_vector<T>::value ||
			detail::is_std_list<T>::value ||
			detail::is_std_map<T>::value ||
			boost::is_same<T, Buffer>::value
			};
	};

	/**
	 * @brief Helper class to identify whether a given type has built-in
	 * read/write implementation.
	 *
	 * @note To add a new built-in type (that is, readBuiltin(X) is provided),
	 * be sure to append that type into this list.
	 */
	template <typename T>
	struct is_builtin_types
	{
		enum { value =
			boost::is_same<typename boost::remove_const<T>::type, bool>::value ||
			boost::is_same<typename boost::remove_const<T>::type, char>::value ||
			boost::is_same<typename boost::remove_const<T>::type, short>::value ||
			boost::is_same<typename boost::remove_const<T>::type, int>::value ||
			boost::is_same<typename boost::remove_const<T>::type, long>::value ||
			boost::is_same<typename boost::remove_const<T>::type, long long>::value ||
			boost::is_same<typename boost::remove_const<T>::type, unsigned char>::value ||
			boost::is_same<typename boost::remove_const<T>::type, unsigned short>::value ||
			boost::is_same<typename boost::remove_const<T>::type, unsigned int>::value ||
			boost::is_same<typename boost::remove_const<T>::type, unsigned long>::value ||
			boost::is_same<typename boost::remove_const<T>::type, unsigned long long>::value ||
			boost::is_same<typename boost::remove_const<T>::type, int8>::value ||
			boost::is_same<typename boost::remove_const<T>::type, uint8>::value ||
			boost::is_same<typename boost::remove_const<T>::type, int16>::value ||
			boost::is_same<typename boost::remove_const<T>::type, uint16>::value ||
			boost::is_same<typename boost::remove_const<T>::type, int32>::value ||
			boost::is_same<typename boost::remove_const<T>::type, uint32>::value ||
			boost::is_same<typename boost::remove_const<T>::type, int64>::value ||
			boost::is_same<typename boost::remove_const<T>::type, uint64>::value ||
			boost::is_same<typename boost::remove_const<T>::type, std::size_t>::value ||
			boost::is_same<typename boost::remove_const<T>::type, float>::value ||
			boost::is_same<typename boost::remove_const<T>::type, double>::value ||
			boost::is_same<typename boost::remove_const<T>::type, std::string>::value ||
			boost::is_same<typename boost::remove_const<T>::type, UUID>::value ||
			boost::is_same<T, char*>::value ||
			boost::is_same<T, const char*>::value ||
			boost::is_array<T>::value ||
			detail::is_std_vector<T>::value ||
			detail::is_std_list<T>::value ||
			detail::is_std_map<T>::value ||
			detail::is_boost_array<T>::value ||
			boost::is_same<typename boost::remove_const<T>::type, boost::system::error_code>::value ||
			boost::is_same<typename boost::remove_const<T>::type, Buffer>::value
			};
	};

	/**
	 * @brief Helper class to identify whether a given type can be directly
	 * read/write by memcpy.
	 *
	 * @note To add a new direct type (that is, array of direct type can be
	 * read/write through memcpy), be sure to append that type into this list.
	 */
	template <typename T>
	struct is_direct_types
	{
		enum { value =
			//boost::is_same<T, bool>::value || // because bool is treated as int8 in Buffer, so it's not a direct type
			boost::is_same<T, char>::value ||
			boost::is_same<T, short>::value ||
			boost::is_same<T, int>::value ||
			boost::is_same<T, long>::value ||
			boost::is_same<T, long long>::value ||
			boost::is_same<T, unsigned char>::value ||
			boost::is_same<T, unsigned short>::value ||
			boost::is_same<T, unsigned int>::value ||
			boost::is_same<T, unsigned long>::value ||
			boost::is_same<T, unsigned long long>::value ||
			boost::is_same<T, int8>::value ||
			boost::is_same<T, uint8>::value ||
			boost::is_same<T, int16>::value ||
			boost::is_same<T, uint16>::value ||
			boost::is_same<T, int32>::value ||
			boost::is_same<T, uint32>::value ||
			boost::is_same<T, int64>::value ||
			boost::is_same<T, uint64>::value ||
			boost::is_same<T, float>::value ||
			boost::is_same<T, double>::value ||
			boost::is_same<T, UUID>::value ||
			boost::is_same<T, char*>::value ||
			boost::is_same<T, std::size_t>::value
			};
	};

public:
	Buffer()
	{
		mOwner = true; mReadOnly = false; mOnDemand = true;
		mData = NULL;
		mAllocatedSize = 0;
		mReadPos = mWritePos = 0;
		mReadPosMarked = mWritePosMarked = 0;

	}

	/**
	 * Construct a Buffer object with specific size.
	 *
	 * @note the internal data is allocated by the Buffer object and will
	 * be freed in the Buffer's destructor.
	 *
	 * @param size The internal data size.
	 */
	Buffer(std::size_t size)
	{
		BOOST_ASSERT(size > 0);
		mOwner = true; mReadOnly = false; mOnDemand = false;
		mData = (byte*)malloc(size);
		mAllocatedSize = size;
		mReadPos = mWritePos = 0;
		mReadPosMarked = mWritePosMarked = 0;
	}

	/**
	 * @brief Construct a Buffer object with given data pointer and size.
	 *
	 * @note the internal data is owned by the caller and will NOT
	 * be freed in the Buffer's destructor.
	 *
	 * @param data The internal data pointer.
	 * @param size The internal data size.
	 */
	Buffer(byte* data, std::size_t size)
	{
		mOwner = false; mReadOnly = false; mOnDemand = false;
		mData = data;
		mAllocatedSize = size;
		mReadPos = mWritePos = 0;
		mReadPosMarked = mWritePosMarked = 0;
	}

	/**
	 * @brief Consutrct a read-only Buffer object with given const data pointer and size.
	 *
	 * @note the internal data is owned by the caller and will NOT
	 * be freed in the Buffer's destructor.
	 *
	 * @note the Buffer is not mutable, you can only read from the Buffer
	 * object. Any write operation will fail and throw out exception.
	 *
	 * @param data The internal data pointer.
	 * @param size The internal data size.
	 * @return
	 */
	Buffer(const byte* data, std::size_t size)
	{
		mOwner = false; mReadOnly = true; mOnDemand = false;
		mData = (byte*)data;
		mAllocatedSize = size;
		mReadPos = mWritePos = 0;
		mReadPosMarked = mWritePosMarked = 0;
	}

	/**
	 * @brief Consutrct a clone of given buffer
	 *
	 * @param buffer The buffer to be cloned
	 * @return
	 */
	Buffer(const Buffer& buffer)
	{
		if(buffer.mOwner)
		{
			mOwner = true;
			mReadOnly = false;
			mOnDemand = buffer.mOnDemand;
			mData = (byte*)malloc(buffer.mAllocatedSize);
			mAllocatedSize = buffer.mAllocatedSize;
			mReadPos = buffer.mReadPos;
			mWritePos = buffer.mWritePos;
			mReadPosMarked = buffer.mReadPosMarked;
			mWritePosMarked = buffer.mWritePosMarked;

			::memcpy(mData, buffer.mData, buffer.mAllocatedSize);
		}
		else
		{
			mOwner = false;
			mReadOnly = buffer.mReadOnly;
			mOnDemand = buffer.mOnDemand;
			mData = buffer.mData;
			mAllocatedSize = buffer.mAllocatedSize;
			mReadPos = buffer.mReadPos;
			mWritePos = buffer.mWritePos;
			mReadPosMarked = buffer.mReadPosMarked;
			mWritePosMarked = buffer.mWritePosMarked;
		}
	}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
	Buffer(Buffer&& buffer)
	{
		mOwner = buffer.mOwner;
		mReadOnly = buffer.mReadOnly;
		mOnDemand = buffer.mOnDemand;
		mData = buffer.mData;
		mAllocatedSize = buffer.mAllocatedSize;
		mReadPos = buffer.mReadPos;
		mWritePos = buffer.mWritePos;
		mReadPosMarked = buffer.mReadPosMarked;
		mWritePosMarked = buffer.mWritePosMarked;

		buffer.mOwner = false;
		buffer.mReadOnly = false;
		buffer.mOnDemand = false;
		buffer.mData = NULL;
		buffer.mAllocatedSize = 0;
		buffer.mReadPos = 0;
		buffer.mWritePos = 0;
		buffer.mReadPosMarked = 0;
		buffer.mWritePosMarked = 0;
	}
#endif

	/**
	 * @brief Destruct the Buffer object.
	 */
	~Buffer()
	{
		if(mOwner && mData)
		{
			free((void*)mData); mData = NULL;
		}
	}

public:
	Buffer& operator = (const Buffer& buffer)
	{
		if(mOwner && mData)
		{
			free((void*)mData); mData = NULL;
		}

		if(buffer.mOwner)
		{
			mOwner = true;
			mReadOnly = false;
			mData = (byte*)malloc(buffer.mAllocatedSize);
			mAllocatedSize = buffer.mAllocatedSize;
			mReadPos = buffer.mReadPos;
			mWritePos = buffer.mWritePos;
			mReadPosMarked = buffer.mReadPosMarked;
			mWritePosMarked = buffer.mWritePosMarked;

			::memcpy(mData, buffer.mData, buffer.mAllocatedSize);
		}
		else
		{
			mOwner = false;
			mReadOnly = buffer.mReadOnly;
			mData = buffer.mData;
			mAllocatedSize = buffer.mAllocatedSize;
			mReadPos = buffer.mReadPos;
			mWritePos = buffer.mWritePos;
			mReadPosMarked = buffer.mReadPosMarked;
			mWritePosMarked = buffer.mWritePosMarked;
		}

		return *this;
	}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
	Buffer& operator = (Buffer&& buffer)
	{
		if(this == &buffer)
			return *this;

		if(mOwner && mData)
		{
			free((void*)mData); mData = NULL;
		}

		mOwner = buffer.mOwner;
		mReadOnly = buffer.mReadOnly;
		mOnDemand = buffer.mOnDemand;
		mData = buffer.mData;
		mAllocatedSize = buffer.mAllocatedSize;
		mReadPos = buffer.mReadPos;
		mWritePos = buffer.mWritePos;
		mReadPosMarked = buffer.mReadPosMarked;
		mWritePosMarked = buffer.mWritePosMarked;

		buffer.mOwner = false;
		buffer.mReadOnly = false;
		buffer.mOnDemand = false;
		buffer.mData = NULL;
		buffer.mAllocatedSize = 0;
		buffer.mReadPos = 0;
		buffer.mWritePos = 0;
		buffer.mReadPosMarked = 0;
		buffer.mWritePosMarked = 0;

		return *this;
	}
#endif

public:
	/**
	 * @brief Probe the actual data size of a given type.
	 *
	 * @return The actual size stored in Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSize()
	{
		if(UNLIKELY(typeid(T) == typeid(bool)))
			return sizeof(int8);
		else
			return sizeof(T);
	}

	/**
	 * @brief Probe the actual data size of a given variable.
	 *
	 * @param value The variable to probe its actual data size.
	 * @return The actual data size of the variable stored in the Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSize(const T &value)
	{
		return probeSizeDispatch<T>(value, boost::mpl::bool_<is_builtin_types<T>::value>());
	}

//private:
	template <typename T>
	inline static std::size_t probeSizeDispatch(const T &value, boost::mpl::true_ /* is_builtin_type */)
	{
		return probeSizeBuiltin(value);
	}

	template <typename T>
	inline static std::size_t probeSizeDispatch(const T &value, boost::mpl::false_ /* is_builtin_type */)
	{
		return probeSizeSerializable(value);
	}

	/**
	 * @brief Probe the actual data size of a given std::vector.
	 *
	 * @param value The std::vector to probe its actual data size.
	 * @return The actual data size of the std::vector stored in the Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSizeBuiltin(const std::vector<T> &value)
	{
		std::size_t size = sizeof(uint32);
		size += probeSizeBuiltinForVector<T>(value, boost::mpl::bool_<is_variable_types<T>::value>());
		return size;
	}

	template <typename T>
	inline static std::size_t probeSizeBuiltinForVector(const std::vector<T> &value, boost::mpl::true_ /* is_variable_type */)
	{
		std::size_t size = 0;
		for(typename std::vector<T>::const_iterator i = value.begin(); i != value.end(); ++i)
		{
			size += probeSize(*i);
		}
		return size;
	}

	template <typename T>
	inline static std::size_t probeSizeBuiltinForVector(const std::vector<T> &value, boost::mpl::false_ /* is_variable_type */)
	{
		if(value.size() == 0)
			return 0;
		else
			return value.size() * probeSize(*value.begin());
	}

	/**
	 * @brief Probe the actual data size of a given std::list.
	 *
	 * @param value The std::list to probe its actual data size.
	 * @return The actual data size of the std::list stored in the Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSizeBuiltin(const std::list<T> &value)
	{
		std::size_t size = sizeof(uint32);
		size += probeSizeBuiltinForList<T>(value, boost::mpl::bool_<is_variable_types<T>::value>());
		return size;
	}

	template <typename T>
	inline static std::size_t probeSizeBuiltinForList(const std::list<T> &value, boost::mpl::true_ /* is_variable_type */)
	{
		std::size_t size = 0;
		for(typename std::list<T>::const_iterator i = value.begin(); i != value.end(); ++i)
		{
			size += probeSize(*i);
		}
		return size;
	}

	template <typename T>
	inline static std::size_t probeSizeBuiltinForList(const std::list<T> &value, boost::mpl::false_ /* is_variable_type */)
	{
		if(value.size() == 0)
			return 0;
		else
			return value.size() * probeSize(*value.begin());
	}

	/**
	 * @brief Probe the actual data size of a given std::map.
	 *
	 * @param value The std::map to probe its actual data size.
	 * @return The actual data size of the std::map stored in the Buffer object.
	 */
	template <typename K, typename V>
	inline static std::size_t probeSizeBuiltin(const std::map<K,V> &value)
	{
		std::size_t size = sizeof(uint32);
		size += probeSizeBuiltinForMap<K,V>(value, boost::mpl::or_< boost::mpl::bool_<is_variable_types<K>::value>, boost::mpl::bool_<is_variable_types<V>::value> >());
		return size;
	}

	template <typename K, typename V>
	inline static std::size_t probeSizeBuiltinForMap(const std::map<K,V> &value, boost::mpl::true_ /* is_variable_type */)
	{
		std::size_t size = 0;
		for(typename std::map<K,V>::const_iterator i = value.begin(); i != value.end(); ++i)
		{
			size += probeSize(i->first);
			size += probeSize(i->second);
		}
		return size;
	}

	template <typename K, typename V>
	inline static std::size_t probeSizeBuiltinForMap(const std::map<K,V> &value, boost::mpl::false_ /* is_variable_type */)
	{
		if(value.size() == 0)
			return 0;
		else
			return value.size() * (probeSize(value.begin()->first) + probeSize(value.begin()->second));
	}

	/**
	 * @brief Probe the actual data size of a given boost::array.
	 *
	 * @param value The boost::array to probe its actual data size.
	 * @return The actual data size of the boost::array stored in the Buffer object.
	 */
	template <typename T, std::size_t N>
	inline static std::size_t probeSizeBuiltin(const boost::array<T,N> &value)
	{
		return N * sizeof(T) + sizeof(uint32);
	}

	/**
	 * @brief Probe the actual data size of a given boolean variable.
	 *
	 * @note Note that we store boolean variables in just 1 byte.
	 *
	 * @param value The boolean variable to probe its actual data size.
	 * @return The actual data size of the boolean variable stored in the Buffer object.
	 */
	inline static std::size_t probeSizeBuiltin(const bool& value)
	{
		return sizeof(int8);
	}

	/**
	 * @brief Probe the actual data size of a given const array of data.
	 *
	 * @note TODO Note that we use strlen() to identify the length of the buffer, which might
	 * lead to some performance degrade and raise some security concern. Consider to use
	 * some other way to count the length of the given buffer.
	 *
	 * @param value The const data pointer variable to probe its actual data size.
	 * @return The actual data size of the const data pointer variable stored in the Buffer object.
	 */
	inline static std::size_t probeSizeBuiltin(const char* value)
	{
		return strlen(value) + sizeof(uint32);
	}

	/**
	 * @brief Probe the actual data size of a given const std::string.
	 *
	 * @param value The const std::string reference to probe its actual data size.
	 * @return The actual data size of the const std::string reference stored in the Buffer object.
	 */
	inline static std::size_t probeSizeBuiltin(const std::string &value)
	{
		return value.length() + sizeof(uint32);
	}

	/**
	 * @brief Probe the actual data size of a given const Buffer object.
	 *
	 * @param value The const Buffer reference to probe its actual data size.
	 * @return The actual data size of the const Buffer reference stored in the Buffer object.
	 */
	inline static std::size_t probeSizeBuiltin(const Buffer &value)
	{
		return value.dataSize() + sizeof(uint32);
	}

	/**
	 * @brief Probe the actual data size of a given boost::system::error variable.
	 *
	 * @note Note that we store boolean variables in just 1 byte.
	 *
	 * @param value The boost::system::error variable to probe its actual data size.
	 * @return The actual data size of the boost::system::error variable stored in the Buffer object.
	 */
	inline static std::size_t probeSizeBuiltin(const boost::system::error_code& value)
	{
		return sizeof(int32) + sizeof(int32);
	}

	/**
	 * @brief Probe the actual data size of a any given type of data
	 *
	 * @note This templated method takes care of the rest of built-in types which are
	 * believed to be sized to sizeof(T)
	 *
	 * @param value The const variable to probe.
	 * @return The actual data size of the const variable stored in the Buffer object.
	 */
	template<typename T>
	inline static std::size_t probeSizeBuiltin(const T& value)
	{
		return sizeof(T);
	}

public:
	/**
	 * @brief Helper class to probe the size of a serializable data structure.
	 */
	struct SizeProbingArchive
	{
		SizeProbingArchive() : size(0) { }

		template <typename T>
		void operator & (T& v)
		{
			size += Buffer::probeSize(v);
		}

		std::size_t size;
	};

	/**
	 * @brief Probe the actual data size if a serializable data structure.
	 *
	 * @note A serializable data structure is any class or struct which
	 * implements:
	 * @code
	 * 		class SomeSerializableObject
	 * 		{
	 * 			...
	 * 			template <typename Archive>
	 * 			void serialize(Archive& ar, const unsigned int version)
	 * 			{
	 * 				ar & some_variable_1;
	 * 				ar & some_variable_2;
	 * 			}
	 * 			...
	 * 			int32 some_variable_1;
	 * 			std::string some_variable_2;
	 * 		};
	 * @endcode
	 *
	 * @param t The non-const reference to the given serializable data structure.
	 * @return The actual data size of the given serializable data structure stored in the Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSizeSerializable(const T& t)
	{
		T& value = *(const_cast<T*>(&t));
		SizeProbingArchive ar;
		value.serialize(ar, /*UNUSED*/ 0);
		return ar.size;
	}

public:
	/**
	 * @brief Reset the read and write pointer of the Buffer object.
	 *
	 * @note Note that the actual data is not altered but just the pointers reset.
	 */
	inline void clear()
	{
		mReadPos = mWritePos = 0;
		mReadPosMarked = mWritePosMarked = 0;
	}

	/**
	 * @brief Clear all data from current write pointer to the end of the internal data buffer.
	 */
	inline void zero()
	{
		::memset(mData + wpos(), 0, freeSize());
	}

	/**
	 * @brief Move all data enclosed by current read pointer and write pointer to the beginning
	 * of the internal data buffer and reset the read and write pointer afterwards.
	 */
	inline void crunch()
	{
		if(dataSize() > 0 && rpos() > 0)
		{
			std::size_t size = dataSize();
			if(LIKELY(size > 0))
			{
				::memmove(mData, mData + rpos(), size);
			}
			wpos(size);
			rpos(0);
		}
	}

	/**
	 * @brief Sets this buffer's mark at its position.
	 * @note Previously-marked position is overwritten.
	 */
	inline void mark()
	{
		mReadPosMarked = mReadPos;
		mWritePosMarked = mWritePos;
	}

	/**
	 * @brief Resets this buffer's position to the previously-marked position.
	 * @note The default marked position is zero.
	 */
	inline void reset()
	{
		mReadPos = mReadPosMarked;
		mWritePos = mWritePosMarked;
		mReadPosMarked = mWritePosMarked = 0;
	}

	/**
	 * @brief Get the allocated size (the internal data buffer size).
	 *
	 * @return The internal allocated buffer size.
	 */
	inline std::size_t allocatedSize() const
	{
		return mAllocatedSize;
	}

	/**
	 * @brief Get the current free buffer size.
	 *
	 * @note Free buffer segment starts from current write pointer to the end of the
	 * internal buffer.
	 *
	 * @return The free buffer size.
	 */
	inline std::size_t freeSize() const
	{
		return mAllocatedSize - wpos();
	}

	/**
	 * @brief Get the current data size.
	 *
	 * @note Data buffer segment starts from the current read pointer to the current
	 * write pointer.
	 *
	 * @return The current data size.
	 */
	inline std::size_t dataSize() const
	{
		return wpos() - rpos();
	}

public:
	/**
	 * @brief Get the current read pointer position.
	 *
	 * @return The current read pointer position.
	 */
	inline std::size_t rpos() const { return mReadPos; }

	/**
	 * @brief Get the current write pointer position.
	 *
	 * @return The current write pointer position.
	 */
	inline std::size_t wpos() const { return mWritePos; }

	/**
	 * @brief Manually set the current read pointer position.
	 *
	 * @param position The absolute read pointer position.
	 * @return The updated current read pointer position.
	 */
	inline std::size_t rpos(std::size_t position) { mReadPos = position; return mReadPos; }

	/**
	 * @brief Manually set the current write pointer position.
	 *
	 * @param position The absolute write pointer position.
	 * @return The updated current write pointer position.
	 */
	inline std::size_t wpos(std::size_t position) { mWritePos = position; return mWritePos; }

	/**
	 * @brief Get the current read pointer.
	 *
	 * @note You may use the raw read pointer to alter the content of the buffer, but
	 * remember to use with caution.
	 *
	 * @return The current read pointer.
	 */
	inline byte* rptr() const { return (byte*)(mData + mReadPos); }

	/**
	 * @brief Get the current write pointer.
	 *
	 * @note You may use the raw write pointer to alter the content of the buffer, but
	 * remember to use with caution.
	 *
	 * @return The current write pointer.
	 */
	inline byte* wptr() const { return (byte*)(mData + mWritePos); }

	/**
	 * @brief Move forward the current read pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to skip reading.
	 */
	inline void rskip(std::size_t bytes) { mReadPos += bytes; }

	/**
	 * @brief Move forward the current write pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to skip writing.
	 */
	inline void wskip(std::size_t bytes) { mWritePos += bytes; }

	/**
	 * @brief Move backward the current read pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to move backward.
	 */
	inline void rrev(std::size_t bytes) { mReadPos -= bytes; }

	/**
	 * @brief Move backward the current write pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to move backward.
	 */
	inline void wrev(std::size_t bytes) { mWritePos -= bytes; }

	inline byte* baseptr() const { return (byte*)mData; }

public:
	/**
	 * @brief Read an arbitrary variable.
	 *
	 * @note read() is an templated function which dispatch to real
	 * read implementation based on the given variable type.
	 *
	 * @param value The value to be read.
	 */
	template <typename T>
	inline void read(T& value)
	{
		readDispatch(value, boost::mpl::bool_< is_builtin_types<T>::value >() );
	}

//private:
	/**
	 * @brief Helper function to call readBuiltin() due to the lack of partial specialization of function template.
	 *
	 * @note Never call this directly.
	 *
	 * @param value The value to be read.
	 */
	template <typename T>
	inline void readDispatch(T& value, boost::mpl::true_ /*is_builtin_types*/)
	{
		readBuiltin(value);
	}

	/**
	 * @brief Helper function to call readBuiltin() due to the lack of partial specialization of function template.
	 *
	 * @note Never call this directly.
	 *
	 * @param value The value to be read.
	 */
	template <typename T>
	inline void readDispatch(T& value, boost::mpl::false_ /*is_builtin_types*/)
	{
		readSerializable(value);
	}

	/**
	 * @brief Read the raw data into any data structure.
	 *
	 * @param value Any data structure.
	 */
	template <typename T>
	inline void readAny(T& value)
	{
		readArray((char*)&value, sizeof(T));
	}

	/**
	 * @brief The helper archive class to read serializable data structure.
	 */
	struct ReadProxyArchive
	{
		ReadProxyArchive(Buffer& buffer) : mBuffer(buffer) { }

		template <typename T>
		inline void operator & (T& v)
		{
			mBuffer.read(v);
		}

		inline void skip(std::size_t size)
		{
			mBuffer.rskip(size);
		}

		Buffer& buffer() { return mBuffer; }

		Buffer& mBuffer;
	};

	/**
	 * @brief Read any serializable data structure.
	 *
	 * @param value The reference to the serializable data structure to be read.
	 */
	template <typename T>
	inline void readSerializable(T& value)
	{
		ReadProxyArchive ar(*this);
		value.serialize(ar, /*UNUSED*/ 0 );
	}

	/**
	 * @brief Read an int8 (1 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(int8& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an uint8 (1 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(uint8& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an int16 (2 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(int16& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an uint16 (2 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(uint16& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an int32 (4 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(int32& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an uint32 (4 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(uint32& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an int64 (8 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(int64& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read an uint64 (8 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(uint64& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read a float (4 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(float& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read a double (8 byte) variable.
	 *
	 * @param value The value to be read
	 */
	inline void readBuiltin(double& value)
	{
		readDirect(value);
	}

#ifdef __PLATFORM_WINDOWS__
	/**
	 * @brief Read a std::size_t (8 byte or 4 byte depend on the platform) variable.
	 *
	 * @param value The value to be read
	 */
	inline void readBuiltin(std::size_t& value)
	{
		readDirect(value);
	}
#endif

	/**
	 * @brief Read a boolean (treated as uint8) (1 byte) variable.
	 *
	 * @param value The value to be read.
	 */
	inline void readBuiltin(bool& value)
	{
		uint8 x; read(x);
		value = (x > 0) ? true : false;
	}

	/**
	 * @brief Read a std::string variable.
	 *
	 * @param value The value to be read
	 */
	inline void readBuiltin(std::string& value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length <= MAX_STRING_LENGTH))
		{
			value.clear();
			value.append((char*)rptr(), length);
		}
		else
		{
			BOOST_ASSERT(length <= MAX_STRING_LENGTH);
		}

		rskip(length);
	}

	/**
	 * @brief Read an array of data.
	 *
	 * @note The method is pretty dangerous because we don't know the size of value before hand,
	 * so it's better to avoid using this method. Use std::vector<char> or boost::array<> instead.
	 *
	 * @param value The array to be read.
	 */
	inline void readBuiltin(char* value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length <= MAX_STRING_LENGTH))
		{
			BOOST_ASSERT(length <= dataSize());
			getArray(value, rpos(), length);
			value[length] = '\0';
		}
		else
		{
			BOOST_ASSERT(length <= MAX_STRING_LENGTH);
		}

		rskip(length);
	}

	/**
	 * @brief Read an UUID object.
	 *
	 * @param value The UUID variable to be read.
	 */
	inline void readBuiltin(UUID& value)
	{
		readDirect(value);
	}

	/**
	 * @brief Read a std::vector<T> object.
	 *
	 * @param value The std::vector<T> variable to be read.
	 */
	template <typename T>
	inline void readBuiltin(std::vector<T>& value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length <= MAX_VECTOR_LENGTH))
		{
			value.clear();
			// since we know the number of elements to read, just reserve it first for better performance
			value.reserve(length);
			for(uint32 i = 0; i < length; ++i)
			{
				T x; read(x);
				value.push_back(x);
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_VECTOR_LENGTH);
		}
	}

	/**
	 * @brief Read a std::list<T> object.
	 *
	 * @param value The std::list<T> variable to be read.
	 */
	template <typename T>
	inline void readBuiltin(std::list<T>& value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length <= MAX_LIST_LENGTH))
		{
			value.clear();
			for(uint32 i = 0; i < length; ++i)
			{
				T x; read(x);
				value.push_back(x);
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_LIST_LENGTH);
		}
	}

	/**
	 * @brief Read a std::map<K,V> object.
	 *
	 * @param value The std::map<K,V> variable to be read.
	 */
	template <typename K, typename V>
	inline void readBuiltin(std::map<K,V>& value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length <= MAX_LIST_LENGTH))
		{
			value.clear();
			for(uint32 i = 0; i < length; ++i)
			{
				K k; read(k);
				V v; read(v);
				value[k] = v;
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_LIST_LENGTH);
		}
	}

	/**
	 * @brief Read a boost::array<T,N> object.
	 *
	 * @param value The boost::array<T,N> variable to be read.
	 */
	template <typename T, std::size_t N>
	inline void readBuiltin(boost::array<T, N>& value)
	{
		uint32 length; readDirect(length);
		if(LIKELY(length == N * sizeof(T)))
		{
			readBoostArrayImpl(value, boost::mpl::bool_< is_direct_types<T>::value >());
		}
		else
		{
			BOOST_ASSERT(length == N * sizeof(T));
		}
	}

	/**
	 * @brief Read a boost::system::error_code object.
	 *
	 * @param value The boost::system::error_code to be read.
	 */
	inline void readBuiltin(boost::system::error_code &value)
	{
		int32 code; readDirect(code);
		int32 category; readDirect(category);

		switch(category)
		{
		case 0:
			value.assign(code, boost::system::get_system_category()); break;
		case 1:
			value.assign(code, boost::system::get_generic_category()); break;
		case 2:
			value.assign(code, boost::system::get_posix_category()); break;
		default:
			BOOST_ASSERT("reading unknown boost::system::error_code category code" && 0);
			break;
		}
	}

	/**
	 * @brief Read another Buffer object.
	 *
	 * @param value The Buffer variable to be read.
	 */
	inline void readBuiltin(Buffer& value)
	{
		uint32 length; readDirect(length);

		BOOST_ASSERT(value.freeSize() >= length);
		BOOST_ASSERT(dataSize() >= length);

		value.append(*this, length);
	}

	/**
	 * @brief The array element is native types, so we can perform direct memory copy to save time
	 *
	 * @param value The boost::array<T,N> variable to be read.
	 */
	template <typename T, std::size_t N>
	inline void readBoostArrayImpl(boost::array<T, N>& value, boost::mpl::true_ /*native_copy*/)
	{
		readArray((char*)value.data(), N * sizeof(T));
	}

	/**
	 * @brief The array element is not native types, read the element one by one
	 *
	 * @param value The boost::array<T,N> variable to be read.
	 */
	template <typename T, std::size_t N>
	inline void readBoostArrayImpl(boost::array<T, N>& value, boost::mpl::false_ /*non_native_copy*/)
	{
		for(int i=0;i<N;++i)
		{
			read(value[i]);
		}
	}

	/**
	 * @brief Read an arbitrary type of object.
	 *
	 * @param value The object to be read.
	 */
	template <typename T>
	inline void readDirect(T& value)
	{
		getDirect(value, mReadPos);

		mReadPos += sizeof(T);
	}

	/**
	 * @brief Read an array of data with given size.
	 *
	 * @param dest The pointer to the array.
	 * @param size The given size to be read.
	 */
	inline void readArray(char* dest, std::size_t size)
	{
		if(UNLIKELY(!dest))	return;

		getArray(dest, mReadPos, size);

		mReadPos += size;
	}

public:
	template <typename T>
	inline void write(const T& value)
	{
		writeDispatch(value, boost::mpl::bool_< is_builtin_types<T>::value >() );
	}

//private:
	template <typename T>
	inline void writeDispatch(const T& value, boost::mpl::true_ /*is_builtin_types*/)
	{
		writeBuiltin(value);
	}

	template <typename T>
	inline void writeDispatch(const T& value, boost::mpl::false_ /*is_builtin_types*/)
	{
		writeSerializable(value);
	}

	/**
	 * @brief Write any data structure byte by byte.
	 *
	 * @param value Any data structure.
	 */
	template <typename T>
	inline void writeAny(const T& value)
	{
		BOOST_ASSERT(!mReadOnly);
		writeArray((const char*)&value, sizeof(T));
	}

	struct WriteProxyArchive
	{
		WriteProxyArchive(Buffer& buffer) : mBuffer(buffer) { }

		template <typename T>
		inline void operator & (const T& v)
		{
			mBuffer.write(v);
		}

		inline void skip(std::size_t size)
		{
			mBuffer.wskip(size);
		}

		Buffer& buffer() { return mBuffer; }

		Buffer& mBuffer;
	};

	/**
	 * @brief The helper archive class to write serializable data structure.
	 */
	template <typename T>
	inline void writeSerializable(const T& value)
	{
		BOOST_ASSERT(!mReadOnly);
		WriteProxyArchive ar(*this);

		// we have const T here, but we are going to call T::serialize(),
		// which is not const function (because they shared definition with readSerializable)
		// so an explicit const conversion is performed here.
		(const_cast<T&>(value)).serialize(ar, /*UNUSED*/ 0 );
	}

	/**
	 * @brief Write an int8 (1 byte) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const int8& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an uint8 (1 byte) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const uint8& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an int16 (2 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const int16& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an uint16 (2 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const uint16& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an int32 (4 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const int32& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an uint32 (4 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const uint32& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an int64 (8 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const int64& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write an uint64 (8 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const uint64& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write a float (4 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const float& value)
	{
		writeDirect(value);
	}

	/**
	 * @brief Write a double (8 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const double& value)
	{
		writeDirect(value);
	}

#ifdef __PLATFORM_WINDOWS__
	/**
	 * @brief Write a std::size_t (8 bytes or 4 byte depend on the platform) variable.
	 *
	 * @todo Consider cross-platform issue, because on Windows there're few 64bit installation.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const std::size_t& value)
	{
		writeDirect(value);
	}
#endif

	/**
	 * @brief Write a boolean (treated as uint8) (1 bytes) variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const bool& value)
	{
		uint8 x = value ? 1 : 0;
		writeDirect(x);
	}

	/**
	 * @brief Write a std::string variable.
	 *
	 * @param value The value to be written.
	 */
	inline void writeBuiltin(const std::string& value)
	{
		uint32 length = value.length();
		if(LIKELY(length <= MAX_STRING_LENGTH))
		{
			writeDirect(length);
			writeArray((const char*)value.data(), length);
		}
		else
		{
			BOOST_ASSERT(length <= MAX_STRING_LENGTH);
		}
	}

	/**
	 * @brief Write an array of data.
	 *
	 * @param value The array to be written.
	 */
	inline void writeBuiltin(const char* value)
	{
		std::size_t length = strlen(value);
		if(LIKELY(length <= MAX_STRING_LENGTH))
		{
			writeDirect(length);
			writeArray(value, length);
		}
		else
		{
			BOOST_ASSERT(length <= MAX_STRING_LENGTH);
		}
	}

	/**
	 * @brief Write an UUID object.
	 *
	 * @param value The UUID variable to be written.
	 */
	inline void writeBuiltin(const UUID& value)
	{
		//writeArray((const char*)&value.data[0], sizeof(UUID));
		writeDirect(value);
	}

	/**
	 * @brief Write a std::vector<T> object.
	 *
	 * @param value The std::vector<T> variable to be written.
	 */
	template <typename T>
	inline void writeBuiltin(const std::vector<T>& value)
	{
		uint32 length = value.size();
		if(LIKELY(length <= MAX_VECTOR_LENGTH))
		{
			writeDirect(length);
			for(typename std::vector<T>::const_iterator i = value.begin(); i != value.end(); ++i)
			{
				write(*i);
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_VECTOR_LENGTH);
		}
	}

	/**
	 * @brief Write a std::list<T> object.
	 *
	 * @param value The std::list<T> variable to be written.
	 */
	template <typename T>
	inline void writeBuiltin(const std::list<T>& value)
	{
		uint32 length = value.size();
		if(LIKELY(length <= MAX_VECTOR_LENGTH))
		{
			writeDirect(length);
			for(typename std::list<T>::const_iterator i = value.begin(); i != value.end(); ++i)
			{
				write(*i);
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_VECTOR_LENGTH);
		}
	}

	/**
	 * @brief Write a std::list<T> object.
	 *
	 * @param value The std::list<T> variable to be written.
	 */
	template <typename K, typename V>
	inline void writeBuiltin(const std::map<K,V>& value)
	{
		uint32 length = value.size();
		if(LIKELY(length <= MAX_VECTOR_LENGTH))
		{
			writeDirect(length);
			for(typename std::map<K,V>::const_iterator i = value.begin(); i != value.end(); ++i)
			{
				write(i->first);
				write(i->second);
			}
		}
		else
		{
			BOOST_ASSERT(length <= MAX_VECTOR_LENGTH);
		}
	}

	/**
	 * @brief Write a boost::array<T,N> object.
	 *
	 * @param value The boost::array<T,N> variable to be written.
	 */
	template <typename T, std::size_t N>
	inline void writeBuiltin(const boost::array<T, N>& value)
	{
		if(LIKELY(N * sizeof(T) <= MAX_ARRAY_LENGTH))
		{
			uint32 length = N * sizeof(T);
			writeDirect(length);
			writeBoostArrayImpl(value, boost::mpl::bool_< is_direct_types<T>::value >());
		}
		else
		{
			BOOST_ASSERT(N * sizeof(T) <= MAX_ARRAY_LENGTH);
		}
	}

	/**
	 * @brief Write a boost::system::error_code object.
	 *
	 * @param value The boost::system::error_code variable to be written.
	 */
	inline void writeBuiltin(const boost::system::error_code &value)
	{
		int32 code = value.value(); writeDirect(code);
		const boost::system::error_category& category = value.category();
		int32 cat = 0;

		if(category == boost::system::get_system_category())
		{
			cat = 0;
		}
		else if(category == boost::system::get_generic_category())
		{
			cat = 1;
		}
		else if(category == boost::system::get_posix_category())
		{
			cat = 2;
		}
		else
		{
			BOOST_ASSERT("writing unknown boost::system::error_code category" && 0);
		}

		writeDirect(cat);
	}

	/**
	 * @brief Write another Buffer object.
	 *
	 * @param value Another Buffer object to be written.
	 */
	inline void writeBuiltin(const Buffer& value)
	{
		uint32 length = value.dataSize();
		writeDirect(length);

		BOOST_ASSERT(freeSize() >= length);

		Buffer& non_const_value = const_cast<Buffer&>(value);
		append(non_const_value, length);
	}

	/**
	 * @brief The array element is native types, so we can perform direct memory copy to save time.
	 *
	 * @param value The boost::array<T,N> variable to be written.
	 */
	template <typename T, std::size_t N>
	inline void writeBoostArrayImpl(const boost::array<T, N>& value, boost::mpl::true_ /*native_copy*/)
	{
		writeArray((const char*)value.data(), N * sizeof(T));
	}

	/**
	 * @brief The array element is not native types, write the element one by one.
	 *
	 * @param value The boost::array<T,N> variable to be written.
	 */
	template <typename T, std::size_t N>
	inline void writeBoostArrayImpl(const boost::array<T, N>& value, boost::mpl::false_ /*non_native_copy*/)
	{
		for(int i=0;i<N;++i)
		{
			write(value[i]);
		}
	}

	/**
	 * @brief Write an arbitrary type of object.
	 *
	 * @param value The object to be written.
	 */
	template <typename T>
	inline void writeDirect(const T& t)
	{
		BOOST_ASSERT(!mReadOnly);
		if(!mOnDemand)
		{
			BOOST_ASSERT(mAllocatedSize >= mWritePos + sizeof(T));
		}
		else if(mAllocatedSize < mWritePos + sizeof(T))
		{
			std::size_t s = mWritePos + sizeof(T);
			resize(round_up_to_nearest_power_of_two(s));
		}

		setDirect(t, mWritePos);

		mWritePos += sizeof(T);
	}

	/**
	 * @brief Write an array of data with given size.
	 *
	 * @param dest The pointer to the array.
	 * @param size The given size to be written.
	 */
	inline void writeArray(const char* source, std::size_t size)
	{
		BOOST_ASSERT(!mReadOnly);
		if(!mOnDemand)
		{
			BOOST_ASSERT(mAllocatedSize >= mWritePos + size);
		}
		else if(mAllocatedSize < mWritePos + size)
		{
			std::size_t s = mWritePos + size;
			resize(round_up_to_nearest_power_of_two(s));
		}

		setArray(source, mWritePos, size);

		mWritePos += size;
	}

//private:
	/**
	 * @brief Get an object from the buffer at specific buffer position.
	 *
	 * @note Neither the read nor the write pointer is not altered after this call.
	 *
	 * @param t Arbitrary type of object reference.
	 * @param position The specific buffer position.
	 */
	template <typename T>
	inline void getDirect(T& t, std::size_t position)
	{
		t = *((T*)(mData + position));
	}

	/**
	 * @brief Get an array of data from the buffer at specific buffer position.
	 *
	 * @note Neither the read nor the write pointer is not altered after this call.
	 *
	 * @param dest The given array to write data in.
	 * @param position The specific buffer position.
	 * @param size The specific array size.
	 */
	inline void getArray(char* dest, std::size_t position, std::size_t size)
	{
		BOOST_ASSERT(size <= dataSize());

		if(UNLIKELY(size == 0)) return;

		::memcpy(dest, mData + position, size);
	}

	/**
	 * @brief Put an object into the buffer at specific buffer position.
	 *
	 * @note Neither the read nor the write pointer is not altered after this call.
	 *
	 * @param t Arbitrary type of object reference.
	 * @param position The specific buffer position.
	 */
	template <typename T>
	inline void setDirect(const T& t, std::size_t position)
	{
		*((T*)(mData + position)) = t;
	}

	/**
	 * @brief Put an array of data into the buffer at specific buffer position
	 *
	 * @note Neither the read nor the write pointer is not altered after this call.
	 *
	 * @param source The given array to write data in.
	 * @param position The specific buffer position.
	 * @param size The specific array size.
	 */
	inline void setArray(const char* source, std::size_t position, std::size_t size)
	{
		BOOST_ASSERT(size <= freeSize());

		if(UNLIKELY(size == 0)) return;

		::memcpy(mData + position, source, size);
	}

public:
	/**
	 * @brief Append another buffer object.
	 *
	 * @note The read pointer of the source buffer object will not be altered, but the write pointer will be updated.
	 *
	 * @param source The buffer object to be read.
	 */
	inline void append(Buffer &source)
	{
		BOOST_ASSERT(source.dataSize() <= freeSize());
		append(source, source.dataSize());
	}

	/**
	 * @brief Append another buffer object with specific size.
	 *
	 * @note The write pointer will be updated.
	 *
	 * @param source The buffer object to be read.
	 * @param size The specific data size to append.
	 */
	inline void append(Buffer &source, std::size_t size)
	{
		BOOST_ASSERT(size <= freeSize());
		BOOST_ASSERT(size <= source.dataSize());
		writeArray(source.rptr(), size);
		source.rskip(size);
	}

	inline void resize(std::size_t size)
	{
		BOOST_ASSERT(mOnDemand);
		BOOST_ASSERT(size > mAllocatedSize);

		mData = (byte*)realloc((void*)mData, size);
		mAllocatedSize = size;
	}

public:
	/**
	 * @brief Directly byte-level access to the buffer.
	 *
	 * @param position The given position.
	 * @return The value (byte) at the given position.
	 */
	inline char& operator[] (std::size_t position)
	{
		BOOST_ASSERT(position < mWritePos);
		return mData[position];
	}

public:
	/**
	 * @brief Write a given object into the buffer.
	 *
	 * @note The current write pointer will be altered
	 *
	 * @param value The reference to an arbitrary object (as long as we have implementation of write(T)...).
	 * @return
	 */
	template <typename T>
	inline Buffer& operator<< (const T& value)
	{
		write(value);
		return *this;
	}

public:
	/**
	 * @brief Read a given object from the buffer.
	 *
	 * @note The current read pointer will be altered
	 *
	 * @param value The reference to an arbitrary object reference (as long as we have implementation of read(T)...).
	 * @return
	 */
	template <typename T>
	inline Buffer& operator>> (T& value)
	{
		read(value);
		return *this;
	}

public:
	/**
	 * @brief Get the context object associated with the Buffer object.
	 *
	 * @return The context object in shared_ptr<void> form.
	 */
	inline BufferContext getContext()
	{
		return mContext;
	}

	/**
	 * @brief Set the context object associated with the Buffer object.
	 *
	 * @param c The context object in shared_ptr<void> form.
	 */
	inline void setContext(BufferContext c)
	{
		mContext = c;
	}

	/**
	 * @brief Clear or reset the context object associated with the Buffer object.
	 */
	inline void clearContext()
	{
		mContext.reset();
	}

public:
	byte* mData;
	std::size_t mAllocatedSize;

private:
	const static std::size_t DEFAULT_SIZE = 0x1000; // default size is 4Kb
	const static std::size_t MAX_STRING_LENGTH = 8192;
	const static std::size_t MAX_VECTOR_LENGTH = 65536;
	const static std::size_t MAX_LIST_LENGTH = 65536;
	const static std::size_t MAX_ARRAY_LENGTH = 65536;

	bool mOwner;
	bool mReadOnly;
	bool mOnDemand;

	std::size_t mReadPos;
	std::size_t mWritePos;
	std::size_t mReadPosMarked;
	std::size_t mWritePosMarked;

	BufferContext mContext;
};

inline std::ostream& operator << (std::ostream &stream, Buffer& b)
{
	std::size_t size = b.dataSize();
	stream.write((char*)&size, sizeof(std::size_t));
	stream.write(b.rptr(), b.dataSize());
	b.rskip(size);
	return stream;
}

inline std::istream& operator >> (std::istream& stream, Buffer& b)
{
	std::size_t size;
	stream.read((char*)&size, sizeof(std::size_t));
	BOOST_ASSERT(b.freeSize() >= size);
	stream.read(b.wptr(), size);
	b.wskip(size);
	return stream;
}

class ThreadLocalBufferWrapper
{
	struct spec_type { enum type { size_only, non_const_buffer_with_size, const_buffer_with_size }; };
	struct
	{
		spec_type::type type;
		union
		{
			struct
			{
				std::size_t size;
			} size_only;

			struct
			{
				byte* buffer;
				std::size_t size;
			} non_const_buffer_with_size;

			struct
			{
				const byte* buffer;
				std::size_t size;
			} const_buffer_with_size;
		} def;
	} spec;

public:
	ThreadLocalBufferWrapper(std::size_t size)// : data(boost::bind(finalize, _1, true))
	{
		spec.type = spec_type::size_only;
		spec.def.size_only.size = size;
	}

	ThreadLocalBufferWrapper(byte* data, std::size_t size)// : data(boost::bind(finalize, _1, true))
	{
		spec.type = spec_type::non_const_buffer_with_size;
		spec.def.non_const_buffer_with_size.buffer = data;
		spec.def.non_const_buffer_with_size.size = size;
	}

	ThreadLocalBufferWrapper(const byte* data, std::size_t size)// : data(boost::bind(finalize, _1, true))
	{
		spec.type = spec_type::const_buffer_with_size;
		spec.def.const_buffer_with_size.buffer = data;
		spec.def.const_buffer_with_size.size = size;
	}

	~ThreadLocalBufferWrapper()
	{
	}

public:
	Buffer* get()
	{
		Buffer* ptr = data.get();
		if(!ptr)
		{
			switch(spec.type)
			{
			case spec_type::size_only:
				ptr = new Buffer(spec.def.size_only.size);
				break;
			case spec_type::non_const_buffer_with_size:
				ptr = new Buffer(spec.def.non_const_buffer_with_size.buffer, spec.def.non_const_buffer_with_size.size);
				break;
			case spec_type::const_buffer_with_size:
				ptr = new Buffer(spec.def.const_buffer_with_size.buffer, spec.def.const_buffer_with_size.size);
				break;
			}
			data.reset(ptr);
		}

		return ptr;
	}

	Buffer* operator-> ()
	{
		return get();
	}

	Buffer& operator* ()
	{
		return *get();
	}

private:
	static void finalize(Buffer* buffer, bool need_cleanup)
	{
		if(need_cleanup && buffer != NULL)
		{
			delete buffer;
		}
	}

private:
	boost::thread_specific_ptr<Buffer> data;
};

}

#endif/*ZILLIANS_BUFFER_H_*/