#ifndef INC_AL_THREAD_POOL_HPP
#define INC_AL_THREAD_POOL_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Pool of threads to run concurrent tasks and parallel for loop

	Author(s):
	Lance Putnam, 2021
*/

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

namespace al{

/// @addtogroup allocore
/// @{

/// A pool of threads for running concurrent tasks

/// Creating a thread can be more expensive than the task it executes.
/// A thread pool allows us to create a number of threads once and then simply 
/// reuse them for arbitrary tasks.
class ThreadPool{
public:

	typedef std::function<void(void)> Task;

	/// Construct with a number of threads matching hardware concurrency
	ThreadPool();

	/// Construct with a specific number of threads
	ThreadPool(int numThreads);

	/// Joins all threads
	~ThreadPool();


	/// Get number of threads in pool
	int size() const;

	/// Push a new task onto the queue; execution begins immediately
	ThreadPool& push(Task task);

	/// Runs function in parallel for a specified count; a parallel for loop
	ThreadPool& pushRange(int count, std::function<void(int i)> func);
	ThreadPool& pushRange(int count, std::function<void(int i, int threadID)> func);

	/// Returns how many tasks are currently executing; does not block
	unsigned busy();

	/// Block until all tasks are finished
	void wait();


	/// Get a static thread-safe thread pool
	static ThreadPool& get();

private:
	std::vector<std::thread> mThreads;
	std::queue<Task> mTasks;
	std::condition_variable mCondition;
	std::mutex mTasksMutex;
	std::atomic<unsigned> mBusy{0};
	bool mTerminate = false;
};


/// Helper macro for parallel for loop
#define AL_PARALLEL_FOR(index_name, count, ...)\
	ThreadPool::get().pushRange(count, [&](int index_name){\
		__VA_ARGS__\
	}).wait();


/// @} // end allocore group

} // al::
#endif
