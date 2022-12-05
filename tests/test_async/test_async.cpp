// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include "qb/core/thread_pool.hpp"

#include <string>
#include <future>
#include <chrono>
#include <thread>

// add job handler
  std::string run_async_internal(qb::session& s, const std::size_t i, const std::size_t j) {
  //std::stringstream msg;
  //msg << "I am thread " << std::this_thread::get_id() << " running on (i,j): (" << i << ","<< j<<")"<< std::endl; std::cout << msg.str(); msg.str("");
    std::shared_ptr<xacc::Accelerator> qpu(s.get_executor().getNextAvailableQpu());
  //msg << "thread " << std::this_thread::get_id() << " recource allocated" << std::endl; std::cout << msg.str(); msg.str("");
    s.run_async(i, j, qpu);
  //msg << "thread " << std::this_thread::get_id() <<" finished run_async" << std::endl; std::cout << msg.str(); msg.str("");
    s.get_executor().release(std::move(qpu));
  //msg << "thread " << std::this_thread::get_id() <<" released"<< std::endl; std::cout << msg.str(); msg.str("");
    return s.get_out_raws()[i][j];
  };


int main()
{

  std::cout << "execute async test" << std::endl;

  // Start a QB SDK session.
  //auto s = qb::session(true);
  auto s = qb::session(false);
  // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps back-end
  s.qb12();

  std::size_t nWorkers  = 1;
  std::size_t nJobs   = 200; //nWorkers*20;
  std::size_t nOuterLoops = 1;//50;

  std::size_t nThreads = 1;
  qb::thread_pool::set_num_threads(nThreads);
  std::cout << "number of threads: " << qb::thread_pool::get_num_threads() << std::endl;

  // configure parallel workers
  s.set_acc("aer");

  std::stringstream async_workers;
  async_workers << "{\"accs\": [";
  for (std::size_t i=0; i<(nWorkers-1); ++i) {
    async_workers << "{\"acc\": \"aer\"},";
  }
  async_workers << "{\"acc\": \"aer\"}]}";
  s.set_parallel_run_config(async_workers.str());


  // targetCircuit: contains the quantum circuit that will be processed/executed
/*  const std::string targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qreg q) {
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[1];
    cx q[1],q[0];
    measure q[1] -> c[1];
    measure q[0] -> c[0];}
  )";
*/

  //s.set_qn(4);
/*const std::string targetCircuit = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[4];
    creg c[4];
    x q[0];
    x q[2];
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
)";
*/

// use heavier circuit:
  s.set_qn(16);
  const std::string targetCircuit = R"(
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

  // add jobs to the queue
  std::size_t j = 0;
  std::vector<std::vector<std::string>> instrings{nJobs};
  for (auto&& elem : instrings){
    elem.push_back(targetCircuit);
  }
  const std::vector<std::vector<std::string>>& instringsv{instrings};

  for (std::size_t outerLoop = 0; outerLoop<nOuterLoops; ++outerLoop){
    std::cout << "\nOuterLoop: (" << outerLoop + 1 << "/" << nOuterLoops << ")"<< std::endl;
    s.set_instrings(instringsv);

    // compute jobs async
    std::cout << "\tsubmitting jobs..." << std::endl;
    std::vector<std::future<std::string>> futures{};
    for (std::size_t i = 0; i<nJobs; ++i){
      //futures.push_back(std::async(std::launch::async, run_async_internal, std::ref(s), i, j));
      futures.push_back(qb::thread_pool::submit(run_async_internal, std::ref(s), i, j));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "\tsubmitted all jobs. Computing...\n";
    std::future_status status;
    std::vector<bool> finished(nJobs,false);
    std::size_t counter{};
    std::size_t loopCounter{0};
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    counter = 0;
      std::stringstream msg;
      //msg << "Workers ready: [";
      for (std::size_t i = 0; i<nJobs; ++i){
        if (finished[i] == false) {
          status = futures[i].wait_for(std::chrono::milliseconds(0));
          if (status == std::future_status::ready){
            finished[i] = true;
          }
        }
        if (finished[i] == true){
          //msg << "1,";
          counter++;
        }
        else {
          //msg << "0,";
        }
      }
      //msg << '\b' << "], (" << counter << "/" << nJobs << ")" << std::endl; std::cout << msg.str();
      msg << "\t\tworkers ready: (" << counter << "/" << nJobs << ")" << std::endl; std::cout << msg.str();
      loopCounter++;
    } while (counter < nJobs);

    std::cout << "\tfinished all workers in " << loopCounter << " iterations!" << std::endl;

    // retrieve results from futures
    std::vector<std::string> results{};
    for (std::size_t i = 0; i<nJobs; ++i){
      results.push_back(futures[i].get());
      //std::cout << "\tresults[" << i << "]:\n" << results[i] << std::endl;
      if (results[i].empty()){
        std::cout << "\tresults[" << i << "] is empty!" << std::endl;
      }
    }
    std::cout << "\tfinished data retrieval!" << std::endl;
  }// end outerLoop

  std::cout << "\nEnd! " << std::endl;
}
