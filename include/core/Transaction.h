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
 * @date May 6, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_TRANSACTION_H_
#define ZILLIANS_TRANSACTION_H_

#include "core/Prerequisite.h"
#include "core/Worker.h"

// c++0x threading
#include <thread>
#include <future>
#include <mutex>

namespace zillians {

template<typename TransactionState>
class Transaction
{
public:
	/// Each ActionRoutine would return a boolean value, which indicates whether to continue the next execution or to pause (can be resumed by calling next())
	typedef boost::function< bool(TransactionState&) > ActionRoutine;

	/// Each RollbackRoutine is a counter-part of ActionRoutine which invalidate all changes made by ActionRoutine
	typedef boost::function< void(TransactionState&) > RollbackRoutine;

public:
	Transaction(TransactionState& state) : mTransactionState(state), mProgramCounter(0)
	{ }

	~Transaction()
	{ }

public:
#ifndef __GXX_EXPERIMENTAL_CXX0X__
	inline Transaction& add(ActionRoutine action, RollbackRoutine rollback)
	{
		mTransactions.push_back(std::make_pair(action, rollback));
		return *this;
	}
#else
	inline Transaction& add(ActionRoutine&& action, RollbackRoutine&& rollback)
	{
		mTransactions.push_back(std::make_pair(std::forward<ActionRoutine>(action), std::forward<RollbackRoutine>(rollback)));
		return *this;
	}
#endif

public:
	inline bool next(bool blocking = false)
	{
		if(mProgramCounter == -1)
			return true;

		if(mProgramCounter < (int32)mTransactions.size())
		{
			mResult = std::async(boost::bind(&Transaction::run, this));

			if(blocking)
			{
				return mResult.get();
			}
			else
			{
				return false;
			}
		}
		else
		{
			mProgramCounter = -1;
			return true;
		}
	}

	inline void rollback()
	{
		std::lock_guard<std::mutex> lock(mRunLock);

		int rollbackCounter = (mProgramCounter == -1) ? mTransactions.size() - 1 : mProgramCounter;
		while(rollbackCounter >= 0)
		{
			mTransactions[rollbackCounter].second(mTransactionState);
			--rollbackCounter;
		}
		mProgramCounter = 0;
	}

	inline bool waitForCompletion()
	{
		return mResult.get();
	}

	inline void reset()
	{
		std::lock_guard<std::mutex> lock(mRunLock);
		mProgramCounter = 0;
	}

	inline TransactionState& currentState()
	{
		return mTransactionState;
	}

private:
	inline bool run()
	{
		// how to deal with race condition here? (i.e. running an action which returns false but the next() call happened before the end of the current thread context)
		// for example, we send a message and wait for its return, but the return happened before the end of current thread, so next() is called here, which creates a new thread
		// the current solution here is to make sure there's only one thread in the run() function
		std::lock_guard<std::mutex> lock(mRunLock);

		if(mProgramCounter < 0)
			return true;

		while(mProgramCounter < (int32)mTransactions.size())
		{
			try
			{
				bool result = mTransactions[mProgramCounter].first(mTransactionState);
				++mProgramCounter;
				if(!result) break;
			}
			catch(...)
			{
				int rollbackCounter = mProgramCounter;
				while(rollbackCounter >= 0)
				{
					try
					{
						mTransactions[rollbackCounter].second(mTransactionState);
					}
					catch(...)
					{
						// log here since we failed to rollback
					}
					--rollbackCounter;
				}
				mProgramCounter = 0;
				throw;
			}
		}

		if(mProgramCounter == (int32)mTransactions.size())
		{
			mProgramCounter = -1;
			return true;
		}

		return false;
	}

protected:
	std::vector< std::pair<ActionRoutine, RollbackRoutine> > mTransactions;
	TransactionState& mTransactionState;
	std::future<bool> mResult;
	std::mutex mRunLock;
	int32 mProgramCounter;
};

}

#endif /* ZILLIANS_TRANSACTION_H_ */
