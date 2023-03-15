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
				Task task;
				{
					std::unique_lock<std::mutex> lock(mTasksMutex);
					// block until something in queue or terminate called
					mCondition.wait(lock, [this](){
						return !mTasks.empty() || mTerminate;
					});
					if(mTerminate && mTasks.empty()) return; // exit condition
					task = mTasks.front();
					mTasks.pop();
				}
				task();
				--mBusy;
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

int ThreadPool::size() const {
	return mThreads.size();
}

ThreadPool& ThreadPool::push(Task task){
	++mBusy; // tally execution immediately
	{
		std::unique_lock<std::mutex> lock(mTasksMutex);
		mTasks.push(task);
	}
	mCondition.notify_one();
	return *this;
}

ThreadPool& ThreadPool::pushRange(int count, std::function<void(int)> func){
	return pushRange(count, [func](int i, int j){ func(i); });
}

ThreadPool& ThreadPool::pushRange(int count, std::function<void(int,int)> func){
	int numTasks = size();
	double di = double(count)/numTasks;
	for(int j=0; j<numTasks; ++j){
		int beg =  j   *di;
		int end = (j+1)*di;
		//printf("[%d, %d)\n", beg, end);
		push([j,beg,end,func](){
			for(int i=beg; i<end; ++i) func(i,j);
		});
	}
	return *this;
}

unsigned ThreadPool::busy(){
	return mBusy;
}

void ThreadPool::wait(){
	while(busy()){}
}


/*static*/ ThreadPool& ThreadPool::get(){
	static thread_local ThreadPool tp;
	return tp;
}
