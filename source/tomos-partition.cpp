#include "tomos/tomos-partition.hpp"

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
            std::cout << nodes.size() << std::endl;
            if (nodes.size() > limit) { return false; }
        }
        return true;
    }
} // namespace partition
} // namespace tomos
