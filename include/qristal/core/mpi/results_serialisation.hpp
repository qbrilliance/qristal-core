#include <qristal/core/mpi/results_types.hpp>

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <vector>

#include <range/v3/view/chunk.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip.hpp>

namespace qristal::mpi::serialisation {

using ResultsType = uint32_t;
using ProbabilitiesType = Probability;
using GradientsType = Probability;
using CountsType = Count;
using ShotCountType = int32_t;

namespace {

/**
 * @brief Number of bits per element to be sent over MPI
 */
constexpr auto MPI_ARRAY_ELEMENT_BITS = 8 * sizeof(ResultsType);

} // namespace

template <typename TRange>
concept ResultsTypeRange =
    std::ranges::range<TRange> &&
    std::same_as<std::ranges::range_value_t<TRange>, ResultsType>;

template <typename TRange>
concept GradientsTypeRange =
    std::ranges::range<TRange> &&
    std::same_as<std::ranges::range_value_t<TRange>, GradientsType>;

template <typename TRange>
using UnpackedGradientsView = ::ranges::chunk_view<::ranges::subrange<
    std::ranges::iterator_t<const TRange>,
    std::ranges::iterator_t<const TRange>, ::ranges::subrange_kind::sized>>;

template <typename TCallback>
concept MapUpdateCallback =
    requires(TCallback callback, std::vector<bool> vec, CountsType i) {
      { callback(vec, i) } -> std::same_as<void>;
    };

/**
 * @brief Pack the results map for sending over MPI
 *
 * @details The implementation uses std::views to keep serialisation overhead
 * as minimal as possible. The map's key, a vector of booleans storing a "result"
 * from one the calculations, is packed into an array of ResultsType. The output
 * transformation is a contiguous memory vector where each element of the map has
 * been packed into the following format:
 * ```
 * +-----------------------+-----------------------+-----------------------+
 * | ResultsType           | ResultsType[]         | ResultsType           |
 * | Size of results array | Packed results array  | Count                 |
 * |                       | (Map element's key)   | (Map element's value) |
 * +-----------------------+-----------------------+-----------------------+
 * ```
 *
 * @param results_map A map of results from a session object
 * @return std::vector<ResultsType> The packed results map
 */
std::vector<ResultsType> pack_results_map(const ResultsMap &results_map);

/**
 * @brief Unpacks a data stream previously packed with pack_results_map()
 *
 * @tparam TRange Range type for the packed_data
 * @tparam TCallback Callback type to be called to update with deserialised data
 * @param packed_data The serialised map data to unpack from a single process
 * @param map_update_callback The callback function to call once a key and value
 * have be deserialised
 *
 * @see pack_results_map()
 */
template <ResultsTypeRange TRange, MapUpdateCallback TCallback>
void unpack_results_map(TRange packed_data, TCallback &&map_update_callback) {
  if (std::ranges::distance(packed_data) == 0) {
    return;
  }

  std::ranges::iterator_t<const TRange> data_it = packed_data.begin();
  // Unpack bools into an intermediate vector. The size of this is the first
  // element of the data stream. It has static storage duration so that it is
  // only allocated once.
  static Qubits map_key;
  map_key.clear();
  map_key.resize(*data_it);

  // The rest of the map is serialised after this so advance the iterator
  std::advance(data_it, 1);
  while (data_it < packed_data.end()) {
    // Each iteration of this loop deserialises a single entry in the map
    ResultsType array_size = *data_it;
    std::advance(data_it, 1);
    auto array_view =
        std::ranges::subrange{data_it, std::next(data_it, array_size)};

    // Unpack the bools in the intermediary vector
    const auto i_array_view =
        ::ranges::views::zip(::ranges::views::iota(0), array_view);
    std::ranges::for_each(i_array_view, [](auto i_packed_bool) {
      const auto &[i, packed_bool] = i_packed_bool;
      int num_bools_to_unpack =
          map_key.size() - MPI_ARRAY_ELEMENT_BITS * i < MPI_ARRAY_ELEMENT_BITS
              ? map_key.size() - MPI_ARRAY_ELEMENT_BITS * i
              : MPI_ARRAY_ELEMENT_BITS;
      int num_bools_already_unpacked = i * MPI_ARRAY_ELEMENT_BITS;
      for (int j = num_bools_to_unpack + num_bools_already_unpacked - 1,
               k = num_bools_already_unpacked;
           j >= num_bools_already_unpacked; --j, ++k) {
        map_key[j] = (packed_bool >> k) & 1;
      }
    });

    // Get the map's value
    std::advance(data_it, array_size);
    CountsType count = *data_it;

    // Call the callback with the deserialised key and value
    map_update_callback(map_key, count);

    // Advance to the next map entry
    std::advance(data_it, 1);
  }
}

/**
 * @brief Pack the gradients for sending over MPI
 *
 * @details MPI requires contiguous memory for sending. The gradients are stored
 * as a vector of vector and whilst elements within a single vector are
 * guaranteed to be contiguous, a vector of vectors is not (as each element of
 * the outer vector is a vector object which will point to some other point in
 * memory which is not guaranteed to be immediately after the preceeding
 * element's pointed at memory). This function packs gradients so that each of
 * the inner vectors are contiguous with the previous with the addition of the
 * sizes of the outer and inner vectors from the original storage format.
 * ```
 * +-----------------------+-----------------------+-------------------------+
 * | GradientsType         | GradientsType         | GradientsType[]         |
 * | Size of outer vector  | Size of inner vectors | Gradients               |
 * +-----------------------+-----------------------+-------------------------+
 * ```
 *
 * @param gradients The data to pack
 * @return std::vector<GradientsArrayElement> The packed data
 */
std::vector<GradientsType> pack_gradients(OutProbabilityGradients &gradients);

/**
 * @brief Unpacks buffers that have been previously packed with pack_gradients()
 *
 * @tparam Range A view into views of buffers
 * @param packed_data The 2D array to unpack from a single process
 * @return UnpackedGradientsView<TRange> A view of the 2D vector of gradients
 * that can be further processed by the caller
 */
template <GradientsTypeRange TRange>
UnpackedGradientsView<TRange> unpack_gradients(const TRange &packed_data) {
  constexpr auto min_elements = 2; // The sizes of the input 2D range
  if (std::ranges::distance(packed_data) < min_elements) {
    return UnpackedGradientsView<TRange>{};
  }

  // Get the dimensions of the serialised 2D vector
  std::ranges::iterator_t<const TRange> data_it = packed_data.begin();
  uint32_t outer_vec_size = static_cast<uint32_t>(*data_it); // number of params
  uint32_t inner_vec_size = static_cast<uint32_t>(*std::next(data_it));

  // Convert the packed_data to a view of a 2D vector
  constexpr auto prefix_elements = 2;
  return packed_data | ::ranges::views::drop(prefix_elements) |
         ::ranges::views::chunk(inner_vec_size);
}

} // namespace qristal::mpi::serialisation