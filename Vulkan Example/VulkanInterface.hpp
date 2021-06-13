#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"
#include <vector>
#include <string>
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

		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t currentFrame = 0;
		bool frameBufferResized = false;
		

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		std::vector<const char*> validationLayers;
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;

		GLFWwindow* window;
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkDevice logicalDevice = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		VkCommandPool commandPool;

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		QueueFamilyIndices queueFamilies;

		uint32_t w_width;
		uint32_t w_height;


		void initWindow(const uint32_t width, const uint32_t height);
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initVulkan();
		void createSurface();
		bool checkValidationLayerSupport();
		void mainLoop();
		void cleanup();
		void createInstance();
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void pickPhysicalDevices();
		void createLogicalDevice();
		void createSwapChain();
		void recreateSwapChain();
		void cleanupSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipeline();
		void createFrameBuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();
		void drawFrame();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		void printDeviceExtensionSupport(VkPhysicalDevice device);
		VkShaderModule createShaderModule(const std::vector<char>& code);
		std::optional<QueueFamilyIndices> checkDevice(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		static std::vector<char> readFile(const std::string& filename);
	};

} //namespace


