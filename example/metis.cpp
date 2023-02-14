#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <tomos/tomos.hpp>
#include <tomos/tomos-mesh.hpp>

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
        for (const auto& [key, p] : ps) {
            std::cout << key << " : " << p << std::endl;
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
