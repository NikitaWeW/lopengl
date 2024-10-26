#include <array>
#include "Test.hpp"

namespace test
{
    class TestClearcolor : public Test {
    private:
        std::array<float, 4> color;
    public:
        TestClearcolor();
        ~TestClearcolor();
        void onRender(GLFWwindow *window, double deltatime) override;
        void onImGuiRender(double deltatime) override;
    };
}