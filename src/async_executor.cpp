// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/async_executor.hpp"
#include "qristal/core/thread_pool.hpp"

#include "nlohmann/json.hpp"
#include "xacc.hpp"

#include <chrono>
#include <thread>

namespace qristal {

void Executor::initialize(const std::string &in_qpuConfig) {
  auto qpu_configs = nlohmann::json::parse(in_qpuConfig);
  auto accs_configs = qpu_configs["accs"];

  if (accs_configs!=nullptr) { // check if search was successful
    m_pool.clear();
    for (auto it = accs_configs.begin(); it != accs_configs.end(); ++it) {
      auto acc_config = *it;
      const std::string acc_name = acc_config["acc"];
      if (xacc::hasAccelerator(acc_name)) {
        auto qpu = xacc::getAccelerator(acc_name);
        m_pool.emplace_back(qpu);
      }
    }
    //std::cout << "There are " << m_pool.size() << " workers in the executor pool!\n";
  }
  else {
    throw std::invalid_argument("Could not find accs dictionary in configuration file/string. Please check your input!");
  }
}

std::shared_ptr<xacc::Accelerator> Executor::getNextAvailableQpu() {
  std::shared_ptr<xacc::Accelerator> acc;
  while(true) {
    {
      const std::scoped_lock guard(m_mutex);
      if (!m_pool.empty()) {
        acc = m_pool.back();
        m_pool.pop_back();
      }
    }
    if (acc) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return acc;
}

void Executor::release(std::shared_ptr<xacc::Accelerator> acc) {
  const std::scoped_lock guard(m_mutex);
  m_pool.push_back(acc);
}

Handle post(Executor &executor,
            std::shared_ptr<xacc::CompositeInstruction> program, int shots) {
  return thread_pool::submit([=, &executor]() {
    auto acc = executor.getNextAvailableQpu();
    acc->updateConfiguration({{"shots", shots}});
    std::shared_ptr<xacc::AcceleratorBuffer> buffer =
        xacc::qalloc(program->nPhysicalBits());
    acc->execute(buffer, program);
    executor.release(acc);
    buffer->addExtraInfo("qpu", acc->name());
    return buffer;
  });
}

std::string sync(Handle &handle, int time_out_secs) {
  std::shared_ptr<xacc::AcceleratorBuffer> result_buffer = handle.get();
  return result_buffer->toString();
}

}
