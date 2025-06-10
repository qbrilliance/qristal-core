// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/session.hpp>
#include <qristal/core/thread_pool.hpp>

// STL
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <algorithm>

// range v3
#include <range/v3/view/zip.hpp>

// Gtest
#include <gtest/gtest.h>


// Job handler
std::string run_async_internal(qristal::session& s) {
  using qristal::operator<<;
  s.run();
  std::ostringstream out;
  for (const auto& [bits, count] : s.results()) {
    out << bits << ": " << count << std::endl;
  }
  return out.str();
};


TEST(TestAsyncCircuitExecution, FourSessionsTwoThreads) {

  std::cout << "Execute async test" << std::endl;

  // Make Qristal sessions.
  constexpr size_t n_jobs = 4;
  std::array<qristal::session, n_jobs> sims;

  // Set number of threads available in thread pool
  constexpr int threads = 2;
  qristal::thread_pool::set_num_threads(threads);
  std::cout << "Number of threads in thread pool: " << qristal::thread_pool::get_num_threads() << std::endl;
  EXPECT_EQ(qristal::thread_pool::get_num_threads(), threads);

  // Compute jobs async
  std::cout << "\tsubmitting jobs..." << std::endl;
  std::array<std::future<std::string>, n_jobs> futures;
  for (auto&& [s, f] : ::ranges::views::zip(sims, futures)) {
    s.acc = "aer";
    s.qn = 16;
    s.sn = 1000;
    s.instring = R"(
      OPENQASM 2.0;
      include "qelib1.inc";
      qreg q[16];
      creg c[16];
      h q[0];
      cx q[0],q[1];
      cx q[1],q[2];
      cx q[2],q[3];
      cx q[3],q[4];
      cx q[4],q[5];
      cx q[5],q[6];
      cx q[6],q[7];
      cx q[7],q[8];
      cx q[8],q[9];
      cx q[9],q[10];
      cx q[10],q[11];
      cx q[11],q[12];
      cx q[12],q[13];
      cx q[13],q[14];
      cx q[14],q[15];
      measure q -> c;
    )";
    //f = std::async(std::launch::async, run_async_internal, std::ref(s)));
    f = qristal::thread_pool::submit(run_async_internal, std::ref(s));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cout << "\tsubmitted all jobs. Computing...\n";
  std::future_status status;
  std::array<bool, n_jobs> finished;
  finished.fill(false);
  std::size_t counter{};
  std::size_t loopCounter{0};
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    counter = 0;
    for (auto&& [done, f] : ::ranges::views::zip(finished, futures)) {
      if (not done) {
        status = f.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) done = true;
      }
      if (done) counter++;
    }
    std::cout << "\t\tworkers ready: (" << counter << "/" << n_jobs << ")" << std::endl;
    loopCounter++;
  } while (counter < n_jobs);

  std::cout << "\tfinished all workers in " << loopCounter << " iterations!" << std::endl;

  // retrieve results from futures
  std::array<std::string, n_jobs> results;
  for (auto&& [r, f] : ::ranges::views::zip(results, futures)) r = f.get();
  std::cout << "\tfinished data retrieval!" << std::endl;
  std::cout << "\nEnd! " << std::endl;
  ASSERT_TRUE(std::all_of(results.cbegin(), results.cend(), [](std::string r){ return not r.empty(); }));
}
