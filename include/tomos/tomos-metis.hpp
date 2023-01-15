#ifndef TOMOS_METIS_HPP__
#define TOMOS_METIS_HPP__

#include <iostream>
#include <mesh/mesh.hpp>
#include <metis.h>

namespace tomos {
namespace metis {
    using Index         = mesh::element::Number;
    using Neighbours    = std::vector<Index>;
    using Adjacency     = std::map<Index, Neighbours>;

    using Partition     = std::size_t;
    using Partitions    = std::map<Index, Partition>;

    enum class Common : uint8_t { NODE = 1, EDGE = 2, FACE = 3 };

    template <typename Precision>
    class Dual {
        public:
            Dual(const mesh::Mesh<Precision>& mesh, Common common)
                : ne_(static_cast<idx_t>(mesh.element.size()))
                , nn_(static_cast<idx_t>(mesh.nodes.size()))
            {
                std::vector<idx_t> eptr = {0};
                std::vector<idx_t> eind = {};

                for (const auto& [key, e] : mesh.element) {
                    keys_.push_back(key);

                    eptr.push_back(e.nodes.size() + eptr.back());
                    for (const mesh::node::Number& n : e.nodes) {
                        eind.push_back(n - 1); // C-style 0-based indexing
                    }
                }

                idx_t numflag = 0;  // C-style 0-based indexing
                idx_t ncommon = static_cast<idx_t>(common);

                METIS_MeshToDual(&ne_, &nn_, eptr.data(), eind.data(), &ncommon, &numflag, &xadj_, &adjncy_);
            }

            ~Dual() {
                METIS_Free(xadj_);
                METIS_Free(adjncy_);
            }

            Adjacency
            adjacency() const {
                Adjacency values;

                idx_t * prev = xadj_;
                idx_t * next = xadj_ + 1;
                for (const Index& key : keys_) {
                    values[key] = {};
                    for (idx_t j = *prev; j < *next; j++) { values[key].push_back(adjncy_[j] + 1); }

                    prev++; next++;
                }
                return values;
            }

            Partitions
            partition(std::size_t count) {
                idx_t ncon      = 1;
                idx_t nparts    = static_cast<idx_t>(count);
                idx_t edgecut   = 0;

                std::vector<idx_t> part(ne_);
                idx_t options[METIS_NOPTIONS];
                METIS_SetDefaultOptions(options);

                METIS_PartGraphKway(
                          &ne_          // number of vertices
                        , &ncon         // number of balancing constraints
                        , xadj_
                        , adjncy_
                        , NULL          // vwgt
                        , NULL          // vsize
                        , NULL          // adjwgt
                        , &nparts       // number of partitions
                        , NULL          // tpwgts
                        , NULL          // ubvec
                        , options       // options
                        , &edgecut      // objval
                        , part.data()
                        );

                Partitions values;
                for (std::size_t i = 0; i < keys_.size(); i++) {
                    const Index& key        = keys_[i];
                    const idx_t& partition  = part[i];

                    values[key] = static_cast<std::size_t>(partition);
                }
                return values;
            }
        private:
            idx_t ne_;  // number of elements
            idx_t nn_;  // number of nodes

            idx_t * xadj_;      // pointer structure
            idx_t * adjncy_;    // pointer structure

            std::vector<Index> keys_;
    };

    template <typename Precision>
    class Nodal {
        public:
            Nodal(const mesh::Mesh<Precision>& mesh)
                : ne_(static_cast<idx_t>(mesh.element.size()))
                , nn_(static_cast<idx_t>(mesh.nodes.size()))
            {
                std::vector<idx_t> eptr = {0};
                std::vector<idx_t> eind = {};

                for (const auto& [key, _] : mesh.nodes) { keys_.push_back(key); }
                for (const auto& [key, e] : mesh.element) {
                    eptr.push_back(e.nodes.size() + eptr.back());
                    for (const mesh::node::Number& n : e.nodes) { eind.push_back(n - 1); }  // C-style 0-based indexing
                }

                idx_t numflag = 0;  // C-style 0-based indexing
                METIS_MeshToNodal(&ne_, &nn_, eptr.data(), eind.data(), &numflag, &xadj_, &adjncy_);
            }

            Adjacency
            adjacency() const {
                Adjacency values;

                idx_t * prev = xadj_;
                idx_t * next = xadj_ + 1;
                for (const Index& key : keys_) {
                    values[key] = {};
                    for (idx_t j = *prev; j < *next; j++) { values[key].push_back(adjncy_[j] + 1); }

                    prev++; next++;
                }
                return values;
            }

            ~Nodal() {
                METIS_Free(xadj_);
                METIS_Free(adjncy_);
            }
        private:
            idx_t ne_;  // number of elements
            idx_t nn_;  // number of nodes

            idx_t * xadj_;      // pointer structure
            idx_t * adjncy_;    // pointer structure

            std::vector<Index> keys_;
    };
} // namespace metis
} // namespace tomos

#endif // TOMOS_METIS_HPP__
