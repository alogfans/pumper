// Thread.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Implement POSIX Thread in C++ style, supported RAII and the interface may be
// similar to C++1x builtin thread library, but currently support POSIX platforms
// only.

#ifndef __THREAD_H__
#define __THREAD_H__

#include "Status.h"
#include <pthread.h>
#include <functional>

namespace Pumper {
	class Thread
	{
	public:
		explicit Thread(const ThreadFunc &func, const String &name = String());
		~Thread();

		Status Start();
		int Join();
		bool Started();
		pthread_t ThreadId();
		const String& Name();

	private:
		bool is_started;
		bool is_joined;
		ThreadFunc func;
		String name;
		pthread_t pthread_id;
	};

	// Use for Thread::Start(), because PTHRAED requires the following style of functions 
	// for invoking.
	void * pthread_stub(void * thread_data);

	struct ThreadData
	{
		ThreadData(const ThreadFunc &func, const String name) : func(func), name(name) { }
		void Invoke();
		ThreadFunc func;
		String name;
	};

} // namespace Pumper

#endif // __THREAD_H__
