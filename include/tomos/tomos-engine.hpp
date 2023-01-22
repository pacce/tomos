#ifndef TOMOS_ENGINE_HPP__
#define TOMOS_ENGINE_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <filesystem>
#include <mesh/mesh.hpp>

namespace tomos {
    class Engine {
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

            cl::Device  device_;
            cl::Context context_;
    };
} // namespace tomos

#endif // TOMOS_ENGINE_HPP__
