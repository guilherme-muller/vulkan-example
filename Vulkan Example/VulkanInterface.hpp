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
		KEY_DOWN = 1 << 6,
		KEY_LEFT = 1 << 7,
		KEY_RIGHT = 1 << 8
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
//#define FORCE_MAX_MSAA

#ifdef BIG_MODEL_LOAD
		const std::string MODEL_PATH = "models/estances_lq.obj";
		const std::string TEXTURE_PATH = "textures/estances_lq_u1_v1.jpg";
		const glm::vec3 modelScale = { 0.1f, 0.1f, 0.1f };
#else
		const std::string MODEL_PATH = "models/viking_room.obj";
		const std::string TEXTURE_PATH = "textures/viking_room.png";
		const glm::vec3 modelScale = { 1.0f, 1.0f, 1.0f };
#endif

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		


		KeyboardKeys pressedKeys;
		

		float lastX = 400, lastY = 300;
		bool firstMouse = false;
		float pitchStep = 0.03f;
		float yaw = -90.0f;
		float pitch = 30.0f;
		float rotation = 1.0f;
		const float cameraSpeed = 0.015f;
		glm::vec3 cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
		glm::vec3 cameraFront = glm::vec3(-2.0f, -2.0f, -2.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);


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
		uint32_t mipLevels;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkImage textureImage;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkDeviceMemory textureImageMemory;

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		uint32_t w_width;
		uint32_t w_height;


		void initWindow(const uint32_t width, const uint32_t height);
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
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
		void createColorResources();
		void createDepthResources();
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
		VkSampleCountFlagBits getMaxUsableSampleCount();
		void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		void createTextureSampler();
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
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
		std::optional<VkFormat> findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);

		void loadModel(glm::vec3 position, glm::vec3 scale);
		static std::vector<char> readFile(const std::string& filename);
	};

} //namespace


