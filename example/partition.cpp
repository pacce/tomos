#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <tomos/tomos.hpp>
#include <tomos/tomos-mesh.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        tomos::mesh::Mesh mesh = tomos::mesh::decode(std::filesystem::path{argv[1]});

        std::size_t optimal = tomos::partition::optimal(mesh, 200);
        std::cout << "optimal: " << optimal << std::endl;

        tomos::metis::Dual dual(mesh, tomos::metis::Common::EDGE);
        tomos::metis::Partitions ps = dual.partition(optimal);

        for (const auto& [key, indices] : ps) { 
            using tomos::mesh::node::Number;
            using tomos::partition::Index;

            std::set<Number> nodes;
            for (const Index& index : indices) {
                const tomos::mesh::Element& element = mesh.elements[index];
                for (const Number& node : element.nodes) { nodes.insert(node); }
            }
            std::cout << key << " : " << nodes.size() << " nodes\n";
        }
        std::cout << std::flush;
    } catch (const std::exception& e) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
