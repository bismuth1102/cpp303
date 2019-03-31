// [303] This is a new file for the server thread pool.  The code herein is not
//       concurrent.  You will need to rewrite most, if not all, of this file in
//       order to create a proper thread pool.  The reference solution is about
//       130 lines of code.

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <list>
#include <functional>
#include <condition_variable>
#include <unistd.h>

using namespace std;

#define  THREADPOOL_MAX_NUM 16

/**
 * thread_pool encapsulates a pool of threads that are all waiting for data to
 * appear in a queue.  Whenever data arrives in the queue, a thread will pull
 * the data off and process it, using the handler function provided at
 * construction time.
 */
class thread_pool {
  using Task = function<void()>;
  vector<thread> _pool;
  queue<Task> _tasks;
  mutex _lock;
  condition_variable _task_cv;
  atomic<int>  _idlThrNum{ 0 };

  /** Has an event caused the pool to start shutting down? */
  bool received_shutdown = false;

  /** The code to run on any item that arrives in the queue */
  std::function<bool(int)> service_routine;

  /** Remote code to run when the thread pool shuts down */
  std::function<void()> shutdown_handler;

public:
  /**
   * construct a thread pool by providing a size and the function to run on each
   * element that arrives in the queue
   *
   * @param size    The number of threads in the pool
   * @param handler The code to run whenever something arrives in the pool
   */
  thread_pool(int size, std::function<bool(int)> handler)
      : service_routine(handler) {
    addThread(size);
    // set a dummy shutdown handler
    shutdown_handler = []() {};
  }

  void addThread(unsigned short size)
  {
    for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
    {
      _pool.emplace_back( [this]{
        while (_run)
        {
          Task task;
          {
            unique_lock<mutex> lock{ _lock };
            _task_cv.wait(lock, [this]{
                return !_run || !_tasks.empty();
            });
            if (!_run && _tasks.empty())
              return;
            task = move(_tasks.front());
            _tasks.pop();
          }
          _idlThrNum--;
          task();
          _idlThrNum++;
        }
      });
      _idlThrNum++;
    }
  }

  /** Allow a user of the pool to see if the pool has been shut down */
  bool check_active() { return !received_shutdown; }

  /**
   * Shutting down the pool can take some time.  await_shutdown() lets a user of
   * the pool wait until the threads are all done servicing clients.
   */
  void await_shutdown() {
    received_shutdown = false;
    _task_cv.notify_all();
    for (thread& thread : _pool) {
      if(thread.joinable())
        thread.join();
    }
  }

  /**
   * When a new connection arrives at the server, it calls this to pass the
   * connection to the pool for processing.
   *
   * @param sd The socket descriptor for the new connection
   */
  void service_connection(int sd) {
    if (service_routine(sd)) {
      received_shutdown = true;
      shutdown_handler();
    }
    // NB: ignore errors in close()
    close(sd);
  }

  /**
   * Allow a user of the pool to provide some code to run when the pool decides
   * it needs to shut down.
   *
   * @param func The code that should be run when the pool shuts down
   */
  void set_shutdown_handler(std::function<void()> func) {
    shutdown_handler = func;
  }
};