#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <logger.h>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <fstream>

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

int main() {
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
    // Initialize GLFW
    if (!glfwInit()) {
        LOG_FATAL("Failed to initialize GLFW");
        return -1;
    }

    // Make sure Vulkan is supported by GLFW
    if (!glfwVulkanSupported()) {
        LOG_FATAL("Vulkan not supported by GLFW");
        return -1;
    }

    // Vulkan instance creation
    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        LOG_FATAL("Failed to create Vulkan instance");
        return -1;
    }

    // List available extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    LOG_INFO("Available Vulkan extensions:");
    for (const auto& extension : extensions) {
        LOG_DEBUG("\t%s", extension.extensionName);
    }

    // Clean up
    vkDestroyInstance(instance, nullptr);
    glfwTerminate();

    LOG_INFO("Vulkan setup was successful!");
    return 0;
}
