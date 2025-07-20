#include <qristal/core/mpi/mpi_manager.hpp>
#include <qristal/core/session.hpp>

#include <gtest/gtest.h>
#include <yaml-cpp/node/node.h>

#include <stdexcept>
#include <string>

struct test_data {
  qristal::session session;
  YAML::Node remote_backend_database;
};

test_data make_test_data(std::string_view accelerator, std::vector<std::string> mpi_accelerators) {
  qristal::session session;
  session.acc = accelerator;
  session.mpi_hardware_accelerators = std::move(mpi_accelerators);

  YAML::Node remote_backends;
  for (const std::string &accelerator : session.mpi_hardware_accelerators) {
    remote_backends[accelerator] = 0;
  }

  return {.session = session, .remote_backend_database = remote_backends};
}

TEST(SessionConfigurationTester, SingleProcessEmptyMpiHardwareAccelerators) {
  test_data data = make_test_data("aer", {});
  std::string acc_before = data.session.acc;
  EXPECT_NO_THROW(qristal::validate_mpi_config(1, 0, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
  EXPECT_EQ(data.session.acc, acc_before);
}

TEST(SessionConfigurationTester, MultiProcessEmptyMpiHardwareAccelerators) {
  test_data data = make_test_data("aer", {});
  std::string acc_before = data.session.acc;
  EXPECT_NO_THROW(qristal::validate_mpi_config(4, 1, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
  EXPECT_EQ(data.session.acc, acc_before);
}

TEST(SessionConfigurationTester, SingleProcessPopulatedMpiHardwareAcceleratorsLocalSimulator) {
  test_data data = make_test_data("aer", {"qpp", "hardware1"});
  YAML::Node remote_backend_database;
  remote_backend_database[data.session.mpi_hardware_accelerators[1]] = 0;
  EXPECT_THROW(qristal::validate_mpi_config(1, 0, data.session.mpi_hardware_accelerators, data.session.acc,
                                            remote_backend_database),
               std::runtime_error);
}

TEST(SessionConfigurationTester, MultiProcessPopulatedMpiHardwareAcceleratorsLocalSimulator) {
  test_data data = make_test_data("aer", {"qpp", "hardware1"});
  YAML::Node remote_backend_database;
  remote_backend_database[data.session.mpi_hardware_accelerators[1]];
  EXPECT_THROW(qristal::validate_mpi_config(2, 0, data.session.mpi_hardware_accelerators, data.session.acc,
                                            remote_backend_database),
               std::runtime_error);
}

TEST(SessionConfigurationTester, SingleProcessPopulatedMpiHardwareAcceleratorsRemoteHardware) {
  test_data data = make_test_data("aer", {"hardware1", "hardware2"});
  EXPECT_NO_THROW(qristal::validate_mpi_config(1, 0, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
  EXPECT_EQ(data.session.acc, "hardware1");
}

TEST(SessionConfigurationTester, MultiProcessPopulatedMpiHardwareAcceleratorsRemoteHardware) {
  test_data data = make_test_data("aer", {"hardware1", "hardware2"});
  EXPECT_NO_THROW(qristal::validate_mpi_config(2, 1, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
  EXPECT_EQ(data.session.acc, "hardware2");
}

TEST(SessionConfigurationTester, MoreProcessesThanAccelerators) {
  test_data data = make_test_data("aer", {"hardware1", "hardware2"});
  EXPECT_THROW(qristal::validate_mpi_config(3, 0, data.session.mpi_hardware_accelerators, data.session.acc,
                                            data.remote_backend_database),
               std::runtime_error);
}

TEST(SessionConfigurationTester, ProcessesEqualsAccelerators) {
  test_data data = make_test_data("aer", {"hardware1", "hardware2"});
  EXPECT_NO_THROW(qristal::validate_mpi_config(2, 1, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
}

TEST(SessionConfigurationTester, FewerProcessesThanAccelerators) {
  test_data data = make_test_data("aer", {"hardware1", "hardware2", "hardware3", "hardware4"});
  EXPECT_NO_THROW(qristal::validate_mpi_config(3, 1, data.session.mpi_hardware_accelerators, data.session.acc,
                                               data.remote_backend_database));
}

TEST(SessionConfigurationTester, ProcessSelectsCorrectMpiHardwareAcceleratorIndex) {
  std::vector<std::string> mpi_accelerators = {"hardware1", "hardware2", "hardware3"};
  for (size_t idx = 0; idx < mpi_accelerators.size(); ++idx) {
    test_data data = make_test_data("aer", mpi_accelerators);
    EXPECT_NO_THROW(qristal::validate_mpi_config(mpi_accelerators.size(), idx, data.session.mpi_hardware_accelerators,
                                                 data.session.acc, data.remote_backend_database));
    EXPECT_EQ(data.session.acc, mpi_accelerators[idx]) 
        << "This process should have selected \"" << mpi_accelerators[idx] << "\"";
  }
}
