#ifndef TOMOS_ENGINE_HPP__
#define TOMOS_ENGINE_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <filesystem>
#include <mesh/mesh.hpp>

#include "tomos-sparse.hpp"

namespace tomos {
    class Engine {
        struct __attribute__ ((packed)) Triangle {
            cl_uint3 nodes;
            cl_float resistivity;
            cl_uint  indices[9];
        };
        using Coordinates = std::map<sparse::Coordinate, sparse::Index>;
        public:
            Engine(cl_device_type model)
                : device_(Engine::device(model))
                , context_(device_)
            {}

            template <typename Precision>
            std::vector<float>
            area(const mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
                std::string source = Engine::kernel(path);

                cl::CommandQueue queue(context_, device_);
                cl::Program program(context_, cl::Program::Sources({source}));
                program.build(device_);

                std::vector<cl_uint> indices;

                for (const auto& [_, element] : mesh.element) {
                    for (const mesh::node::Number& node : element.nodes) { indices.push_back(node - 1); }
                }

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->buffer(indices, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
                cl::Buffer values   = this->buffer<float>(mesh.element.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "area");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.element.size(), cl::NullRange);

                std::vector<float> xs(mesh.element.size());
                queue.enqueueReadBuffer(values, CL_TRUE, 0, xs.size() * sizeof(float), xs.data());

                return xs;
            }

            template <typename Precision>
            std::vector<mesh::Node<Precision>>
            centroid(const mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
                std::string source = Engine::kernel(path);

                cl::CommandQueue queue(context_, device_);
                cl::Program program(context_, cl::Program::Sources({source}));
                program.build(device_);

                std::vector<cl_uint> indices;

                for (const auto& [_, element] : mesh.element) {
                    for (const mesh::node::Number& node : element.nodes) { indices.push_back(node - 1); }
                }

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->buffer(indices, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
                cl::Buffer values   = this->buffer<cl_float3>(mesh.element.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "centroid");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.element.size(), cl::NullRange);

                std::vector<cl_float3> cs(mesh.element.size());
                queue.enqueueReadBuffer(values, CL_TRUE, 0, cs.size() * sizeof(cl_float3), cs.data());

                std::vector<mesh::Node<Precision>> xs;
                xs.reserve(cs.size());
                for (const cl_float3& centroid : cs) {
                    xs.emplace_back(centroid.s[0], centroid.s[1], centroid.s[2]);
                }
                return xs;
            }

            template <typename Precision>
            std::vector<mesh::Node<Precision>>
            normal(const mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
                std::string source = Engine::kernel(path);

                cl::CommandQueue queue(context_, device_);
                cl::Program program(context_, cl::Program::Sources({source}));
                program.build(device_);

                std::vector<cl_uint> indices;

                for (const auto& [_, element] : mesh.element) {
                    for (const mesh::node::Number& node : element.nodes) { indices.push_back(node - 1); }
                }

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer elements = this->buffer(indices, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
                cl::Buffer values   = this->buffer<cl_float3>(mesh.element.size(), CL_MEM_READ_WRITE);

                cl::Kernel kernel(program, "normal");

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.element.size(), cl::NullRange);

                std::vector<cl_float3> cs(mesh.element.size());
                queue.enqueueReadBuffer(values, CL_TRUE, 0, cs.size() * sizeof(cl_float3), cs.data());

                std::vector<mesh::Node<Precision>> xs;
                xs.reserve(cs.size());
                for (const cl_float3& centroid : cs) {
                    xs.emplace_back(centroid.s[0], centroid.s[1], centroid.s[2]);
                }
                return xs;
            }

            template <typename Precision>
            std::vector<float>
            color(const mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
                std::vector<float> values(sparse::nonzeros(mesh), 0.0f);

                using Color     = tomos::color::Color;
                using Index     = tomos::color::Index;
                using Indices   = std::vector<Index>;

                tomos::color::Colors cs = tomos::color::build(mesh, tomos::metis::Common::NODE);

                std::map<Color, Indices> colors;
                for (const auto& [element, color] : cs) {
                    auto [it, inserted] = colors.insert({color, {element}});
                    if (not inserted) { it->second.push_back(element); }
                }

                std::string source      = Engine::kernel(path);

                cl::CommandQueue queue(context_, device_);
                cl::Program program(context_, cl::Program::Sources({source}));
                program.build(device_);

                cl::Kernel kernel(program, "stiffness");

                cl::Buffer nodes    = this->nodes(mesh);
                cl::Buffer sparse   = this->buffer(values, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);

                Coordinates coo  = sparse::coo(mesh);
                for (const auto& [color, es] : colors) {
                    std::vector<Triangle> vs(es.size());
                    for (std::size_t i = 0; i < es.size(); i++) {
                        const mesh::Element& e  = mesh.element.find(es[i])->second;
                        vs[i].nodes             = {{
                                  static_cast<uint32_t>(e.nodes[0] - 1)
                                , static_cast<uint32_t>(e.nodes[1] - 1)
                                , static_cast<uint32_t>(e.nodes[2] - 1)
                                }};
                        vs[i].resistivity       = 1.0;
                        std::vector<cl_uint> nz = Engine::nonzero(e, coo);

                        std::copy(nz.begin(), nz.end(), vs[i].indices);
                    }
                    cl::Buffer elements = this->buffer(vs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
                    kernel.setArg(0, static_cast<ulong>(es.size()));
                    kernel.setArg(1, nodes);
                    kernel.setArg(2, elements);
                    kernel.setArg(3, sparse);

                    queue.enqueueNDRangeKernel(kernel, cl::NullRange, es.size(), cl::NullRange);
                }
                queue.enqueueReadBuffer(sparse, CL_TRUE, 0, values.size() * sizeof(float), values.data());

                return values;
            }
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
            nodes(const mesh::Mesh<Precision>& mesh) {
                std::vector<cl_float3> xs(mesh.nodes.size());

                std::size_t index = 0;
                for (const auto& [_, node] : mesh.nodes) {
                    xs[index++] = {node.x(), node.y(), node.z()};
                }
                return buffer(xs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            }

            static std::vector<cl_uint>
            nonzero(const mesh::Element& element, const Coordinates& coo) {
                const std::vector<mesh::node::Number>& nodes    = element.nodes;
                std::size_t count                               = nodes.size();

                std::vector<cl_uint> ii(count * count);
                for (std::size_t i = 0; i < count; i++) {
                for (std::size_t j = 0; j < count; j++) {
                    std::size_t offset  = i * 3 + j;
                    ii[offset]          = coo.find({nodes[i], nodes[j]})->second;
                }
                }

                return ii;
            }

            cl::Device  device_;
            cl::Context context_;
    };
} // namespace tomos

#endif // TOMOS_ENGINE_HPP__
