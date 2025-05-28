#include <cstdlib>
#include <qristal/core/mpi/results_serialisation.hpp>
#include <qristal/core/mpi/results_types.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/zip.hpp>

#include <algorithm>
#include <concepts>
#include <random>
#include <ranges>

using RessultsElementType = qristal::mpi::serialisation::ResultsType;

namespace {
static std::mt19937_64 random_engine{std::random_device{}()};

template <typename Number>
  requires std::integral<Number> || std::floating_point<Number>
Number random_number(Number start = std::numeric_limits<Number>::min(),
                     Number end = std::numeric_limits<Number>::max()) {
  if constexpr (std::integral<Number>) {
    std::uniform_int_distribution<Number> distribution(start, end);
    return distribution(random_engine);
  } else if constexpr (std::floating_point<Number>) {
    std::uniform_real_distribution<Number> distribution(start, end);
    return distribution(random_engine);
  }
}

} // namespace

// Tests packing and unpacking a map whose keys are std::vector<bool> and values
// are int32_t via qristal::mpi::serialisation::pack_results_map().

/**
 * @brief Test packing and unpacking a map with zero elements
 *
 */
TEST(SerialisationTester, PackAndUnpackZeroElementMap) {
  qristal::mpi::ResultsMap results_map;

  // Serialise and check
  auto packed_map = qristal::mpi::serialisation::pack_results_map(results_map);

  EXPECT_EQ(packed_map.size(), 0); // No elements to pack

  // Deserialise and check
  int cb_called = 0;
  qristal::mpi::serialisation::unpack_results_map(
      packed_map | std::ranges::views::all,
      [&results_map,
       &cb_called](std::vector<bool> key,
                   qristal::mpi::serialisation::ResultsType value) {
        results_map[key] += value;
        ++cb_called;
      });
  EXPECT_EQ(results_map.size(), 0); // Map should still be empty
  EXPECT_EQ(cb_called, 0);          // Callback should not have been called
}

/**
 * @brief In this test, the map's keys fit into a single element of the packed
 * array. I.e. the length of the bool array (MPI_ARRAY_ELEMENT_BITS) is less
 * than the size of the packed element (int32_t).
 *
 */
TEST(SerialisationTester, PackAndUnpackMapMultipleEntriesKeysInOneElement) {
  // Setup test data
  const auto key1 = std::vector<bool>{1, 0, 1, 0};
  constexpr RessultsElementType key1_serialised = 0b1010;
  constexpr RessultsElementType value1 = 2456;
  const auto key2 = std::vector<bool>{0, 1, 0, 1};
  constexpr RessultsElementType key2_serialised = 0b0101;
  constexpr RessultsElementType value2 = 76;

  // Create the map to serialise
  qristal::mpi::ResultsMap results_map{{key1, value1}, {key2, value2}};

  // Serialise
  auto packed_map = qristal::mpi::serialisation::pack_results_map(results_map);

  // Check the map has been serialised correctly
  EXPECT_EQ(packed_map.size(), 7);
  EXPECT_EQ(packed_map[0], 4); // Original size of the vector<bool> map key
  // Note: key2 and value 2 will be serialised first because this is an ordered
  // map and the key for entry 2 (0101) is less than entry 1 (1010)
  EXPECT_EQ(packed_map[1], 1);               // Size of array for entry 2
  EXPECT_EQ(packed_map[2], key2_serialised); // Packed bools for entry 2
  EXPECT_EQ(packed_map[3], value2);          // Count for entry 2
  EXPECT_EQ(packed_map[4], 1);               // Size of array for entry 1
  EXPECT_EQ(packed_map[5], key1_serialised); // Packed bools for entry 1
  EXPECT_EQ(packed_map[6], value1);          // Count for entry 1

  // The original map is used to test adding an element where a key doesn't
  // exist and adding to an existing element where a key does exist
  results_map.erase(key2); // Remove an entry to test adding to the map where an
                           // element with the same key doesn't exist

  // Deserialise
  int cb_called = 0;
  qristal::mpi::serialisation::unpack_results_map(
      packed_map | std::ranges::views::all,
      [&results_map,
       &cb_called](std::vector<bool> key,
                   qristal::mpi::serialisation::ResultsType value) {
        results_map[key] += value;
        ++cb_called;
      });

  // Check everything has been deserialised correctly
  EXPECT_EQ(results_map.size(), 2);
  EXPECT_EQ(results_map.size(), cb_called);
  EXPECT_EQ(results_map.at(key1), value1 * 2);
  EXPECT_EQ(results_map.at(key2), value2);
}

/**
 * @brief In this test, the map's keys do not fit into a single element of the
 * packed array. I.e. the length of the bool array (MPI_ARRAY_ELEMENT_BITS) is
 * greater than the size of the packed element (int32_t).
 *
 */
TEST(SerialisationTester,
     PackAndUnpackMapMultipleEntriesKeysInMoreThanOneOneElement) {
  // Setup test data
  const auto key1 =
      std::vector<bool>{1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
                        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1};
  constexpr RessultsElementType key1_serialised_part1 =
      0b10111010101010101010101010101010;
  constexpr RessultsElementType key1_serialised_part2 = 0b11101;
  constexpr RessultsElementType value1 = 1234;
  const auto key2 =
      std::vector<bool>{0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0};
  constexpr RessultsElementType key2_serialised_part1 =
      0b01011101010101010101010101010101;
  constexpr RessultsElementType key2_serialised_part2 = 0b01110;
  constexpr RessultsElementType value2 = 5678;

  // Create the map to serialise
  qristal::mpi::ResultsMap results_map{{key1, value1}, {key2, value2}};

  // Serialise
  auto packed_map = qristal::mpi::serialisation::pack_results_map(results_map);

  // Check the map has been serialised correctly
  EXPECT_EQ(packed_map.size(), 9);
  EXPECT_EQ(packed_map[0], 37); // Original size of the vector<bool> map key
  // Note: key2 and value 2 will be serialised first because this is an ordered
  // map and the key for entry 2 is less than entry 1
  EXPECT_EQ(packed_map[1], 2); // Size of array for entry 2 (37 bits)
  EXPECT_EQ(packed_map[2],
            key2_serialised_part1); // Packed bools for entry 2 (first 32 bits)
  EXPECT_EQ(
      packed_map[3],
      key2_serialised_part2); // Packed bools for entry 2 (remaining 5 bits)
  EXPECT_EQ(packed_map[4], value2); // Count for entry 1
  EXPECT_EQ(packed_map[5], 2);      // Size of array for entry 1 (37 bits)
  EXPECT_EQ(packed_map[6],
            key1_serialised_part1); // Packed bools for entry 1 (first 32 bits)
  EXPECT_EQ(
      packed_map[7],
      key1_serialised_part2); // Packed bools for entry 1 (remaining 5 bits)
  EXPECT_EQ(packed_map[8], value1); // Count for entry 1

  // Deserialise
  int cb_called = 0;
  qristal::mpi::serialisation::unpack_results_map(
      packed_map | std::ranges::views::all,
      [&results_map,
       &cb_called](std::vector<bool> key,
                   qristal::mpi::serialisation::ResultsType value) {
        results_map[key] += value;
        ++cb_called;
      });

  // Check everything has been deserialised correctly
  EXPECT_EQ(results_map.size(), 2);
  EXPECT_EQ(results_map.size(), cb_called);
  // The entries already exist in the map so their values should now be doubled
  // (unpacking should sum the values already in the map)
  EXPECT_EQ(results_map.at(key1), value1 * 2);
  EXPECT_EQ(results_map.at(key2), value2 * 2);
}

/**
 * @brief Test packing and unpacking a 2D vector with zero elements
 *
 */
TEST(SerialisationTester, PackAndUnpackEmpty2dVector) {
  qristal::mpi::OutProbabilityGradients input_gradients;

  // Serialise and check
  auto packed_gradients =
      qristal::mpi::serialisation::pack_gradients(input_gradients);

  EXPECT_EQ(packed_gradients.size(), 0);

  // Try to deserialise
  auto unpacked_gradients =
      qristal::mpi::serialisation::unpack_gradients(packed_gradients);

  // range-v3 v0.12.0 has a bug with chunk_view when it is empty. This causes a
  // divide by zero floating point core dump when its internal `size_()` member
  // method is called. As a workaround proxy check to see whether the returned
  // view is empty, this test checks for a floating point divide by zero SIGFPE
  // process exit in a forked process. This is the only way to detect this
  // failure in a gtest. There is also a different exception thrown (less
  // specific) when checking whether it is empty via its `empty()` method or
  // iterating through the view.
  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "Fork failed";

  if (pid == 0) {
    // Child process: cause SIGFPE
    EXPECT_EQ(::ranges::distance(unpacked_gradients), 0);
    _exit(0); // Should not reach here
  } else {
    // Parent process: wait for child
    int status;
    waitpid(pid, &status, 0);

    // Check if child terminated due to SIGFPE
    ASSERT_TRUE(WIFSIGNALED(status));
    EXPECT_EQ(WTERMSIG(status), SIGFPE);
  }
}

/**
 * @brief This test checks that pack_gradients() and unpack_gradients() can
 * correctly serialise and deserialise a 2D vector.
 *
 */
TEST(SerialisationTester, PackAndUnpack2DVector) {
  constexpr auto num_rows = 20;
  constexpr auto num_columns = 10;

  // Create the 2D vector to serialise
  qristal::mpi::OutProbabilityGradients input_gradients{num_rows};

  // Setup test data
  std::ranges::for_each(input_gradients, [](auto &row) {
    for (auto i = 0; i < num_columns; ++i) {
      row.emplace_back(random_number(-1.0, 1.0));
    }
  });

  // Serialise
  auto packed_gradients =
      qristal::mpi::serialisation::pack_gradients(input_gradients);

  // Check the map has been serialised correctly
  constexpr auto packed_padding = 2;
  EXPECT_EQ(packed_gradients.size(), num_rows * num_columns + packed_padding);
  EXPECT_EQ(packed_gradients.at(0), num_rows);
  EXPECT_EQ(packed_gradients.at(1), num_columns);

  auto flattened_input_gradients =
      input_gradients | std::views::join |
      ::ranges::to<std::vector<qristal::mpi::Probability>>;
  auto packed_gradients_values =
      packed_gradients | ::ranges::views::drop(packed_padding) |
      ::ranges::to<std::vector<qristal::mpi::Probability>>;
  EXPECT_THAT(flattened_input_gradients,
              testing::ContainerEq(packed_gradients_values));

  // Deserialise
  auto unpacked_gradients =
      qristal::mpi::serialisation::unpack_gradients(packed_gradients) |
      ::ranges::to<std::vector<std::vector<qristal::mpi::Probability>>>;

  // Check the data has been deserialised correctly
  EXPECT_THAT(input_gradients, testing::ContainerEq(unpacked_gradients));
}
