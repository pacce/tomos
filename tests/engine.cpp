#include <boost/spirit/include/qi.hpp>
#include <gtest/gtest.h>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

TEST(GPU, Area) {
    const mesh::Mesh<float> mesh = {
        {2.2, 0, 8}
        , mesh::node::Map<float>{
              {1, mesh::Node<float>(0.0f, 0.0f, 0.0f)}
            , {2, mesh::Node<float>(1.0f, 0.0f, 0.0f)}
            , {3, mesh::Node<float>(1.0f, 1.0f, 0.0f)}
            , {4, mesh::Node<float>(2.0f, 0.0f, 0.0f)}
            , {5, mesh::Node<float>(2.0f, 2.0f, 0.0f)}
            , {6, mesh::Node<float>(3.0f, 0.0f, 0.0f)}
            , {7, mesh::Node<float>(3.0f, 3.0f, 0.0f)}
        }
        , mesh::element::Map{
              {1, {mesh::element::Type::TRIANGLE3, {1}, {1, 2, 3}}}
            , {2, {mesh::element::Type::TRIANGLE3, {1}, {1, 4, 5}}}
            , {3, {mesh::element::Type::TRIANGLE3, {1}, {1, 6, 7}}}
        }
    };
    tomos::Engine engine(CL_DEVICE_TYPE_GPU);
    std::vector<float> expected = {0.5, 2.0, 4.5};
    std::vector<float> actual   = engine.area(mesh, "./shaders/tomos.kernel");

    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) { EXPECT_EQ(actual[i], expected[i]); }
}

int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
