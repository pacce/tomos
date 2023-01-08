#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        mesh::Mesh<float> mesh  = mesh::decode<float>(std::filesystem::path{argv[1]});
        std::cout << tomos::sparse::nonzeros(mesh) << std::endl;

        auto [cols, rows] = tomos::sparse::csr(mesh);
        for (std::size_t i = 0; i < rows.size() - 1; i++) {
            for (std::size_t j = rows[i]; j < rows[i + 1]; j++) {
                std::cout << cols[j] << " ";
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
