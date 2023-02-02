#include <iostream>
#include <complex>
#include "qb/core/optimization/qaoa/autodiff/forward.hpp"

#if defined(__clang__)
namespace std {
template <>
complex<autodiff::dual> operator*(const complex<autodiff::dual> &__z,
                                  const complex<autodiff::dual> &__w) {
  autodiff::dual __a = __z.real();
  autodiff::dual __b = __z.imag();
  autodiff::dual __c = __w.real();
  autodiff::dual __d = __w.imag();
  autodiff::dual __ac = __a * __c;
  autodiff::dual __bd = __b * __d;
  autodiff::dual __ad = __a * __d;
  autodiff::dual __bc = __b * __c;
  autodiff::dual __x = __ac - __bd;
  autodiff::dual __y = __ad + __bc;
  return complex<autodiff::dual>(__x, __y);
}
} // namespace std
#endif
