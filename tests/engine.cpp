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

TEST(Stiffness, Oracle) {
    const mesh::Mesh<float> mesh = {
        {2.2, 0, 8}
        , mesh::node::Map<float>{
              {1, mesh::Node<float>(-0.99703213, -0.07698669, 0.0)}
            , {2, mesh::Node<float>(-0.91614308, -0.04714896, 0.0)}
            , {3, mesh::Node<float>(-1.00000000, -0.00000000, 0.0)}
        }
        , mesh::element::Map{
            {1, {mesh::element::Type::TRIANGLE3, {1}, {1, 2, 3}}}
        }
    };
    tomos::Engine engine(CL_DEVICE_TYPE_GPU);
    std::vector<float> expected = {
           0.73267143964767456
        , -0.30705833435058594
        , -0.42561310529708862
        ,  0.46990343928337097
        , -0.30705833435058594
        , -0.16284510493278503
        ,  0.58845824003219604
        , -0.42561310529708862
        , -0.16284510493278503
    };
    std::vector<float> actual   = engine.color(mesh, "./shaders/tomos.kernel");
    ASSERT_EQ(actual.size(), expected.size());

    auto kv = tomos::sparse::csr(mesh);
    for (std::size_t i = 0; i < actual.size(); i++) {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}

TEST(Stiffness, Square) {
    const mesh::Mesh<float> mesh = {
        {2.2, 0, 8}
        , mesh::node::Map<float>{
              {1, mesh::Node<float>(0.0, 0.0, 0.0)}
            , {2, mesh::Node<float>(1.0, 0.0, 0.0)}
            , {3, mesh::Node<float>(1.0, 1.0, 0.0)}
            , {4, mesh::Node<float>(0.0, 1.0, 0.0)}
        }
        , mesh::element::Map{
              {1, {mesh::element::Type::TRIANGLE3, {1}, {1, 2, 3}}}
            , {2, {mesh::element::Type::TRIANGLE3, {1}, {1, 3, 4}}}
        }
    };
    tomos::Engine engine(CL_DEVICE_TYPE_GPU);
    std::vector<float> expected = {
           1.0  // (1, 1) -  0
        , -0.5  // (1, 2) -  1
        ,  0.0  // (1, 3) -  2
        , -0.5  // (1, 4) -  3
        ,  1.0  // (2, 2) -  4
        , -0.5  // (2, 1) -  5
        , -0.5  // (2, 3) -  6
        ,  1.0  // (3, 3) -  7
        ,  0.0  // (3, 1) -  8
        , -0.5  // (3, 2) -  9
        , -0.5  // (3, 4) - 10
        ,  1.0  // (4, 4) - 11
        , -0.5  // (4, 1) - 12
        , -0.5  // (4, 3) - 13
    };
    std::vector<float> actual   = engine.color(mesh, "./shaders/tomos.kernel");
    ASSERT_EQ(actual.size(), expected.size());

    auto kv = tomos::sparse::csr(mesh);
    for (std::size_t i = 0; i < actual.size(); i++) {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}


int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
