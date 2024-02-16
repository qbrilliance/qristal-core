#include "qb/core/tools/UpperTriangleIterator.hpp"

namespace qb {
    namespace tools {


        UpperTriangleIterator::UpperTriangleIterator(const size_t& maxN, const size_t& dim) :
            std::vector<size_t>(dim), _maxN(maxN) {
                this->init(0);
            }

        //initialize to 01234... 
        void UpperTriangleIterator::init(size_t j) {
            for ( ; j+1 < this->size(); j++)
            (*this)[j+1] = (*this)[j] + 1;
        }

        UpperTriangleIterator & UpperTriangleIterator::operator ++ () {
            int j = this->size();
            if (j == 0)
                return *this;
            --j;
            while ( ++(*this)[j] == 1 + _maxN + (j - static_cast<int>(this->size())) ) //increment j as long as it is not last
            if ( --j < 0 ) //go to previous index
            return *this; //if there is no further previous index: end
            init(j);
            return *this;
        }


        bool UpperTriangleIterator::valid() const {
            if (this->size() == 0)
                return false;
            
            return this->back() < _maxN;	
        }

        std::ostream & operator << (std::ostream & os, const UpperTriangleIterator& uti)
        {
            os << "[";
            for ( size_t i=0 ; i<uti.idx().size() ; ++i )
            {
                if ( i>0 )
                    os << " ";
                os << uti.idx()[i];
            }
            os << "]";
            return os;
        }
    }
}