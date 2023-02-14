#ifndef TOMOS_ENGINE_HPP__
#define TOMOS_ENGINE_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <filesystem>
#include <tomos/tomos-mesh.hpp>
#include <mesh/mesh.hpp>

// #include "tomos-sparse.hpp"

namespace tomos {
namespace nodes {
    template <typename Precision>
    std::vector<cl_float3>
    encode(const ::mesh::Mesh<Precision>& mesh) {
        std::vector<cl_float3> xs(mesh.nodes.size());

        std::size_t index = 0;
        for (const auto& [_, node] : mesh.nodes) {
            xs[index++] = {node.x(), node.y(), node.z()};
        }
        return xs;
    }

    template <typename Precision>
    std::vector<cl_float3>
    encode(const std::vector<::mesh::Node<Precision>>& ns) {
        std::vector<cl_float3> xs(ns.size());

        std::size_t index = 0;
        for (const ::mesh::Node<Precision>& n : ns) { xs[index++] = {n.x(), n.y(), n.z()}; }
        return xs;
    }

    template <typename Precision>
    std::vector<::mesh::Node<Precision>>
    decode(const std::vector<cl_float3>& ns) {
        std::vector<::mesh::Node<Precision>> xs(ns.size());

        std::size_t index = 0;
        for (const cl_float3& n : ns) { xs[index++] = {n.s[0], n.s[1], n.s[2]}; }

        return xs;
    }
} // namespace nodes
    class Engine {
        struct __attribute__ ((packed)) Triangle {
            cl_uint3 nodes;
            cl_float resistivity;
            cl_uint  indices[9];
        };
        // using Coordinates = std::map<sparse::Coordinate, sparse::Index>;
        public:
            Engine(cl_device_type model)
                : device_(Engine::device(model))
                , context_(device_)
            {}

            std::vector<float>
            area(const tomos::mesh::Mesh& mesh, const std::filesystem::path& path) {
                cl::Program program = this->program(path);

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->indices(mesh);
                cl::Buffer values   = this->buffer<float>(mesh.elements.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "area");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.elements.size(), cl::NullRange);

                return this->read<float>(queue, values, mesh.elements.size());
            }

            std::vector<mesh::Node>
            centroid(const tomos::mesh::Mesh& mesh, const std::filesystem::path& path) {
                cl::Program program = this->program(path);

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->indices(mesh);
                cl::Buffer values   = this->buffer<cl_float3>(mesh.elements.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "centroid");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.elements.size(), cl::NullRange);

                return this->read<cl_float3>(queue, values, mesh.elements.size());
            }

            std::vector<tomos::mesh::Node>
            normal(const tomos::mesh::Mesh& mesh, const std::filesystem::path& path) {
                cl::Program program = this->program(path);

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->indices(mesh);
                cl::Buffer values   = this->buffer<cl_float3>(mesh.elements.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "normal");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.elements.size(), cl::NullRange);

                return this->read<cl_float3>(queue, values, mesh.elements.size());
            }

            // template <typename Precision>
            // std::vector<float>
            // color(const ::mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
            //     std::vector<float> values(sparse::nonzeros(mesh), 0.0f);

            //     using Color     = tomos::color::Color;
            //     using Index     = tomos::color::Index;
            //     using Indices   = std::vector<Index>;

            //     tomos::color::Colors cs = tomos::color::build(mesh, tomos::metis::Common::NODE);

            //     std::map<Color, Indices> colors;
            //     for (const auto& [element, color] : cs) {
            //         auto [it, inserted] = colors.insert({color, {element}});
            //         if (not inserted) { it->second.push_back(element); }
            //     }

            //     cl::Program program = this->program(path);

            //     cl::Buffer nodes    = this->nodes(mesh);
            //     cl::Buffer sparse   = this->buffer(values, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);

            //     cl::Kernel kernel(program, "stiffness");
            //     cl::CommandQueue queue(context_, device_);

            //     Coordinates coo  = sparse::coo(mesh);
            //     for (const auto& [color, es] : colors) {
            //         std::vector<Triangle> vs(es.size());
            //         for (std::size_t i = 0; i < es.size(); i++) {
            //             const ::mesh::Element& e    = mesh.element.find(es[i])->second;
            //             vs[i].nodes                 = {{
            //                       static_cast<uint32_t>(e.nodes[0] - 1)
            //                     , static_cast<uint32_t>(e.nodes[1] - 1)
            //                     , static_cast<uint32_t>(e.nodes[2] - 1)
            //                     }};
            //             vs[i].resistivity       = 1.0;
            //             std::vector<cl_uint> nz = Engine::nonzero(e, coo);

            //             std::copy(nz.begin(), nz.end(), vs[i].indices);
            //         }
            //         cl::Buffer elements = this->buffer(vs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            //         kernel.setArg(0, static_cast<ulong>(es.size()));
            //         kernel.setArg(1, nodes);
            //         kernel.setArg(2, elements);
            //         kernel.setArg(3, sparse);

            //         queue.enqueueNDRangeKernel(kernel, cl::NullRange, es.size(), cl::NullRange);
            //     }
            //     queue.enqueueReadBuffer(sparse, CL_TRUE, 0, values.size() * sizeof(float), values.data());

            //     return values;
            // }
        private:
            static cl::Device
            device(cl_device_type model) {
                std::vector<cl::Platform> platforms;
                cl::Platform::get(&platforms);

                if (platforms.empty()) {
                    throw std::runtime_error("could not find a valid OpenCL platform");
                }

                for (const cl::Platform& platform : platforms) {
                    std::vector<cl::Device> devices;
                    platform.getDevices(model, &devices);

                    for (const cl::Device& device : devices) {
                        if (not device.getInfo<CL_DEVICE_AVAILABLE>()) { continue; }
                        return device;
                    }
                }
                throw std::runtime_error("could not find a valid OpenCL device");
                return {};
            }

            static std::string
            kernel(const std::filesystem::path& path) {
                std::ifstream handle(path);
                if (not handle.is_open()) {
                    throw std::runtime_error("could not load OpenCL kernel");
                }

                std::stringstream ss;
                ss << handle.rdbuf();

                return ss.str();
            }

            cl::Program
            program(const std::string& source) {
                cl::Program p(context_, cl::Program::Sources({source}));
                p.build(device_);

                return p;
            }

            cl::Program
            program(const std::filesystem::path& path) {
                std::string source = Engine::kernel(path);
                return this->program(source);
            }

            template <typename T>
            cl::Buffer
            buffer(std::vector<T>& vs, cl_mem_flags flag) {
                return {context_, flag, vs.size() * sizeof(T), vs.data()};
            }

            template <typename T>
            cl::Buffer
            buffer(std::size_t size, cl_mem_flags flag) {
                return {context_, flag, size * sizeof(T)};
            }

            template <typename Precision>
            cl::Buffer
            nodes(const ::mesh::Mesh<Precision>& mesh) {
                std::vector<cl_float3> xs = nodes::encode(mesh);
                return buffer(xs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            }

            cl::Buffer
            nodes(const tomos::mesh::Mesh& mesh) {
                return {
                      context_
                    , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR
                    , mesh.nodes.size() * sizeof(cl_float3)
                    , const_cast<cl_float4*>(mesh.nodes.data())
                };
            }

            template <typename Precision>
            cl::Buffer
            indices(const ::mesh::Mesh<Precision>& mesh) {
                std::vector<cl_uint> xs;

                for (const auto& [_, element] : mesh.element) {
                    for (const ::mesh::node::Number& node : element.nodes) { xs.push_back(node - 1); }
                }
                return this->buffer(xs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            }

            cl::Buffer
            indices(const tomos::mesh::Mesh& mesh) {
                std::vector<cl_uint> xs;
                for (const tomos::mesh::Element& e : mesh.elements) {
                    xs.insert(xs.end(), e.nodes.begin(), e.nodes.end());
                }

                return this->buffer(xs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            }

            template <typename T>
            std::vector<T>
            read(const cl::CommandQueue& queue, const cl::Buffer& buffer, std::size_t count) {
                std::vector<T> xs(count);
                queue.enqueueReadBuffer(buffer, CL_TRUE, 0, count * sizeof(T), xs.data());
                return xs;
            }

            // static std::vector<cl_uint>
            // nonzero(const ::mesh::Element& element, const Coordinates& coo) {
            //     const std::vector<::mesh::node::Number>& nodes  = element.nodes;
            //     std::size_t count                               = nodes.size();

            //     std::vector<cl_uint> ii(count * count);
            //     for (std::size_t i = 0; i < count; i++) {
            //     for (std::size_t j = 0; j < count; j++) {
            //         std::size_t offset  = i * 3 + j;
            //         ii[offset]          = coo.find({nodes[i], nodes[j]})->second;
            //     }
            //     }

            //     return ii;
            // }

            cl::Device  device_;
            cl::Context context_;
    };
} // namespace tomos

#endif // TOMOS_ENGINE_HPP__
