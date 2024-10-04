#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm.hpp>
#include <logger.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <iostream> //standart includes
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <optional>

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

#define checkGLError \
errorgl = glGetError();                                                             \
if(errorgl != GL_NO_ERROR) LOG_ERROR("opengl error: %s", getGLErrorString(errorgl));\

GLenum errorgl;
GLFWwindow* window;
size_t iteration = 0;

double frameBeginTimeSeconds;
double renderTimeSeconds = 0;
double deltaTime = 0;
double FPS = 0;

int windowWidth;
int windowHeight;
unsigned vertexShader;
unsigned fragmentShader;
unsigned shaderProgram;

float vertices[] = {
    -0.5f, -0.5f, 0.0f, // 0
     0.5f, -0.5f, 0.0f, // 1
     0.5f,  0.5f, 0.0f, // 2
    -0.5f,  0.5f, 0.0f, // 3
};
unsigned indicies[] = {
    0, 1, 2,
    0, 2, 3
};

struct Shader {
    std::string Vertex;
    std::string Fragment;
};

void cleanupAndTerminate(int code = 0) {
    LOG_INFO("cleaning up.");
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(code);
}
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}
double getTimeSeconds() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count(); 
}
bool compileShader(unsigned &shader, const char *shaderSource, int mode, size_t log_size = 0, char *log = nullptr) {
    shader = glCreateShader(mode);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        if(log) glGetShaderInfoLog(shader, log_size, nullptr, log);
        return false;
    }
    return true;
}
const char* getGLErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_STACK_OVERFLOW:
            return "Stack overflow";
        case GL_STACK_UNDERFLOW:
            return "Stack underflow";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Invalid framebuffer operation";
        default:
            return "Unknown error";
    }
}
Shader parseShader(std::string filepath) {
    std::ifstream fileStream(filepath, std::ios::in);
    std::stringstream shaderStreams[2];
    unsigned currentIndex;
    std::string line;
    while(getline(fileStream, line)) {
        if(line.find("#shader") != std::string::npos) {
            if(line.find("vertex") != std::string::npos) {
                currentIndex = 0;
            } else if(line.find("fragment") != std::string::npos) {
                currentIndex = 1;
            } 
        } else {
            shaderStreams[currentIndex] << line << '\n';
        }
    }
    return {shaderStreams[0].str(), shaderStreams[1].str()};
}

int main()
{
    //setup
    std::filesystem::create_directory("logs");
    std::fstream createLogFile("logs/main.log");
    
    logger_initConsoleLogger(stdout);
    logger_initFileLogger("logs/main.log", 1024 * 1024, 5);
    logger_setLevel(debug ? LogLevel_DEBUG : LogLevel_INFO);

    if (glfwInit()) LOG_INFO("glfw initialised successfully");
    else {
        LOG_FATAL("failed to init glfw!");
        cleanupAndTerminate(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(640, 480, "-- FPS", NULL, NULL);
    if (window) LOG_INFO("window initialised successfully at adress %i", window);
    else {
        LOG_FATAL("failed to initialise window.");
        cleanupAndTerminate(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (gladLoadGL((GLADloadfunc) glfwGetProcAddress)) 
        LOG_INFO("gl loaded successfully. gl version: %s", glGetString(GL_VERSION));
    else{
        LOG_FATAL("gladLoadGL: Failed to initialize GLAD!");
        cleanupAndTerminate(-1);
    }

    ImGui::CreateContext();

    IMGUI_CHECKVERSION();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    
    // compile shaders
    char info_log[512];
    auto shader = parseShader("src/shaders/basic.glsl");
    if(compileShader(vertexShader, shader.Vertex.c_str(), GL_VERTEX_SHADER, 512, info_log)) {
        LOG_INFO("compiled vertex shader successfully.");
    } else {
        LOG_FATAL("failed to compile vertex shader. log:\n%s", info_log);
        cleanupAndTerminate(-2);
    }
    if(compileShader(fragmentShader, shader.Fragment.c_str(), GL_FRAGMENT_SHADER, 512, info_log)) {
        LOG_INFO("compiled fragment shader successfully.");
    } else {
        LOG_FATAL("failed to compile fragment shader. log:\n%s", info_log);
        cleanupAndTerminate(-2);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, info_log);
        LOG_FATAL("failed to link %s. log:\n%s", "shaderProgram", info_log);
        cleanupAndTerminate(-2);
    } else {
        LOG_INFO("linked successfully");
    }

    glUseProgram(shaderProgram);

    unsigned VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glClearColor(0, 0.1, 0.1, 0);
    double displayRenderTimeSeconds = 0;
    double displayDeltaTime = 0;
    int refreshRate = 500;
    while (!glfwWindowShouldClose(window))
    {
        frameBeginTimeSeconds = getTimeSeconds();

        //render here
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        renderTimeSeconds = getTimeSeconds() - frameBeginTimeSeconds;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //dear imgui here
        ImGui::Begin("debug");
        ImGui::Text("FPS: %f", FPS);
        ImGui::Text("delta time: %fms", displayDeltaTime * 1000);
        ImGui::Text("render time: %fms", displayRenderTimeSeconds * 1000);
        ImGui::InputInt("refresh rate", &refreshRate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        checkGLError
        deltaTime = getTimeSeconds() - frameBeginTimeSeconds;
        if(iteration++ % (refreshRate > 0 ? refreshRate : 1) == 0) {
            FPS = deltaTime > 0 ? 1 / deltaTime : 0;
            displayRenderTimeSeconds = renderTimeSeconds;
            displayDeltaTime = deltaTime;
            glfwSetWindowTitle(window, (std::to_string((int) FPS) + " FPS").c_str());
        }
    }

    cleanupAndTerminate();
}
