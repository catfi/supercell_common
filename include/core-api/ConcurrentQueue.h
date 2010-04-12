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

#ifndef ZILLIANS_CONCURRENTQUEUE_H_
#define ZILLIANS_CONCURRENTQUEUE_H_

namespace zillians {

template <typename T, int N>
class ConcurrentQueue
{
public:
	ConcurrentQueue()
	{
		mBegin = new Chunk;
		mBeginPos = 0;

		mBack = NULL;
		mBackPos = 0;

		mEnd = mBegin;
		mEndPos = 0;
	}

	~ConcurrentQueue()
	{
		while(true)
		{
			if(mBegin == mEnd)
			{
				delete mBegin;
				break;
			}

			Chunk* current = mBegin;
			mBegin = mBegin->next;
			delete current;
		}
	}

	inline T& front()
	{
		return mBegin->values[mBeginPos];
	}

	inline T& back()
	{
		return mBack->value[mBackPos];
	}

	inline void push()
	{

	}

	inline void pop()
	{

	}

private:
	struct Chunk
	{
		T values[N];
		Chunk* next;
	};

	Chunk* mBegin;
	int mBeginPos;
	Chunk* mBack;
	int mBackPos;
	Chunk* mEnd;
	int mEndPos;

	// forbid object copy constructor and copy operator
private:
	ConcurrentQueue(const ConcurrentQueue&);
	void operator = (const ConcurrentQueue&);
};

}

#endif /* ZILLIANS_CONCURRENTQUEUE_H_ */
