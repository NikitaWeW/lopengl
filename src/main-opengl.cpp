#include <glad/glad.h> //important includes
#include <GLFW/glfw3.h>

#include <logger.h>

#include <iostream> //standart includes
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

int main()
{
    {
        try { //manage files: rename "lastest" to "previous", create lastest.
            std::filesystem::remove("logs/previous.log");
            std::filesystem::rename("logs/lastest.log", "logs/previous.log");
        } catch(std::filesystem::filesystem_error) {} //its fine
        std::filesystem::create_directory("logs");
        std::fstream createLogFile("logs/lastest.log");
        
        
        logger_initConsoleLogger(stdout); //code from readme example
        logger_initFileLogger("logs/lastest.log", 1024 * 1024, 5);
        logger_setLevel(debug ? LogLevel_DEBUG : LogLevel_INFO);
        LOG_INFO("logging started");
    }

    if (glfwInit()) LOG_DEBUG("glfw initialised successful");
    else {
        LOG_FATAL("failed to init glfw!");
        return -1;
    }

    GLFWwindow* window;
    window = glfwCreateWindow(640, 480, "-- FPS", NULL, NULL);
    if (window) LOG_DEBUG("window initialised sucsessful at adress %i", window);
    else {
        LOG_FATAL("failed to initialise window.");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    int res = gladLoadGL();
    if(res) LOG_DEBUG("gl loaded successful. gl version: %s", glGetString(GL_VERSION));
    else {
        LOG_FATAL("failed to load gl: gladLoadGLLoader returned code is %i but not 0!", res);
        glfwTerminate();
        return -1;
    }
    glClearColor(0, 0.1, 0.1, 0);
    double s = 0.5;
    size_t iteration = 0;
    while (!glfwWindowShouldClose(window))
    {
        double begin = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glColor3d(1, 0, 0); glVertex2d(s, -s);
        glColor3d(0, 1, 0); glVertex2d(-s, -s);
        glColor3d(0, 0, 1); glVertex2d(0, s);
        glEnd();

        glfwSwapBuffers(window);

        glfwPollEvents();
        ++iteration;
        double end = glfwGetTime();
        double FPS = 1 / (end - begin);
        if(iteration % 2500 == 0) glfwSetWindowTitle(window, (std::to_string((int) FPS) + " FPS").c_str());
    }

    glfwTerminate();
    return 0;
}
