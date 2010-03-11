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
 * @date May 11, 2009 bbbb - Initial version created.
 */

// there are four cores, so we use four threads
// 4*10000000 = 40000000

#define THREAD_BUFFER_SIZE 10000000

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread_time.hpp>
#include <tbb/atomic.h>
#include <iostream>
#include <vector>

using namespace std;
typedef char BYTE;

	int mThreadNum = 4;
	int mBufferNum = 4;
	int mBufferSize;

	tbb::atomic<long long> mThroughPut;
	tbb::atomic<int> mSendCount;
	bool mTerminateThreadFlag;
	time_t mStartTime;

	tbb::atomic<int> mWritePos;
	BYTE* mCurrentBuffer;
//	vector<tbb::atomic<int> > mWritePosList;
//	vector<BYTE*> mCurrentBufferList;
	tbb::atomic<int> mWritePosList[20];
	BYTE* mCurrentBufferList[20];


	// simulate copying messages to a buffer that shared by all threads
	void runThreadSharedBuffer(int singleMsgSize, int delayTime, int threadid)
	{
		while(true)
		{
			// buffer full, send current buffer and create new buffer
			int pos = mWritePos.fetch_and_add(singleMsgSize);
			pos += singleMsgSize;

			if(pos - singleMsgSize < mBufferSize && pos >= mBufferSize)
			{
				mThroughPut += pos - singleMsgSize;
				mSendCount++;
				mWritePos -= mBufferSize;
				continue;
			}

			if(pos > mBufferSize) pos -= mBufferSize;

			//copy data to current buffers
			memset(mCurrentBuffer + pos, 1, singleMsgSize);

			if(mTerminateThreadFlag) break;
			if(delayTime > 0)usleep(delayTime);
		}

		if(threadid == 0)
		{
			mThroughPut += mWritePos;
			mSendCount++;
		}
	}

	// simulate copying messages to a local buffer
	void runThreadMultiBuffer(int singleMsgSize, int delayTime)
	{
		int writePos = 0;
		BYTE* currentBuffer = (BYTE*)malloc(mBufferSize);
		while(true)
		{
			// buffer full, send current buffer and create new buffer
			if(writePos + singleMsgSize > mBufferSize)
			{
				mThroughPut += writePos;
				mSendCount++;
				writePos = 0;
			}

			//copy data to current buffer
			memset(currentBuffer + writePos, 1, singleMsgSize);

			writePos += singleMsgSize;

			if(mTerminateThreadFlag) break;

			if(delayTime > 0)usleep(delayTime);
		}

		mThroughPut += writePos;
		mSendCount++;

		free(currentBuffer);
	}

	void runThreadSharedMultiBuffer(int singleMsgSize, int delayTime, int threadid, int cycleRange)
	{

		int bufferId, cycleCounter = threadid % cycleRange;

		while(true)
		{
			bufferId = min(mBufferNum-1, cycleCounter);
			// buffer full, send current buffer and create new buffer
			int pos = mWritePosList[bufferId].fetch_and_add(singleMsgSize);
			pos += singleMsgSize;
			if(pos - singleMsgSize < mBufferSize && pos >= mBufferSize)
			{
				mThroughPut += pos - singleMsgSize;
				mSendCount++;
				mWritePosList[bufferId] -= mBufferSize;
				continue;
			}

			if(pos > mBufferSize) pos -= mBufferSize;

			//copy data to current buffers
			memset(mCurrentBufferList[bufferId] + pos, 1, singleMsgSize);

			if(mTerminateThreadFlag) break;
			if(delayTime > 0)usleep(delayTime);

			cycleCounter++;
			if(cycleCounter == cycleRange) cycleCounter = 0;
		}

		if(threadid == 0)
		{
			for(int i = 0; i < mBufferNum; i++)
				mThroughPut += mWritePosList[i];
			mSendCount++;
		}
	}

	// print performance result every second
	void timer(int totalTime, long long maxFlow)
	{
		int currentTime = 0;
		cout<<"Thread: "<<mThreadNum<<endl;
		cout<<"Buffer: "<<mBufferNum<<"*"<<mBufferSize<<endl;
		cout<<"Time\tThroughPut\tCost"<<endl;
		cout<<"----------------------------"<<endl;
		while(currentTime < totalTime)
		{
			sleep(1);
			{
				currentTime++;
				// print mThroughPut (mb/s)
				cout<< currentTime <<"\t"<<mThroughPut / 1000000.0<<"\t\t"<<mSendCount<<endl;
			}
		}
		mTerminateThreadFlag = true;
		// print mThroughPut (mb/s)
		cout<<"-----{avg  performance}-----"<<endl;
		cout<<"1\t"<<mThroughPut / (totalTime * 1000000.0) << "\t" << mSendCount/totalTime<<endl;

		if(maxFlow != 0)
		{
			cout << "MaxFlow:" << maxFlow << endl;
			cout << "Ratio  :" << (double)mThroughPut / (maxFlow * 1000000.0) << endl;
		}
	}

	void run(int totalTime, int bufferOption, int singleMsgSize, int delayTime, double balanceRatio)
	{
		boost::thread_group threadGroup;
		mThroughPut = 0;
		mSendCount = 0;
		mWritePos = 0;
		mTerminateThreadFlag = false;
		time(&mStartTime);
		double maxFlow = 0;

		switch(bufferOption)
		{
		case 0:
			mBufferNum = 1;
			mBufferSize = mThreadNum * THREAD_BUFFER_SIZE;
			mCurrentBuffer = (BYTE*)malloc(mBufferSize);
			for(int i = 0; i < (mThreadNum+1)/2; i++)
			{
				threadGroup.create_thread(boost::bind(&runThreadSharedBuffer, singleMsgSize, delayTime*balanceRatio, i));
			}
			for(int i = 0; i < mThreadNum/2; i++)
			{
				threadGroup.create_thread(boost::bind(&runThreadSharedBuffer, singleMsgSize, delayTime*(2-balanceRatio), i + (mThreadNum+1)/2));
			}
			break;
		case 1:
			mBufferSize = THREAD_BUFFER_SIZE;
			mBufferNum = mThreadNum;
			for(int i = 0; i < (mThreadNum+1)/2; i++)
			{
				threadGroup.create_thread(boost::bind(&runThreadMultiBuffer, singleMsgSize, delayTime*balanceRatio));
			}
			for(int i = 0; i < mThreadNum/2; i++)
			{
				threadGroup.create_thread(boost::bind(&runThreadMultiBuffer, singleMsgSize, delayTime*(2-balanceRatio)));
			}
			break;
		case 2:
			mBufferSize = mThreadNum * THREAD_BUFFER_SIZE / mBufferNum;
//			mCurrentBufferList.reserve(mBufferNum);
//			mWritePosList.reserve(mBufferNum);
			for(int i = 0; i < mBufferNum; i++)
			{
//				mCurrentBufferList.push_back((BYTE*)malloc(mBufferSize));
				mCurrentBufferList[i] = (BYTE*)malloc(mBufferSize);
				mWritePosList[i] = 0;
			}
			for(int i = 0; i < mThreadNum; i++)
			{
				threadGroup.create_thread(boost::bind(&runThreadSharedMultiBuffer, singleMsgSize, delayTime, i, (int)mBufferNum/max(0.1, balanceRatio)));
			}
//			mCurrentBuffer = (BYTE*)malloc(mBufferSize);
//			threadGroup.create_thread(boost::bind(&runThreadSharedBuffer, singleMsgSize, delayTime*balanceRatio, 10));

			break;
		}

		if(delayTime != 0)
		{
			// maxFlow (mb)
			maxFlow = (double)totalTime * singleMsgSize * mThreadNum / delayTime;
		}

		threadGroup.create_thread(boost::bind(&timer, totalTime, maxFlow));


		threadGroup.join_all();

		if(bufferOption == 0)free(mCurrentBuffer);
		else if(bufferOption == 2)
		{
			for(int i = 0; i < mBufferNum; i++)
			{
				free(mCurrentBufferList[i]);
			}
		}
	}

	int main(int argc, char* argv[])
	{
		int ch;
		int optionCount = 0;
		while ((ch = getopt(argc, argv, "t:b:")) != -1)
		{
			switch (ch)
			{
			case't': mThreadNum = atoi(optarg); break;
			case'b': mBufferNum = atoi(optarg); break;
			}
			optionCount++;
		}
		argc -= optionCount*2;
		argv += optionCount*2;

		if(argc != 6)
		{
			cout<<"Usage: progname [-t ThreadNumber(default 4)][-b BufferNumber(default 4)]\n"
					"[TotalTime(s)] [BufferOption(share/multi/shareMulti)] [SingleMsgSize(byte)] [DelayTime(ms)] [BalanceRatio(0.1~1)]\n"
				<<"Example: ./GatewayReceivePerformanceTest -b 2 10 2 256 5 0.5\n";

			return 0;
		}

		run(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atof(argv[5]));

		return 0;
	}
