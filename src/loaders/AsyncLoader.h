#pragma once

#include "util/SafeQueue.h"
#include <chrono>
#include <thread>
#include <functional>

#include <fmt/printf.h>
#include <fmt/xchar.h>

#if PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::chrono_literals;

class AsyncLoader
{
	AsyncLoader(uint32_t threadCount)
	{
		_threads.resize(threadCount);
		for (uint32_t i = 0; i < threadCount; i++)
		{
			_threads[i] = std::thread(std::bind(&AsyncLoader::ProcessAsyncWork, this));
			
#if PLATFORM_WIN32
			SetThreadDescription(_threads[i].native_handle(), fmt::sprintf(L"Async Loader Thread %i", i).c_str());
#endif
		}
	}
	~AsyncLoader()
	{
		_queue.Finish();
		for (std::thread& thread : _threads)
			thread.join();
	}

public:
	using AsyncWork = std::function<void (void)>;

	static AsyncLoader* Instance()
	{
		static AsyncLoader loader(8);
		return &loader;
	}

	void ProcessAsyncWork()
	{
		AsyncWork work;
		while (_queue.ConsumeWait(work))
		{
			work();
		}
	}

	void AddWork(AsyncWork work)
	{
		_queue.Produce(work);
	}

private:
	std::vector<std::thread> _threads;
	SafeQueue<AsyncWork> _queue;
};

#define SAsyncLoader AsyncLoader::Instance()