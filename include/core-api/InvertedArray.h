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
 * @date Mar 17, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_INVERTEDARRAY_H_
#define ZILLIANS_INVERTEDARRAY_H_

#include <vector>
#include <algorithm>

namespace zillians {

class InvertedArrayItem
{
public:
	InvertedArrayItem() : mInvertedArrayIndex(-1) { }
	~InvertedArrayItem() { }

public:
	inline void setIndex(int index)
	{
		mInvertedArrayIndex = index;
	}

	inline int getIndex()
	{
		return mInvertedArrayIndex;
	}

private:
	int mInvertedArrayIndex;

	// forbid object copy constructor and copy operator
private:
	InvertedArrayItem(const InvertedArrayItem&);
	void operator = (const InvertedArrayItem&);
};

template<typename T>
class InvertedArray
{
public:
	InvertedArray()
	{ }

	~InvertedArray()
	{ }

	inline bool empty()
	{
		return mArrayItems.empty();
	}

	inline std::size_t size()
	{
		return mArrayItems.size();
	}

	inline T*& operator [] (std::size_t index)
	{
		return mArrayItems[index];
	}

	inline void pushBack(T* item)
	{
		if(item)
			item->setIndex(mArrayItems.size());

		mArrayItems.push_back(item);
	}

	inline void erase(T* item)
	{
		erase(T->getIndex());
	}

	inline void erase(std::size_t index)
	{
		if(mArrayItems.back())
			mArrayItems.back()->setIndex(index);

		mArrayItems[index] = mArrayItems.back();
		mArrayItems.pop_back();
	}

	inline void swap(std::size_t index1, std::size_t index2)
	{
		if(mArrayItems[index1])
			mArrayItems[index1]->setIndex(index2);
		if(mArrayItems[index2])
			mArrayItems[index2]->setIndex(index1);
		std::swap(mArrayItems[index1], mArrayItems[index2]);
	}

	inline void clear()
	{
		mArrayItems.clear();
	}

	inline std::size_t index(T* item)
	{
		return item->getIndex();
	}

private:
	std::vector<T*> mArrayItems;

	// forbid object copy constructor and copy operator
private:
	InvertedArray(const InvertedArray&);
	void operator = (const InvertedArray&);

};

} } }

#endif /* ZILLIANS_INVERTEDARRAY_H_ */
