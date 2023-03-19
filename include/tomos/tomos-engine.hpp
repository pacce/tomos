#ifndef TOMOS_ENGINE_HPP__
#define TOMOS_ENGINE_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <filesystem>
#include <fstream>
#include <tomos/tomos-mesh.hpp>

#include "tomos-color.hpp"
#include "tomos-sparse.hpp"

namespace tomos {
    struct Memory {
        std::size_t local;
        std::size_t global;

        friend std::ostream&
        operator<<(std::ostream& os, const Memory& memory) {
            const std::size_t KiB = 1024;
            const std::size_t MiB = KiB * KiB;
            const std::size_t GiB = KiB * MiB;

            std::cout   << (memory.local / KiB)
                        << " KiB"
                        << ", "
                        << (memory.global / GiB)
                        << " GiB"
                        ;
            return os;
        }
    };

    class Engine {
        struct __attribute__ ((packed)) Triangle {
            cl_uint3 nodes;
            cl_float resistivity;
            cl_uint  indices[9];
        };
        using Coordinates = std::map<sparse::Coordinate, sparse::Index>;
        public:
            Engine(const std::filesystem::path& path, const tomos::mesh::Mesh& mesh)
                : device_(Engine::device(CL_DEVICE_TYPE_GPU))
                , context_(device_)
                , memory_(Engine::memory(device_))
                , mesh_(mesh)
            {
                nodes_      = this->nodes(mesh);
                elements_   = this->indices(mesh);

                program_ = cl::Program(context_, Engine::kernel(path));
                program_.build();

                area_       = cl::Kernel(program_, "area");
                centroid_   = cl::Kernel(program_, "centroid");
                normal_     = cl::Kernel(program_, "normal");
                stiffness_  = cl::Kernel(program_, "stiffness");
            }

            std::vector<float>
            area() {
                std::size_t elements = mesh_.elements.size();
                cl::Buffer values = this->buffer<float>(elements, CL_MEM_READ_WRITE);

                area_.setArg(0, nodes_);
                area_.setArg(1, elements_);
                area_.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(area_, cl::NullRange, elements, cl::NullRange);

                return this->read<float>(queue, values, elements);
            }

            std::vector<tomos::mesh::Node>
            centroid() {
                std::size_t elements    = mesh_.elements.size();
                cl::Buffer values       = this->buffer<cl_float3>(elements, CL_MEM_READ_WRITE);

                centroid_.setArg(0, nodes_);
                centroid_.setArg(1, elements_);
                centroid_.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(centroid_, cl::NullRange, elements, cl::NullRange);
                return this->read<cl_float3>(queue, values, elements);
            }

             std::vector<tomos::mesh::Node>
             normal() {
                std::size_t elements    = mesh_.elements.size();
                cl::Buffer values       = this->buffer<cl_float3>(elements, CL_MEM_READ_WRITE);

                normal_.setArg(0, nodes_);
                normal_.setArg(1, elements_);
                normal_.setArg(2, values);

                cl::CommandQueue queue(context_, device_);
                queue.enqueueNDRangeKernel(normal_, cl::NullRange, elements, cl::NullRange);
                return this->read<cl_float3>(queue, values, elements);
             }

            std::vector<float>
            color() {
                std::vector<float> values(sparse::nonzeros(mesh_), 0.0f);

                using Color         = tomos::color::Color;
                using Index         = tomos::color::Index;
                using Indices       = std::vector<Index>;

                tomos::color::Colors cs = tomos::color::build(mesh_, tomos::metis::Common::NODE);

                std::map<Color, Indices> colors;
                for (const auto& [element, color] : cs) {
                    auto [it, inserted] = colors.insert({color, {element}});
                    if (not inserted) { it->second.push_back(element); }
                }

                cl::Buffer sparse   = this->buffer(values, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);
                cl::CommandQueue queue(context_, device_);

                Coordinates coo  = sparse::coo(mesh_);
                for (const auto& [color, es] : colors) {
                    std::vector<Triangle> vs(es.size());
                    for (std::size_t i = 0; i < es.size(); i++) {
                        const tomos::mesh::Element& e = mesh_.elements[es[i]];
                        vs[i].nodes         = {{e.nodes[0], e.nodes[1], e.nodes[2]}};
                        vs[i].resistivity   = 1.0;
                        std::vector<cl_uint> nz = Engine::nonzero(e, coo);
                        std::copy(nz.begin(), nz.end(), vs[i].indices);
                    }

                    cl::Buffer elements = this->buffer(vs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
                    stiffness_.setArg(0, static_cast<ulong>(es.size()));
                    stiffness_.setArg(1, nodes_);
                    stiffness_.setArg(2, elements);
                    stiffness_.setArg(3, sparse);

                    queue.enqueueNDRangeKernel(stiffness_, cl::NullRange, es.size(), cl::NullRange);
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

            static Memory
            memory(const cl::Device& device) {
                return {
                      device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>()
                    , device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()
                };
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

            cl::Buffer
            nodes(const tomos::mesh::Mesh& mesh) {
                return {
                      context_
                    , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR
                    , mesh.nodes.size() * sizeof(cl_float3)
                    , const_cast<cl_float4*>(mesh.nodes.data())
                };
            }

            cl::Buffer
            indices(const tomos::mesh::Mesh& mesh) {
                std::vector<cl_uint> xs;
                for (const tomos::mesh::Element& e : mesh.elements) {
                    xs.insert(xs.end(), e.nodes.begin(), e.nodes.end());
                }

                return this->buffer<cl_uint>(xs, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
            }

            template <typename T>
            std::vector<T>
            read(const cl::CommandQueue& queue, const cl::Buffer& buffer, std::size_t count) {
                std::vector<T> xs(count);
                queue.enqueueReadBuffer(buffer, CL_TRUE, 0, count * sizeof(T), xs.data());
                return xs;
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
            Memory      memory_;

            tomos::mesh::Mesh   mesh_;
            cl::Buffer          nodes_;
            cl::Buffer          elements_;

            cl::Program program_;
            cl::Kernel  area_;
            cl::Kernel  centroid_;
            cl::Kernel  normal_;
            cl::Kernel  stiffness_;
    };
} // namespace tomos

#endif // TOMOS_ENGINE_HPP__
