#include "tomos/tomos-engine.hpp"

namespace tomos {
namespace engine {
namespace device {
    cl::Device
    choose(cl_device_type model) {
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
} // namespace device
    std::string
    kernel(const std::filesystem::path& path) {
        std::ifstream handle(path);
        if (not handle.is_open()) { throw std::runtime_error("could not load OpenCL kernel"); }

        std::stringstream ss;
        ss << handle.rdbuf();

        return ss.str();
    }
} // namespace engine
} // namespace tomos
