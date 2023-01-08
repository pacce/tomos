#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        mesh::Mesh<float> mesh          = mesh::decode<float>(std::filesystem::path{argv[1]});
        tomos::metis::Adjacency dual    = tomos::metis::dual(mesh, tomos::metis::Common::EDGE);

        for (const auto& [key, vs] : dual) {
            std::cout << key << " : ";
            for (const auto& v : vs) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
        tomos::metis::Adjacency nodal = tomos::metis::nodal(mesh);
        for (const auto& [key, vs] : nodal) {
            std::cout << key << " : ";
            for (const auto& v : vs) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
        tomos::metis::Partitions ps = tomos::metis::partition(mesh, tomos::metis::Common::EDGE, 4);
        for (const auto& [key, p] : ps) {
            std::cout << key << " : " << p << std::endl;
        }
    } catch (const std::exception& e) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
