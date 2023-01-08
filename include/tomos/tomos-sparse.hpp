#ifndef TOMOS_SPARSE_HPP__
#define TOMOS_SPARSE_HPP__

#include <mesh/mesh.hpp>
#include "tomos-metis.hpp"

namespace tomos {
namespace sparse {
    using Index     = std::size_t;
    using Indices   = std::vector<Index>;

    template <typename Precision>
    std::size_t
    nonzeros(const mesh::Mesh<Precision>& mesh) {
        std::size_t count           = 0;
        metis::Adjacency adjacency  = tomos::metis::nodal(mesh);

        for (const auto& [_, neighbours] : adjacency) { count += (1 + neighbours.size()); }
        return count;
    }

    template <typename Precision>
    std::pair<Indices, Indices>
    csr(const mesh::Mesh<Precision>& mesh) {
        metis::Adjacency adjacency      = tomos::metis::nodal(mesh);
        Indices rows   = {0};
        Indices cols   = {};

        for (const auto& [key, neighbours] : adjacency) { 
            cols.push_back(key - 1);
            for(const auto& neighbour : neighbours) { cols.push_back(neighbour - 1); }
            rows.push_back(1 + neighbours.size() + rows.back());
        }

        return {cols, rows};
    }
} // namespace sparse
} // namespace tomos

#endif // TOMOS_SPARSE_HPP__
