#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <tomos/tomos.hpp>

tomos::mesh::Mesh
decode(const std::filesystem::path& p) {
    std::ifstream handle(p);
    if (!handle.is_open()) { throw std::runtime_error("could not open mesh file"); }
    handle.unsetf(std::ios::skipws);

    return tomos::mesh::decode(boost::spirit::istream_iterator(handle), {});
}

int
main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::invalid_argument("invalid number of arguments"); }
        tomos::mesh::Mesh mesh  = decode(std::filesystem::path{argv[1]});

        tomos::Engine engine("./shaders/tomos.kernel", mesh);

        std::vector<float> xs               = engine.area();
        std::vector<tomos::mesh::Node> ns   = engine.normal();
        std::vector<tomos::mesh::Node> cs   = engine.centroid();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
