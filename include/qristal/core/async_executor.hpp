// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include "Accelerator.hpp"
#include "CompositeInstruction.hpp"

#include <future>
#include <mutex>
#include <string>
#include <thread>

namespace qristal {

  using Handle = std::future<std::shared_ptr<xacc::AcceleratorBuffer>>;

  class Executor {
    private:
      std::vector<std::shared_ptr<xacc::Accelerator>> m_pool;
      std::mutex m_mutex;

    public:
      void initialize(const std::string &in_qpuConfig);
      std::shared_ptr<xacc::Accelerator> getNextAvailableQpu();
      void release(std::shared_ptr<xacc::Accelerator> acc);
  };

  Handle post(Executor &executor, std::shared_ptr<xacc::CompositeInstruction> program, int shots);

  std::string sync(Handle &handle, int time_out_secs = -1);

}
