// Thread.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Implement POSIX Thread in C++ style, supported RAII and the interface may be
// similar to C++1x builtin thread library, but currently support POSIX platforms
// only.

#include "Status.h"
#include "Thread.h"

#include <pthread.h>
#include <functional>

namespace Pumper {
	Thread::Thread(const ThreadFunc &func, const String &name) 
		: is_started(false), is_joined(false), func(func), name(name), pthread_id(0)
	{
	}

	Thread::~Thread()
	{
		if (is_started && !is_joined)
			pthread_detach(pthread_id);
	}

	Status Thread::Start()
	{
		WARNING_ASSERT(!is_started);
		ThreadData * thread_data = new ThreadData(func, name);
		if (pthread_create(&pthread_id, NULL, &pthread_stub, thread_data) < 0)
		{
			delete thread_data;
			RETURN_WARNING("pthread_create failed");
		}
		is_started = true;
		RETURN_SUCCESS();
	}

	int Thread::Join()
	{
		is_joined = true;
		return pthread_join(pthread_id, NULL);
	}

	bool Thread::Started()
	{
		return is_started;
	}

	pthread_t Thread::ThreadId()
	{
		return pthread_id;
	}

	const String& Thread::Name()
	{
		return name;
	}

	void * pthread_stub(void * native_thread_data)
	{
		if (!native_thread_data)
			return NULL;

		ThreadData * thread_data = (ThreadData *) native_thread_data;
		// Really invoke user-defined function here
		thread_data->Invoke();
		delete thread_data;
		return NULL;
	}

	void ThreadData::Invoke()
	{
		try 
		{
			func();
		}
		catch(const std::exception& ex)
		{
			fprintf(stderr, "Exception caught. Thread: %s, cause: %s\n", name.c_str(), ex.what());
			abort();
		}
		catch(...)
		{
			fprintf(stderr, "Unknwon exception caught. Thread: %s\n", name.c_str());
			exit(-1);
		}
	}

} // namespace Pumper

