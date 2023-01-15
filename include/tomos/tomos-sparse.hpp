#ifndef TOMOS_SPARSE_HPP__
#define TOMOS_SPARSE_HPP__

#include <mesh/mesh.hpp>
#include "tomos-metis.hpp"

namespace tomos {
namespace sparse {
    using Index         = std::size_t;
    using Indices       = std::vector<Index>;
    using Coordinate    = std::pair<Index, Index>;

    template <typename Precision>
    std::size_t
    nonzeros(const metis::Nodal<Precision>& nodal) {
        metis::Adjacency adjacency  = nodal.adjacency();
        std::size_t count           = 0;

        for (const auto& [_, neighbours] : adjacency) { count += (1 + neighbours.size()); }
        return count;
    }

    template <typename Precision>
    std::size_t
    nonzeros(const mesh::Mesh<Precision>& mesh) {
        metis::Nodal<Precision> nodal(mesh);
        return nonzeros(nodal);
    }

    template <typename Precision>
    std::pair<Indices, Indices>
    csr(const metis::Nodal<Precision>& nodal) {
        metis::Adjacency adjacency = nodal.adjacency();

        Indices rows   = {0};
        Indices cols   = {};

        for (const auto& [key, neighbours] : adjacency) {
            cols.push_back(key - 1);
            for(const auto& neighbour : neighbours) { cols.push_back(neighbour - 1); }
            rows.push_back(1 + neighbours.size() + rows.back());
        }

        return {cols, rows};
    }

    template <typename Precision>
    std::pair<Indices, Indices>
    csr(const mesh::Mesh<Precision>& mesh) {
        metis::Nodal<Precision> nodal(mesh);
        return csr(nodal);
    }

    template <typename Precision>
    std::map<Coordinate, Index>
    coo(const metis::Nodal<Precision>& nodal) {
        metis::Adjacency adjacency = nodal.adjacency();
        std::size_t count = 0;

        std::map<Coordinate, Index> ps;
        for (const auto& [key, neighbours] : adjacency) {
            std::vector<metis::Index> xs{key};
            xs.insert(xs.end(), neighbours.begin(), neighbours.end());

            for (std::size_t i = 0; i < xs.size(); i++) {
                auto [_, inserted] =  ps.insert({{key, xs[i]}, count});
                if (inserted) { count++; }
            }
        }
        return ps;
    }

    template <typename Precision>
    std::map<Coordinate, Index>
    coo(const mesh::Mesh<Precision>& mesh) {
        metis::Nodal<Precision> nodal(mesh);
        return coo(nodal);
    }
} // namespace sparse
} // namespace tomos

#endif // TOMOS_SPARSE_HPP__
