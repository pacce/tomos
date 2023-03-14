#include <gmsh.h>

#include <iostream>
#include <numeric>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>
#include <tomos/tomos-mesh.hpp>

const double LC                 = 1e-1;
const double RADIUS             = 1e-1;
const std::size_t ELECTRODES    = 16;

int
main(int, char**) {
    gmsh::initialize();
    gmsh::model::add("disk");

    for (std::size_t electrode = 0; electrode < ELECTRODES; electrode++) {
        double step     = static_cast<double>(electrode) / static_cast<double>(ELECTRODES);
        double angle    = 2.0 * std::numbers::pi * step;

        double x = RADIUS * std::cos(angle);
        double y = RADIUS * std::sin(angle);

        gmsh::model::geo::addPoint(x, y, 0.0, LC, electrode);
    }
    std::vector<int> loop = {};
    for (std::size_t electrode = 0; electrode < ELECTRODES; electrode++) {
        loop.push_back(
                gmsh::model::geo::addLine(electrode, (electrode + 1) % ELECTRODES, electrode)
                );
    }
    gmsh::model::geo::addCurveLoop(loop, 1);
    gmsh::model::geo::addPlaneSurface({1}, 1);
    gmsh::model::geo::synchronize();

    gmsh::model::mesh::generate(2);
    gmsh::option::setNumber("Mesh.MshFileVersion", 2.2);
    gmsh::write("t1.msh");
    gmsh::finalize();

    mesh::Mesh<float> mesh = mesh::decode<float>("t1.msh");
    tomos::mesh::Mesh forward;
    for (const auto& [_, node] : mesh.nodes) {
        forward.nodes.push_back({{node.x(), node.y(), node.z()}});
    }
    for (const auto& [_, element] : mesh.element) {
        switch(element.type) {
            case mesh::element::Type::TRIANGLE3:
            {
                tomos::mesh::node::Numbers ns;
                std::transform(
                          element.nodes.begin()
                        , element.nodes.end()
                        , std::back_inserter(ns)
                        , [](const mesh::node::Number& n) { return static_cast<cl_uint>(n - 1); }
                        );
                forward.elements.push_back({tomos::mesh::element::Type::TRIANGLE3, ns});
                break;
            };
            default: continue;
        }
    }
    tomos::Engine engine("./shaders/tomos.kernel", forward);
    std::vector<float> vs = engine.color();

    return 0;
}
