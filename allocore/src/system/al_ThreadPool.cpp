#include "allocore/system/al_ThreadPool.hpp"
using namespace al;

// Implementation based on https://stackoverflow.com/a/32593825

ThreadPool::ThreadPool()
:	ThreadPool(std::thread::hardware_concurrency()){}

ThreadPool::ThreadPool(int numThreads){
	//printf("%d threads\n", numThreads);
	if(numThreads <= 0) numThreads = 1;

	for(int i=0; i<numThreads; ++i){
		mThreads.emplace_back([this](){
			while(true){
				std::function<void(void)> task;
				{
					std::unique_lock<std::mutex> lock(mTasksMutex);
					mCondition.wait(lock, [this](){
						// block until something in queue or terminate called
						return !mTasks.empty() || mTerminate;
					});
					if(mTerminate && mTasks.empty()) return; // exit condition
					task = mTasks.front();
					mTasks.pop();
				}
				task();
			}
		});
	}
}

ThreadPool::~ThreadPool(){
	{
		std::unique_lock<std::mutex> lock(mTasksMutex);
		mTerminate = true;
	}

	mCondition.notify_all(); 			// wake up all threads
	for(auto& t : mThreads) t.join();	// join all threads
}

void ThreadPool::push(Task task){
	{
		std::unique_lock<std::mutex> lock(mTasksMutex);
		mTasks.push(task);
	}
	mCondition.notify_one();
}
