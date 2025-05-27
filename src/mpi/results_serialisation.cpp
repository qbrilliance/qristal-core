#include <qristal/core/mpi/results_serialisation.hpp>

#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/transform.hpp>

namespace qristal::mpi::serialisation {

std::vector<ResultsType> pack_results_map(const ResultsMap &results_map) {
  if (results_map.size() == 0) {
    return {};
  }

  // Pack the map into a ResultsType array
  auto packed_data_view =
      results_map |
      ::ranges::views::transform([](const auto &result_pair) {
        const auto &[bool_vec, count] = result_pair;

        // Pack the vector of bools into a ResultsType array
        auto packed_bools_view =
            bool_vec |
            // Chunk is used to split the results vector of bools into
            // ResultsType sized chunks. This creates a view into the
            // results vector which can be evaluated to a 2D array where the
            // inner array elements are ResultsType bools in length.
            ::ranges::views::chunk( // range-v3's chunk is used because
                                    // std::views::chunk is not
                                    // available until C++23
                MPI_ARRAY_ELEMENT_BITS) |
            ::ranges::views::transform( // range-v3's chunk is not compatible
                                        // with std::views::transform so
                                        // the range-v3 version is used here
                [](const auto &chunk) {
                  // Pack each chunk (view of MPI_ARRAY_ELEMENT_BITS bools)
                  // into a ResultsType
                  return std::accumulate(chunk.begin(), chunk.end(),
                                         static_cast<ResultsType>(0),
                                         [](ResultsType packed_bool, bool bit) {
                                           return (packed_bool << 1) |
                                                  static_cast<ResultsType>(bit);
                                         });
                });

        // The size of the packed array needs to be added before the array of
        // packed bools so that they can be unpacked. This acts as the delimiter
        // for each result
        ResultsType packed_bools_view_size =
            std::ranges::distance(packed_bools_view);

        // Add the size of the packed bools array to the start of the packed
        // data view
        return ::ranges::views::concat(
            ::ranges::views::single(packed_bools_view_size), packed_bools_view,
            ::ranges::views::single(count));
      })
      // At this point, the results map has been transformed into a view that
      // can be evaluated into an array of an array of ResultsType. This
      // needs to be flattened into a one-dimensional array (contiguous memory
      // space) so that it can be sent over MPI.
      | ::ranges::views::join;

  // Add number of bools per key to the start of the packed data and return
  return ::ranges::views::concat(
             ::ranges::views::single(results_map.begin()->first.size()),
             packed_data_view) |
         ::ranges::to<std::vector<ResultsType>>;
}

std::vector<GradientsType> pack_gradients(OutProbabilityGradients &gradients) {
  if (gradients.size() == 0) {
    return {};
  }

  size_t outer_vec_size = gradients.size();
  size_t inner_vec_size = gradients.front().size();

  // Reserve memory in the output vector
  constexpr auto prefix_elements = 2;
  std::vector<GradientsType> packed_gradients;
  packed_gradients.reserve(prefix_elements + outer_vec_size * inner_vec_size);

  // Add the sizes to the start of the packed vector
  packed_gradients.push_back(static_cast<GradientsType>(outer_vec_size));
  packed_gradients.push_back(static_cast<GradientsType>(inner_vec_size));

  // Flatten and add the gradients to the packed vector
  auto flattened = gradients | std::views::join;
  std::ranges::copy(flattened, std::back_inserter(packed_gradients));

  return packed_gradients;
}

} // namespace qristal::mpi::serialisation