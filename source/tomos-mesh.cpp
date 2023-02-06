#include "tomos/tomos-mesh.hpp"

namespace tomos {
namespace mesh {
    Mesh
    decode(const std::filesystem::path& path) {
        std::ifstream handle(path);
        if (!handle.is_open()) { throw std::runtime_error("could not open mesh file"); }
        handle.unsetf(std::ios::skipws);

        return tomos::mesh::decode(boost::spirit::istream_iterator(handle), {});
    }
} // namespace mesh
} // namespace tomos
