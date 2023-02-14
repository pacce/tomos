#ifndef TOMOS_SPARSE_HPP__
#define TOMOS_SPARSE_HPP__

#include "tomos-metis.hpp"

namespace tomos {
namespace sparse {
    using Index         = std::size_t;
    using Indices       = std::vector<Index>;
    using Coordinate    = std::pair<Index, Index>;

    std::size_t
    nonzeros(const metis::Nodal& nodal);

    std::pair<Indices, Indices>
    csr(const metis::Nodal& nodal);

    std::map<Coordinate, Index>
    coo(const metis::Nodal& nodal);
} // namespace sparse
} // namespace tomos

#endif // TOMOS_SPARSE_HPP__
