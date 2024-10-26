#include <array>
#include "Test.hpp"

namespace test
{
    class TestClearColor : public Test {
    private:
        std::array<float, 4> color;
    public:
        TestClearColor();
        ~TestClearColor();
        void onRender(GLFWwindow *window, double deltatime) override;
        void onImGuiRender(double deltatime) override;
    };
}