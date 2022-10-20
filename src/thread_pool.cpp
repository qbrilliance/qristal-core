// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/thread_pool.hpp"

#include <iostream>

namespace qbOS
{

  /// Public interface for setting the number of threads to be maintained in the pool
  void thread_pool::set_num_threads(const int n)
  {
    get_instance().set_num_threads_internal(n);
  }

  /// Public interface for retrieving the number of threads to be maintained in the pool
  int thread_pool::get_num_threads()
  {
    return get_instance().get_num_threads_internal();
  }

  /// Set the number of threads to be maintained in the pool
  void thread_pool::set_num_threads_internal(const int n)
  {
    // Do some housekeeping to clean out std::thread objects leftover from previously shrinking the pool
    {
      std::scoped_lock<std::mutex> lock(threads_m);
      // Erase any threads that have exited the loop() function already
      threads.erase(std::remove_if(threads.begin(), threads.end(),
       [](std::pair<std::thread, std::shared_ptr<bool>>& t)
       {
         // If the thread has finished running loop(), tell it to join in preparation for destruction
         if (*t.second) t.first.join();
         // Erase the thread if and only if it is done
         return *t.second;
       }),
       threads.end());

      // Nothing else to do if num_threads is already equal to the value requested.
      if (num_threads == n) return;

      // More threads requested
      if (n > num_active_threads)
      {
        // Grow the thread vector
        threads.resize(n);
        // Create the new threads and set them to run thread_pool::loop
        for (int i = num_active_threads; i < n; i++) threads.at(i) = initialise_thread();
      }
      // Update the overall number of threads
      num_threads = n;
    }
  }

  /// Retrieve the number of threads to be maintained in the pool
  int thread_pool::get_num_threads_internal()
  {
    return num_threads;
  }

  /// Getter for the instance; makes this class a threadsafe singleton
  thread_pool& thread_pool::get_instance()
  {
    // This is guaranteed to be threadsafe by C++11
    static thread_pool tp;
    return tp;
  }

  /// Constructor
  thread_pool::thread_pool()
   : num_threads(std::thread::hardware_concurrency()),
     shutting_down(false)
  {
    // Create all threads and set them to run thread_pool::loop
    threads.resize(num_threads);
    for (auto& t : threads) t = initialise_thread();
  }

  /// Destructor
  thread_pool::~thread_pool()
  {
    // Tell all threads to finish their current tasks and return from the loop
    shutting_down = true;
    // Wait for them all to finish before finally destroying the assets of the pool.
    for (auto& t : threads) t.first.join();
  }

  /// Initialise a thread-future pair
  std::pair<std::thread, std::shared_ptr<bool>> thread_pool::initialise_thread()
  {
    // Define a flag used to indicate if the loop() member function is finished
    std::shared_ptr<bool> done = std::make_shared<bool>(false);
    // Define a thread that runs the loop() member function, passing the flag as input
    std::thread thread(&thread_pool::loop, this, done);
    // Increment the number of active threads
    num_active_threads += 1;
    // Return a pair of the thread and flag
    return {std::move(thread), done};
  }

  /// Work collector.  Each thread runs this indefinitely until the pool is destroyed.
  void thread_pool::loop(std::shared_ptr<bool> done)
  {
    std::function<void()> task;
    bool work_pending = false;

    // Keep looking for tasks until the thread should drain off. When the
    // destructor starts, the shutting_down flag is set true and no more
    // tasks are allocated to threads, even if the queue is not empty yet.
    while (not shutting_down and not *done)
    {
      // Threadlocked block for updating the queue
      {
        std::scoped_lock<std::mutex> lock(queue_m);
        // Determine if there is work pending or not
        work_pending = not queue.empty();
        if (work_pending)
        {
          // There's work to do! Get the next task from the front of the queue.
          task = queue.front();
          queue.pop();
        }
      }

      // Run the task just taken from the front of the queue.
      if (work_pending) task();

      // Threadlocked block for reading and updating num_active_threads
      {
        std::scoped_lock<std::mutex> lock(threads_m);
        // Determine if this thread should drain out because the pool got shrunk.
        if (num_active_threads > num_threads)
        {
          *done = true;
          num_active_threads -= 1;
        }
      }
    }
  }

}
