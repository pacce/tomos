#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invali number of arguments"); }
        mesh::Mesh<float> mesh = mesh::decode<float>(std::filesystem::path{argv[1]});
        auto values = tomos::metis::dual(mesh, tomos::metis::Common::EDGE);

        for (const auto& [key, vs] : values) {
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
