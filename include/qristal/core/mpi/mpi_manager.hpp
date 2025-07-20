#pragma once

#include <qristal/core/mpi/message_types.hpp>
#include <qristal/core/wait_until.hpp>

#include <mpi.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ranges>
#include <span>
#include <stdexcept>
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace {

// See page 34 of https://www.mpi-forum.org/docs/mpi-4.1/mpi41-report.pdf
const std::unordered_map<std::type_index, MPI_Datatype> MPI_TYPE_MAP = {
    {typeid(char), MPI_CHAR},       {typeid(int16_t), MPI_INT16_T},
    {typeid(int32_t), MPI_INT32_T}, {typeid(uint32_t), MPI_UINT32_T},
    {typeid(int64_t), MPI_INT64_T}, {typeid(float), MPI_FLOAT},
    {typeid(double), MPI_DOUBLE}};

} // namespace

namespace qristal::mpi {

template <typename TCallback, typename TData>
concept ReceiveDataCallback =
    requires(TCallback callback, int32_t i, std::span<TData> dataSpan) {
      { callback(i, dataSpan) } -> std::same_as<void>;
    };

/**
 * @brief Manager class for MPI functionality (not thread-safe)
 *
 * This class is a simple wrapper around the MPI API designed for use by an
 * MPI architecture whereby the supervisor process, rank 0, broadcasts all
 * messages to worker processes once at the start of application, runs its
 * workload and then waits for all other processes to finish so it can receive
 * and collate the results.
 *
 * https://mpitutorial.com/tutorials is a good resource for MPI semantics.
 *
 * @warning This class does not use the thread-safe initialisation function
 * (MPI_Init_thead(..., MPI_THREAD_MULTIPLE, ...) available in MPI. This
 * is known to not be performant due to extra locks required and Qristal's
 * current use-case does not need this functionality.
 */
class MpiManager {
private:
  std::vector<std::byte> receive_buffer_;
  int32_t mpi_process_id_;
  int32_t total_processes_;

  /**
   * @brief Waits for a message of the input message tag for the input MPI
   * process id and then returns the size of it so it can be received into a
   * buffer later
   *
   * @exception std::runtime_error Thrown if no message is ready to be received
   * within 30 seconds
   *
   * @tparam TData The data type of the message to wait for. Must be a
   * compatible MPI data type. See MPI_TYPE_MAP
   * @param message_tag The message type to wait for
   * @param process_id The MPI process id to probe
   * @return int32_t The size of the message
   */
  template <typename TData>
  int32_t get_awaiting_message_size(int32_t process_id,
                                    MessageTags message_tag) {
    if (process_id == mpi_process_id_) {
      // Skip probing if there's data to receive from this process
      return 0;
    }

    MPI_Status status;

    using namespace std::chrono_literals;
    bool message_ready = wait_until(
        [process_id, message_tag, &status]() {
          int message_ready = 0;
          MPI_Iprobe(process_id, static_cast<int>(message_tag), MPI_COMM_WORLD,
                     &message_ready, &status);

          return message_ready == 1;
        },
        30s);

    if (!message_ready) {
      std::ostringstream oss;
      oss << "Timed out while probing for message from MPI process with ID: "
          << process_id << " and Tag: " << static_cast<int>(message_tag)
          << " (enumerator value of enum class MessageTags)";
      throw std::runtime_error(oss.str());
    }
    int32_t data_size;
    MPI_Get_count(&status, MPI_TYPE_MAP.at(typeid(TData)), &data_size);

    return data_size;
  }

  /**
   * @brief Gets a view of the object's receive buffer resized to the correct
   * size as determined by the input argument.
   *
   * @tparam TData The data type with which to view the buffer
   * @param num_elements The number elements of TData to resize the buffer to
   * @return std::span<TData> A view into the buffer
   */
  template <typename TData>
  std::span<TData> get_receive_buffer(size_t num_elements) {
    const size_t memory_to_allocate = num_elements * sizeof(TData);
    try {
      receive_buffer_.resize(memory_to_allocate);
    } catch (const std::bad_alloc &e) {
      std::stringstream ss;
      ss << "Unable to allocate " << memory_to_allocate
         << "B of extra memory to receive data from other MPI processes.\n";
      std::fputs(ss.str().c_str(), stderr);
      throw e;
    }

    return {reinterpret_cast<TData *>(receive_buffer_.data()), num_elements};
  }

public:
  MpiManager();
  MpiManager(int32_t *argc, char ***argv);

  /**
   * @brief Get the current process's id
   *
   * @return decltype(mpi_process_id_) The current process's id
   */
  decltype(mpi_process_id_) get_process_id() const { return mpi_process_id_; }

  /**
   * @brief Get the total number of MPI processes
   *
   * @return decltype(total_processes_) The total number of MPI processes
   */
  decltype(total_processes_) get_total_processes() const { return total_processes_; }

  /**
   * @brief Sends data to the supervisor MPI process
   *
   * @tparam TData The data type to send. Must be a compatible MPI data
   * type. See MPI_TYPE_MAP
   * @param data_buffer The data to send over MPI
   * @param message_tag The type of message to send
   */
  template <typename TData>
  void send_to_supervisor(std::span<TData> data_buffer,
                          MessageTags message_tag) {
    MPI_Send(data_buffer.data(), data_buffer.size(),
             MPI_TYPE_MAP.at(typeid(TData)), 0, static_cast<int>(message_tag),
             MPI_COMM_WORLD);
  }

  /**
   * @brief Receives data from all other MPI processes.
   * This function is a synchronous receive function. Data from each MPI
   * process is received into an internally managed input buffer. The ID of the
   * process and the received message buffer are then given to the inputted
   * callback function so the caller can use the received data.
   *
   * @exception std::runtime_error Thrown if no message is received within 30
   * seconds
   *
   * @tparam TData The data type to receive. Must be a compatible MPI data
   * type. See MPI_TYPE_MAP
   * @tparam TCallback The type of the callback that's called once data has been
   * received.
   * @param message_tag The message type to receive
   * @param receive_data_callback The callback to run after data has been
   * received from a process
   */
  template <typename TData, ReceiveDataCallback<TData> TCallback>
  void receive_from_others(MessageTags message_tag,
                           TCallback &&receive_data_callback) {
    auto other_process_ids =
        std::views::iota(0, total_processes_) |
        std::views::filter([this](auto id) { return mpi_process_id_ != id; });

    // Memory usage can be quite high for certain quantum calculation
    // configurations. Data is received in serial to minimise the amount of
    // extra memory that is required to sync results from other processes.
    MPI_Datatype type_to_receive = MPI_TYPE_MAP.at(typeid(TData));
    std::ranges::for_each(other_process_ids, [this, message_tag,
                                              type_to_receive,
                                              &receive_data_callback](auto id) {
      int32_t message_size = get_awaiting_message_size<TData>(id, message_tag);
      if (message_size > 0) {
        std::span<TData> receive_buffer =
            get_receive_buffer<TData>(message_size);
        MPI_Status status;
        MPI_Recv(receive_buffer.data(), receive_buffer.size(), type_to_receive,
                 id, static_cast<int>(message_tag), MPI_COMM_WORLD, &status);

        receive_data_callback(id, receive_buffer);
      }
    });
  }
};

} // namespace qristal::mpi
