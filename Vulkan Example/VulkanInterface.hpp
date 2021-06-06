#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "QueueFamilyIndices.hpp"
#include "VulkanInterface.hpp"
#include <vector>
namespace vulkanExample
{
	class VulkanInterface
	{
	public:
		std::vector<const char*> getRequiredExtensions();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		VulkanInterface(const uint32_t width, const uint32_t height);
		~VulkanInterface();
		void run();


	private:
		std::vector<const char*> validationLayers;

		GLFWwindow* window;
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		QueueFamilyIndices queueFamilies;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkDevice logicalDevice = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger;

		uint32_t w_width;
		uint32_t w_height;

		void initWindow(const uint32_t width, const uint32_t height);
		void initVulkan();
		void createSurface();
		bool checkValidationLayerSupport();
		void mainLoop();
		void cleanup();
		void createInstance();
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void pickPhysicalDevices();
		std::optional<QueueFamilyIndices> checkDevice(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void createLogicalDevice();
	};

} //namespace


