#include "tomos/tomos-partition.hpp"
#include "tomos/tomos-metis.hpp"

namespace tomos {
namespace partition {
    bool
    valid(const tomos::mesh::Mesh& mesh, const Partitions& ps, std::size_t limit) {
        std::map<Partition, std::size_t> nodal;

        for (const auto& [p, indices] : ps) {
            using tomos::mesh::node::Number;

            std::set<Number> nodes;
            for (const Index& index : indices) {
                const tomos::mesh::Element& element = mesh.elements[index];
                for (const Number& node : element.nodes) { nodes.insert(node); }
            }
            if (nodes.size() > limit) { return false; }
        }
        return true;
    }

    using Bounds = std::pair<std::size_t, std::size_t>;

    Bounds
    guess(metis::Dual& dual, const tomos::mesh::Mesh& mesh, std::size_t limit) {
        std::size_t prev = 1;
        std::size_t next = 2;

        while (true) {
            Partitions ps = dual.partition(next);
            if (valid(mesh, ps, limit)) {
                return {prev, next}; 
            } else {
                prev = next;
                next = 2 * prev;
            }
        }
        return {prev, next}; 
    }

    bool
    complete(const Bounds& bounds) {
        return (bounds.first >= bounds.second) 
            or ((bounds.second - bounds.first) == 1)
            ;
    }

    Bounds
    bissect(
              metis::Dual&              dual
            , const tomos::mesh::Mesh&  mesh
            , Bounds                    bounds
            , std::size_t               limits
           )
    {
        std::size_t mean = (bounds.second + bounds.first) / 2;
        if (valid(mesh, dual.partition(mean), limits)) {
            return {bounds.first, mean};
        } else {
            return {mean, bounds.second};
        }
    }

    std::size_t
    optimal(const tomos::mesh::Mesh& mesh, std::size_t limit) {
        if (mesh.nodes.size() < limit) { 
            return 1; 
        } else {
            metis::Dual dual(mesh, metis::Common::EDGE);

            Bounds bounds = guess(dual, mesh, limit);
            while(not complete(bounds)) {
                bounds = bissect(dual, mesh, bounds, limit);
            }
            return bounds.second;
        }
    }
} // namespace partition
} // namespace tomos
