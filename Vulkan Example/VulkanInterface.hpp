#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"
#include "Vertex.hpp"
#include <vector>
#include <string>
#include <map>
namespace vulkanExample
{

	enum class KeyboardKeys : uint16_t
	{
		KEY_NONE = 1 << 0,
		KEY_W = 1 << 1,
		KEY_S = 1 << 2,
		KEY_A = 1 << 3,
		KEY_D = 1 << 4,
		KEY_UP = 1 << 5,
		KEY_DOWN = 1 << 6
	};

	inline void operator+=(KeyboardKeys &a, KeyboardKeys b)
	{
		a = static_cast<KeyboardKeys>(static_cast<uint16_t>(a) + static_cast<uint16_t>(b));
	}

	inline void operator-=(KeyboardKeys &a, KeyboardKeys b)
	{
		a = static_cast<KeyboardKeys>(static_cast<uint16_t>(a) - static_cast<uint16_t>(b));
	}

	inline bool operator&(KeyboardKeys a, KeyboardKeys b)
	{
		bool result = (static_cast<uint16_t>(a) & static_cast<uint16_t>(b)) > 0;
		return result;
	}

	inline std::ostream& operator<<(std::ostream &out, KeyboardKeys a)
	{
		static std::map<KeyboardKeys, std::string> keyPairs
		{
			{KeyboardKeys::KEY_NONE, "KEY_NONE "},
			{KeyboardKeys::KEY_W, "KEY_W "},
			{KeyboardKeys::KEY_A, "KEY_A "},
			{KeyboardKeys::KEY_S, "KEY_S "},
			{KeyboardKeys::KEY_D, "KEY_D "},
			{KeyboardKeys::KEY_UP, "KEY_UP "},
			{KeyboardKeys::KEY_DOWN, "KEY_DOWN "}
		};
		
		if (a == KeyboardKeys::KEY_NONE)
			out << keyPairs[KeyboardKeys::KEY_NONE];;
		if (a & KeyboardKeys::KEY_W)
			out << keyPairs[KeyboardKeys::KEY_W];
		if (a & KeyboardKeys::KEY_A)
			out << keyPairs[KeyboardKeys::KEY_A];
		if (a & KeyboardKeys::KEY_S)
			out << keyPairs[KeyboardKeys::KEY_S];
		if (a & KeyboardKeys::KEY_D)
			out << keyPairs[KeyboardKeys::KEY_D];
		if (a & KeyboardKeys::KEY_UP)
			out << keyPairs[KeyboardKeys::KEY_UP];
		if (a & KeyboardKeys::KEY_DOWN)
			out << keyPairs[KeyboardKeys::KEY_DOWN];
		return out;
	}

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

		std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};

		std::vector<uint16_t> indices = {
			0,1,2,2,3,0
		};

		


		KeyboardKeys pressedKeys;
		

		float viewPitch;
		const float minPitch = -0.1f;
		const float maxPitch = -2.0f;
		const float pitchFactor = 0.01f;

		float viewRotation;
		const float rotationFactor = 0.03f;
		
		float viewZoom;
		const float zoomFactor = 0.015f;
		//They are inverted, yes
		const float maxZoom = 0.2f;
		const float minZoom = 3.0f;



		uint64_t frameCounter = 0;
		

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

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;

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
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipeline graphicsPipeline;
		VkCommandPool commandPool;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		QueueFamilyIndices queueFamilies;
		VkImage textureImage;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkDeviceMemory textureImageMemory;

		uint32_t w_width;
		uint32_t w_height;


		void initWindow(const uint32_t width, const uint32_t height);
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
		void initVulkan();
		void createSurface();
		bool checkValidationLayerSupport();
		void mainLoop();
		void cleanup();
		void createInstance();
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void updateVertexDataToMemory();
		void pickPhysicalDevices();
		void createLogicalDevice();
		void createSwapChain();
		void recreateSwapChain();
		void cleanupSwapChain();
		void createImageViews();
		void createRenderPass();
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
		void createFrameBuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createTextureImage();
		void createVertextBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void createSyncObjects();
		void updateUniformBuffer(uint32_t currentImage);
		void updateViewPosition();
		void drawFrame();
		
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		void createTextureSampler();
		VkImageView createImageView(VkImage image, VkFormat format);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
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


