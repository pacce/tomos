#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        tomos::Engine engine(CL_DEVICE_TYPE_GPU);

        mesh::Mesh<float> mesh              = mesh::decode<float>(std::filesystem::path{argv[1]});
        std::vector<mesh::Node<float>> ns   = engine.normal(mesh, "./shaders/tomos.kernel");
        std::cout << ns.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
