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
 * @date Mar 22, 2010 sdk - Initial version created.
 */

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

#include "core-api/Buffer.h"

namespace zillians {

class CircularBuffer
{
public:
	CircularBuffer(SharedPtr<Buffer> buffer)
	{
		mBuffer = buffer;
	}

	CircularBuffer(std::size_t size)
	{
		mBuffer = SharedPtr<Buffer>(new Buffer(size));
	}

	CircularBuffer(byte* data, std::size_t size)
	{
		mBuffer = SharedPtr<Buffer>(new Buffer(data, size));
	}

	~CircularBuffer()
	{

	}

public:
	/**
	 * @brief Probe the actual data size of a given variable.
	 *
	 * @param value The variable to probe its actual data size.
	 * @return The actual data size of the variable stored in the Buffer object.
	 */
	template <typename T>
	inline static std::size_t probeSize(const T &value)
	{
		return Buffer::probeSizeBuiltin(value);
	}

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
		return Buffer::probeSizeSerializable(t);
	}

	/**
	 * @brief Reset the read and write pointer of the Buffer object.
	 *
	 * @note Note that the actual data is not altered but just the pointers reset.
	 */
	inline void clear()
	{
		mBuffer->clear();
	}

	/**
	 * @brief Get the allocated size (the internal data buffer size).
	 *
	 * @return The internal allocated buffer size.
	 */
	inline std::size_t allocatedSize() const
	{
		return mBuffer->allocatedSize();
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

		return mBuffer->allocatedSize() - mFillCount;
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
		return mFillCount;
	}

	/**
	 * @brief Move forward the current read pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to skip reading.
	 */
	inline void rskip(std::size_t bytes)
	{
		BOOST_ASSERT(bytes < dataSize());

		std::size_t rpos = mBuffer->rpos();

		if(rpos + bytes < allocatedSize())
		{
			mBuffer->rskip(bytes);
		}
		else
		{
			mBuffer->rpos(rpos + bytes - allocatedSize());
		}

		mFillCount -= bytes;
	}

	/**
	 * @brief Move forward the current write pointer by given number of bytes.
	 *
	 * @param bytes The number of bytes to skip writing.
	 */
	inline void wskip(std::size_t bytes)
	{
		BOOST_ASSERT(bytes < freeSize());

		std::size_t wpos = mBuffer->wpos();

		if(wpos + bytes < allocatedSize())
		{
			mBuffer->wskip(bytes);
		}
		else
		{
			mBuffer->wpos(wpos + bytes - allocatedSize());
		}

		mFillCount += bytes;
	}

	inline std::size_t readv(std::vector<std::pair<void*, std::size_t> >& v, std::size_t size = 0)
	{
		std::size_t sizeToRead;
		if(size == 0)
			sizeToRead = dataSize();
		else
			sizeToRead = std::min(size, dataSize());

		std::size_t rpos = mBuffer->rpos();
		std::size_t wpos = mBuffer->wpos();

		if(sizeToRead > 0)
		{
			if(rpos + sizeToRead < mBuffer->allocatedSize())
			{
				BOOST_ASSERT(rpos + sizeToRead <= wpos);

				v.push_back(std::make_pair((void*)mBuffer->rptr(), sizeToRead));

				mBuffer->rskip(sizeToRead);
			}
			else
			{
				BOOST_ASSERT(mBuffer->rpos() > mBuffer->wpos());
				BOOST_ASSERT(mBuffer->wpos() >= rpos + sizeToRead - allocatedSize());

				v.push_back(std::make_pair((void*)mBuffer->rptr(), mBuffer->allocatedSize() - rpos));
				v.push_back(std::make_pair((void*)mBuffer->rptr(), rpos + sizeToRead - allocatedSize()));

				mBuffer->rpos(rpos + sizeToRead - allocatedSize());
			}

			return sizeToRead;
		}
		else
		{
			return 0;
		}
	}

	inline std::size_t writev(const std::vector<std::pair<void*, std::size_t> >& v, std::size_t size = 0)
	{
		// find out the total size in the given buffer vector
		std::size_t totalSizeToWrite = 0;
		for(std::vector<std::pair<void*, std::size_t> >::const_iterator cit = v.begin(); cit != v.end(); ++cit)
		{
			totalSizeToWrite += cit->second;
		}

		if(size > 0)
		{
			BOOST_ASSERT(size <= totalSizeToWrite);
		}

		// calculate the maximum allowable write size
		std::size_t sizeToWrite;
		if(size > 0)
			sizeToWrite = std::min(std::min(totalSizeToWrite, size), mBuffer->freeSize());
		else
			sizeToWrite = std::min(totalSizeToWrite, mBuffer->freeSize());

		std::size_t rpos = mBuffer->rpos();
		std::size_t wpos = mBuffer->wpos();

		if(sizeToWrite > 0)
		{
			if(wpos + sizeToWrite < mBuffer->allocatedSize())
			{
				mBuffer->wskip(sizeToWrite);
			}
			else
			{

			}
		}
		else
		{
			return 0;
		}
	}

	inline std::size_t read(SharedPtr<Buffer> buffer, std::size_t size = 0)
	{
		std::size_t sizeToRead;
		if(size == 0)
			sizeToRead = buffer->freeSize();
		else
			sizeToRead = size;
	}

	inline std::size_t write(SharedPtr<Buffer> buffer, std::size_t size = 0)
	{
		std::size_t sizeToWrite;
		if(size == 0)
			sizeToWrite = buffer->dataSize();
		else
			sizeToWrite = size;

	}

public:
	SharedPtr<Buffer> mBuffer;
	std::size_t mFillCount;
};

}

#endif /* CIRCULARBUFFER_H_ */
