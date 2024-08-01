// Copyright Quantum Brilliance Pty Ltd

#pragma once

#include <vector>
#include <iostream>

namespace qristal {
    namespace tools {

        //an iterator for unique tuples i < j < k < ... of dimension dim from a set of maxN elements
        class UpperTriangleIterator : private std::vector<size_t> {
            public:

                UpperTriangleIterator(const size_t& maxN, const size_t& dim);

                UpperTriangleIterator & operator ++ (); // increment iterator
                bool valid() const; // returns true if iterator still valid
                const std::vector<size_t>& idx() const {
                    return static_cast<const std::vector<size_t>&>(*this);
                }
                const size_t& maxN() const {
                    return _maxN;
                }

            private:

                void init(size_t j);
                size_t _maxN;

        };

        std::ostream & operator << (std::ostream & os, const UpperTriangleIterator& uti);

    }
}
