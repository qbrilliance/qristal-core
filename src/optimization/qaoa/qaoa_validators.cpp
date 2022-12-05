#include "qb/core/optimization/qaoa/qaoa_validators.hpp"

namespace qb {
namespace op {

// Specialisations - no validation is implemented for these classes
template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_lt_upperbound<NN>(
  const NN &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_lt_eq_upperbound<NN>(
  const NN &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_lt_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_lt_eq_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_lt_upperbound<NC>(
  const NC &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_lt_eq_upperbound<NC>(
  const NC &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_lt_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_lt_eq_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_gt_lowerbound<NN>(
  const NN &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_gt_eq_lowerbound<NN>(
  const NN &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_gt_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_gt_eq_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_gt_lowerbound<NC>(
  const NC &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_gt_eq_lowerbound<NC>(
  const NC &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_gt_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_gt_eq_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
) {
  return true;
}

}
}
