#include "qb/core/async_executor.hpp"
#include <gtest/gtest.h>
#include "xacc.hpp"
#include <fstream>
#include <chrono>
#include <thread>
TEST(coreCLITester, checkSimple) {
  const std::string qpu_config = R"(
{
    "accs": [
        {
            "acc": "qpp"
        },
        {
            "acc": "aer"
        }
    ]
}
 )";
  qb::Executor executor;
  executor.initialize(qpu_config);
  auto compiler = xacc::getCompiler("staq");
  auto program = compiler
                     ->compile(R"(
OPENQASM 2.0;
include "qelib1.inc";
qreg q[4];
creg c[4];
x q[0];
x q[2];
barrier q;
h q[0];
cu1(pi/2) q[1],q[0];
h q[1];
cu1(pi/4) q[2],q[0];
cu1(pi/2) q[2],q[1];
h q[2];
cu1(pi/8) q[3],q[0];
cu1(pi/4) q[3],q[1];
cu1(pi/2) q[3],q[2];
h q[3];
measure q -> c;
)",
                               nullptr)
                     ->getComposites()[0];

    const int nTests = 1000;
    std::vector<qb::Handle> jobHandles;
    for (int i = 0; i < nTests; ++i) {
      std::cout << "Posting quantum job " << i << " for execution\n";
      auto handle = qb::post(executor, program, 1024);
      jobHandles.emplace_back(std::move(handle));
    }
    std::cout << "Complete posting all " << nTests << " jobs\n";
    bool allReady = false;
    while (!allReady) {
      int complete_jobs = 0;
      for (int i = 0; i < nTests; ++i) {
        const bool ready = jobHandles[i].wait_for(std::chrono::seconds(0)) ==
                           std::future_status::ready;
        // const std::string status_str = ready ? "DONE" : "PENDING";
        // std::cout << "      - Job " << i << ": " << status_str << "\n";
        if (ready) {
          complete_jobs++;
        }
      }
      allReady = complete_jobs == nTests;
      if (!allReady) {
        std::cout << "Complete " << complete_jobs << "/" << nTests
                  << " jobs. Sleep for 1 sec then check again...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    std::cout << "All jobs have been completed.\n";
    // for (auto &handle : jobHandles) {
    //   std::cout << qb::sync(handle) << "\n";
    // }
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
