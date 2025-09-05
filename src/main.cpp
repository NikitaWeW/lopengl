/*
        +____________+
        /:\         ,:\
       / : \       , : \
      /  :  \     ,  :  \
     /   :   +-----------+
    +....:../:...+   :  /|
    |\   +./.:...`...+ / |
    | \ ,`/  :   :` ,`/  |
    |  \ /`. :   : ` /`  |
    | , +-----------+  ` |
    |,  |   `+...:,.|...`+
    +...|...,'...+  |   /
     \  |  ,     `  |  /
      \ | ,       ` | /
       \|,         `|/
        +___________+

2-Dimensional ASCII Representation Of A 3-Dimensional Cross-Section Of A 4-Dimensional Cube

---

Copyright (c) 2025 Nikita Martynau (https://opensource.org/license/mit)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "main.hpp"

int main(int argc, char **argv)
{
    GLFWwindow *window = nullptr;
    if(!init(&window)) {
        std::cout << "failed to init!\n";
        return -1;
    }
    assert(window);

    // ===================================

    Data data{};

    data.skybox = ogl::Cubemap{"res/textures/milky_way.jpg"};
    data.texture = data.skybox;
    model::Loader loader;
    data.cube = loader.load("res/models/cube.obj");
    
    data.window = window;
    data.distance.falloff = 10;
    data.inputs.sensitivity = 0.5;

    // ===================================
    
    glfwSetWindowUserPointer(data.window, &data);
    glfwSetScrollCallback(data.window, scroll_callback);
    glfwSetKeyCallback(data.window, key_callback);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        float start = (float) glfwGetTime();

        drawFrame(data);

        glfwPollEvents();
        glfwSwapBuffers(window);
        data.deltatime = (float) glfwGetTime() - start;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}
