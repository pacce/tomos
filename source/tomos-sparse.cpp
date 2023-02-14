#include "tomos/tomos-sparse.hpp"

namespace tomos {
namespace sparse {
    std::size_t
    nonzeros(const metis::Nodal& nodal) {
        metis::Adjacency adjacency  = nodal.adjacency();
        std::size_t count           = 0;

        for (const auto& [_, neighbours] : adjacency) { count += (1 + neighbours.size()); }
        return count;
    }

    std::pair<Indices, Indices>
    csr(const metis::Nodal& nodal) {
        metis::Adjacency adjacency = nodal.adjacency();

        Indices rows   = {0};
        Indices cols   = {};

        for (const auto& [key, neighbours] : adjacency) {
            cols.push_back(key);
            cols.insert(cols.end(), neighbours.begin(), neighbours.end());
            rows.push_back(1 + neighbours.size() + rows.back());
        }

        return {cols, rows};
    }

    std::map<Coordinate, Index>
    coo(const metis::Nodal& nodal) {
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
} // namespace sparse
} // namespace tomos
