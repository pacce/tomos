#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <tomos/tomos.hpp>

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        tomos::Engine engine(CL_DEVICE_TYPE_GPU);
        tomos::mesh::Mesh mesh  = tomos::mesh::decode(std::filesystem::path{argv[1]});

        engine.area(mesh, "./shaders/tomos.kernel");
        engine.normal(mesh, "./shaders/tomos.kernel");
        engine.centroid(mesh, "./shaders/tomos.kernel");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
