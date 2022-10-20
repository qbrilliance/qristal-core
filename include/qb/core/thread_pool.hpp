// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <mutex>
#include <thread>
#include <future>
#include <atomic>
#include <vector>
#include <queue>
#include <functional>
#include <type_traits>

namespace qbOS
{

  /// A threadsafe singleton thread pool class based on std::thread
  class thread_pool
  {

    public:

      /// Public interface for setting the number of threads to be maintained in the pool
      static void set_num_threads(const int);

      /// Public interface for retrieving the number of threads to be maintained in the pool
      static int get_num_threads();

      /// Set the number of threads to be maintained in the pool
      void set_num_threads_internal(const int);

      /// Retrieve the number of threads to be maintained in the pool
      int get_num_threads_internal();

      /// Public interface for sending a function with a return type to the thread pool for execution
      template <class Function, class... Args, typename result_type = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
      // This uses SFINAE to disable the template if result_type == void
      static typename std::enable_if<not std::is_void<result_type>::value, std::future<result_type>>::type submit(Function&& f, Args&&... args)
      {
        return get_instance().internal_submit(f, args...);
      }

      /// Public interface for sending a function with no return type to the thread pool for execution
      template <class Function, class... Args, typename result_type = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
      // This uses SFINAE to disable the template if result_type != void
      static typename std::enable_if<std::is_void<result_type>::value, void>::type submit(Function&& f, Args&&... args)
      {
        get_instance().internal_submit(f, args...);
      }

      /// Send a function with a return type to the thread pool for execution
      template <class Function, class... Args, typename result_type = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
      // This uses SFINAE to disable the template if result_type == void
      typename std::enable_if<not std::is_void<result_type>::value, std::future<result_type>>::type internal_submit(Function&& f, Args&&... args)
      {
        // Define a promise in which to capture the result of the submitted function
        std::shared_ptr<std::promise<result_type>> promise_ptr =
         std::make_shared<std::promise<result_type>>();

        // Define a future from the result promise
        std::future<result_type> result_future = promise_ptr->get_future();

        // Make a void() lambda out of the submitted function, returning the result to the promise
        auto func = [=] { promise_ptr->set_value(std::invoke(f, args...)); };

        // Threadlocked block for updating the queue
        {
          std::scoped_lock<std::mutex> lock(queue_m);
          // Put the function in the queue
          queue.push(func);
        }

        // Return the future obtained from the result promise
        return result_future;
      }

      /// Send a function with no return value to the thread pool for execution
      template <class Function, class... Args, typename result_type = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
      // This uses SFINAE to disable the template if result_type != void
      typename std::enable_if<std::is_void<result_type>::value, void>::type internal_submit(Function&& f, Args&&... args)
      {
        // Make a void() lambda out of the submitted function
        auto func = [=] { std::invoke(f, args...); };
        // Threadlocked block for updating the queue
        {
          std::scoped_lock<std::mutex> lock(queue_m);
          // Put the function in the queue
          queue.push(func);
        }
      }

      /// Uncopyable
      thread_pool(const thread_pool&) = delete;

      /// Unassignable
      thread_pool &operator=(const thread_pool&) = delete;

      // Deleting the copy operations means that the compiler does not define default move versions either.

    private:

      /// Getter for the instance; makes this class a threadsafe singleton
      static thread_pool& get_instance();

      /// Constructor
      thread_pool();

      /// Destructor
      ~thread_pool();

      /// Initialise a thread-future pair
      std::pair<std::thread, std::shared_ptr<bool>> initialise_thread();

      /// Work collector.  Each thread runs this indefinitely until the pool is destroyed.
      void loop(std::shared_ptr<bool>);

      /// Number of threads to be maintained in the pool
      int num_threads;

      /// Number of active threads in the pool
      int num_active_threads;

      /// A vector of threads for running tasks, paired with flags indicating if they have exited their loop.
      std::vector<std::pair<std::thread, std::shared_ptr<bool>>> threads;

      /// A queue of tasks waiting to be run by threads
      std::queue<std::function<void()>> queue;

      /// Thread lockers for the queue and the shutdown flag
      std::mutex queue_m, threads_m;

      /// Flag indicating that the pool is being destroyed
      std::atomic<bool> shutting_down;

  };

}
