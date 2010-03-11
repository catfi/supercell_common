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
 * @date Feb 16, 2009 sdk - Initial version created.
 * @date Feb 16, 2009 nothing - Added documentation
 */

#ifndef ZILLIANS_BUFFERMANAGER_H_
#define ZILLIANS_BUFFERMANAGER_H_

#include "core-api/Prerequisite.h"
#include "core-api/Buffer.h"

namespace zillians {

/**
 * @brief BufferManager is an interface providing buffer object allocation and deallocation
 */
class BufferManager
{
public:
	BufferManager() {};
	virtual ~BufferManager() {};

public:
	/**
	 * @brief This method allocates a buffer with specified size. The buffer is de-allocated automatically by SharedPtr
	 *
	 * @param size The size required
	 * @return The buffer object allocated, or NULL if allocation fails.
	 */
	virtual SharedPtr<Buffer> createBuffer(size_t size) = 0;

	/**
	 * Slice a portion of the original buffer as a new buffer (starting from the read pos of the original buffer.)
	 * The original buffer will be saved in buffer context of the new buffer so that the original buffer will be destroyed after the new buffer buffer (thus we created a parent-child relationship between the original and the new buffer).
	 * After slicing the buffer, the read pointer of original buffer will be advanced by given size
	 * @param buffer the original buffer (parent buffer)
	 * @param size the size of the new buffer (if the size equal to 0, it represents all remaining data)
	 * @return the new buffer (the child buffer)
	 */
	virtual SharedPtr<Buffer> sliceBuffer(SharedPtr<Buffer> original, size_t size = 0) = 0;

	/**
	 * Make a copy of the original buffer object.
	 * The original buffer will be saved in buffer context of the new buffer so that the original buffer will be destroyed after the new buffer buffer (thus we created a parent-child relationship between the original and the new buffer).
	 * When the cloned buffer is changed, the original buffer will be altered, but the read/write pointers are independent
	 * @param original original buffer (parent buffer)
	 * @return the cloned buffer
	 */
	virtual SharedPtr<Buffer> cloneBuffer(SharedPtr<Buffer> original) = 0;
};

}

#endif/*ZILLIANS_BUFFERMANAGER_H_*/
