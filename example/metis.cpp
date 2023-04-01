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

        tomos::metis::Dual dual(mesh, tomos::metis::Common::EDGE);

        for (const auto& [key, vs] : dual.adjacency()) {
            std::cout << key << " : ";
            for (const auto& v : vs) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        tomos::metis::Partitions ps = dual.partition(4);
        for (const auto& [key, vs] : ps) {
            std::cout << key << " : ";
            for (std::size_t i = 0; i < vs.size(); i++) {
                std::cout << vs[i];
                if (i < (vs.size() - 1)) { std::cout << " "; }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        tomos::metis::Nodal nodal(mesh);
        for (const auto& [key, vs] : nodal.adjacency()) {
            std::cout << key << " : ";
            for (const auto& v : vs) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
