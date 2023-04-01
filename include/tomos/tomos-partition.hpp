#ifndef TOMOS_PARTITION_HPP__
#define TOMOS_PARTITION_HPP__

#include <cstdint>
#include <map>
#include <tomos/tomos-mesh.hpp>
#include <vector>

namespace tomos {
namespace partition {
    using Index         = std::size_t;
    using Indices       = std::vector<Index>;

    using Partition     = std::size_t;
    using Partitions    = std::map<Partition, Indices>;

    bool
    valid(const tomos::mesh::Mesh& mesh, const Partitions& ps, std::size_t limit);
} // namespace partition
} // namespace tomos

#endif // TOMOS_PARTITION_HPP__
