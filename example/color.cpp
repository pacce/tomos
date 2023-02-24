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

        tomos::Engine engine("./shaders/tomos.kernel", mesh);
        std::vector<float> vs = engine.color();
        for (auto v : vs) { std::cout << v << " "; }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
