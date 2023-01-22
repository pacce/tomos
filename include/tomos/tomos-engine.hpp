#ifndef TOMOS_ENGINE_HPP__
#define TOMOS_ENGINE_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <filesystem>
#include <mesh/mesh.hpp>

namespace tomos {
namespace engine {
namespace host {
    template <typename Precision>
    std::vector<cl_float3>
    nodes(const mesh::Mesh<Precision>& mesh) {
        std::vector<cl_float3> xs(mesh.nodes.size());

        std::size_t index = 0;
        for (const auto& [_, node] : mesh.nodes) {
            xs[index++] = {node.x(), node.y(), node.z()};
        }
        return xs;
    }
} // namespace host
namespace device {
    cl::Device
    choose(cl_device_type model);

    template <typename Precision>
    cl::Buffer
    nodes(cl::Context& context, const mesh::Mesh<Precision>& mesh) {
        std::vector<cl_float3> xs = host::nodes(mesh);
        return {context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, xs.size() * sizeof(cl_float3), xs.data()};
    }
} // namespace device
    std::string
    kernel(const std::filesystem::path& path);
} // namespace engine
    class Engine {
        public:
            Engine(cl_device_type model)
                : device_(engine::device::choose(model))
                , context_(device_)
            {}

            template <typename Precision>
            std::vector<float>
            area(const mesh::Mesh<Precision>& mesh, const std::filesystem::path& path) {
                std::string source = engine::kernel(path);

                cl::CommandQueue queue(context_, device_);
                cl::Program program(context_, cl::Program::Sources({source}));
                program.build(device_);

                cl::Kernel kernel(program, "area");
                cl::Buffer nodes = engine::device::nodes(context_, mesh);

                std::vector<cl_uint> indices;

                for (const auto& [_, element] : mesh.element) {
                    for (const mesh::node::Number& node : element.nodes) { indices.push_back(node - 1); }
                }

                cl::Buffer elements(
                          context_
                        , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR
                        , indices.size() * sizeof(cl_uint)
                        , indices.data()
                        );
                std::vector<float> xs(mesh.element.size());
                cl::Buffer values(context_, CL_MEM_READ_WRITE, mesh.element.size() * sizeof(float));

                kernel.setArg(0, nodes);
                kernel.setArg(1, elements);
                kernel.setArg(2, values);

                queue.enqueueNDRangeKernel(kernel, cl::NullRange, mesh.element.size(), cl::NullRange);
                queue.enqueueReadBuffer(values, CL_TRUE, 0, xs.size() * sizeof(float), xs.data());

                return xs;
            }
        private:
            cl::Device  device_;
            cl::Context context_;
    };
} // namespace tomos

#endif // TOMOS_ENGINE_HPP__
