// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Backend/VulkanBackend/VulkanInstance.hpp"
#include "Minte/Backend/VulkanBackend/VulkanMacros.hpp"

#include <spdlog/spdlog.h>

#if defined(MINTE_PLATFORM_WINDOWS)
#include <vulkan/vulkan_win32.h>

#endif

#include <sstream>
#include <array>
#include <set>

constexpr uint32_t VulkanVersion = VK_API_VERSION_1_3;

namespace /* anonymous */
{
	/**
	 * Vulkan debug callback.
	 * This function is used by Vulkan to report any internal message to the user.
	 *
	 * @param messageSeverity The severity of the message.
	 * @param messageType The type of the message.
	 * @param pCallbackData The data passed by the API.
	 * @param The user data submitted to Vulkan before this call.
	 * @return The status return.
	 */
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::stringstream messageStream;
		messageStream << "Vulkan Validation Layer : ";

		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
			messageStream << "GENERAL | ";

		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			messageStream << "VALIDATION | ";

		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			messageStream << "PERFORMANCE | ";

		messageStream << pCallbackData->pMessage;

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			spdlog::warn(messageStream.str());
			break;

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			spdlog::error(messageStream.str());
			break;

		default:
			spdlog::info(messageStream.str());
			break;
		}

		return VK_FALSE;
	}

	/**
	 * Create the default debug messenger create info structure.
	 *
	 * @return The created structure.
	 */
	VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.pNext = VK_NULL_HANDLE;
		createInfo.pUserData = VK_NULL_HANDLE;
		createInfo.flags = 0;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		return createInfo;
	}

	/**
	 * Static initializer struct.
	 * These structs are used to initialize data that are to be initialized just once in the application.
	 */
	struct StaticInitializer final
	{
		/**
		 * Default constructor.
		 */
		StaticInitializer()
		{
			// Initialize volk.
			MINTE_VK_ASSERT(volkInitialize(), "Failed to initialize volk!");
		}
	};

	/**
	 * Check if the physical device supports the required queues.
	 *
	 * @param physicalDevice The physical device to check.
	 * @param flag The queue flag to check.
	 * @return Whether or not the queues are supported.
	 */
	bool CheckQueueSupport(VkPhysicalDevice physicalDevice, VkQueueFlagBits flag)
	{
		// Get the queue family count.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, VK_NULL_HANDLE);

		// Validate if we have queue families.
		if (queueFamilyCount == 0)
			throw minte::backend::BackendError("Failed to get the queue family property count!");

		// Get the queue family properties.
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		// Iterate over those queue family properties and check if we have a family with the required flag.
		for (const auto& family : queueFamilies)
		{
			if (family.queueCount == 0)
				continue;

			// Check if the queue flag contains what we want.
			if (family.queueFlags & flag)
				return true;
		}

		return false;
	}

	/**
	 * Get the queue family of a queue.
	 *
	 * @param physicalDevice The physical device to get the queue family from.
	 * @param flag The queue flag.
	 * @retrun The queue family.
	 */
	uint32_t GetQueueFamily(VkPhysicalDevice physicalDevice, VkQueueFlagBits flag)
	{
		// Get the queue family count.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, VK_NULL_HANDLE);

		// Validate if we have queue families.
		if (queueFamilyCount == 0)
			throw minte::backend::BackendError("Failed to get the queue family property count!");

		// Get the queue family properties.
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		// Iterate over those queue family properties and check if we have a family with the required flag.
		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			const auto& family = queueFamilies[i];
			if (family.queueCount == 0)
				continue;

			// Check if the queue flag contains what we want.
			if (family.queueFlags & flag)
				return i;
		}

		return -1;
	}

	/**
	 * Check device extension support.
	 *
	 * @param physicalDevice The physical device to check.
	 * @param deviceExtensions The extension to check.
	 */
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions)
	{
		// If there are no extension to check, we can just return true.
		if (deviceExtensions.empty())
			return true;

		// Get the extension count.
		uint32_t extensionCount = 0;
		MINTE_VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &extensionCount, VK_NULL_HANDLE), "Failed to enumerate physical device extension property count!");

		// Load the extensions.
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		MINTE_VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &extensionCount, availableExtensions.data()), "Failed to enumerate physical device extension properties!");

		std::set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		// Iterate and check if it contains the extensions we need. If it does, remove them from the set so we can later check if 
		// all the required extensions exist.
		for (const VkExtensionProperties& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// If the required extensions set is empty, it means that all the required extensions exist within the physical device.
		return requiredExtensions.empty();
	}

	/**
	 * Get the pipeline stage flags from access flags.
	 *
	 * @param flags Access flags.
	 * @return The stage flags.
	 */
	VkPipelineStageFlags GetPipelineStageFlags(VkAccessFlags flags)
	{
		switch (flags)
		{
		case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:						return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		case VK_ACCESS_INDEX_READ_BIT:									return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:						return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		case VK_ACCESS_UNIFORM_READ_BIT:								return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case VK_ACCESS_SHADER_READ_BIT:									return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case VK_ACCESS_SHADER_WRITE_BIT:								return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:						return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:						return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:						return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:				return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:				return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_ACCESS_TRANSFER_READ_BIT:								return VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		case VK_ACCESS_TRANSFER_WRITE_BIT:								return VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		case VK_ACCESS_HOST_READ_BIT:									return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_ACCESS_HOST_WRITE_BIT:									return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_ACCESS_MEMORY_READ_BIT:									return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_ACCESS_MEMORY_WRITE_BIT:								return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT:		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV:					return VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
		case VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV:					return VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
		case VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT:				return VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
		case VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR:	return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		case VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:				return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
		case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:		return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
		case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT:			return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		case VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR:				return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		case VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:			return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		case VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT:				return VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
		default:														return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}
	}
}

namespace minte
{
	namespace backend
	{
		VulkanInstance::VulkanInstance()
		{
			static StaticInitializer initializer;

			setupInstance();
			setupDevice();
			setupAllocator();
		}

		VulkanInstance::~VulkanInstance()
		{
			vmaDestroyAllocator(m_Allocator);
			vkDestroyDevice(m_LogicalDevice, VK_NULL_HANDLE);

#ifdef MINTE_DEBUG
			const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_Debugger, VK_NULL_HANDLE);

#endif

			vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
		}

		void VulkanInstance::changeImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels /*= 1*/, uint32_t layers /*= 1*/) const
		{
			// Create the memory barrier.
			VkImageMemoryBarrier memorybarrier = {};
			memorybarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memorybarrier.srcAccessMask = 0;
			memorybarrier.dstAccessMask = 0;
			memorybarrier.oldLayout = currentLayout;
			memorybarrier.newLayout = newLayout;
			memorybarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memorybarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memorybarrier.image = image;
			memorybarrier.subresourceRange.aspectMask = aspectFlags;
			memorybarrier.subresourceRange.baseMipLevel = 0;
			memorybarrier.subresourceRange.levelCount = mipLevels;
			memorybarrier.subresourceRange.baseArrayLayer = 0;
			memorybarrier.subresourceRange.layerCount = layers;

			// Resolve the source access masks.
			switch (currentLayout)
			{
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_UNDEFINED:
				memorybarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				memorybarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				//vMB.srcAccessMask = VK_ACCESS_;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				memorybarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				throw backend::BackendError("Unsupported layout transition!");
			}

			// Resolve the destination access masks.
			switch (newLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				memorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				memorybarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				memorybarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				memorybarrier.dstAccessMask = memorybarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				memorybarrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				memorybarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				throw backend::BackendError("Unsupported layout transition!");
			}

			// Resolve the pipeline stages.
			const auto sourceStage = GetPipelineStageFlags(memorybarrier.srcAccessMask);
			const auto destinationStage = GetPipelineStageFlags(memorybarrier.dstAccessMask);

			// Issue the commands. 
			getDeviceTable().vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &memorybarrier);
		}

		void VulkanInstance::setupInstance()
		{
			// Setup the application info.
			VkApplicationInfo applicationInfo = {};
			applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			applicationInfo.pNext = VK_NULL_HANDLE;
			applicationInfo.apiVersion = VulkanVersion;
			applicationInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
			applicationInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
			applicationInfo.pApplicationName = "Minte";
			applicationInfo.pEngineName = "Lamiaceae";	// Mentha is a genus of plants in the family Lamiaceae.

			// Setup the instance create info.
			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &applicationInfo;

			std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME , VK_KHR_DISPLAY_EXTENSION_NAME };

#if defined(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);

#elif defined(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME);

#elif defined(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME);

#elif defined(VK_MVK_IOS_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);

#elif defined(VK_MVK_MACOS_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);

#elif defined(VK_EXT_METAL_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);

#elif defined(VK_NN_VI_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_NN_VI_SURFACE_EXTENSION_NAME);

#elif defined(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);

#elif defined(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#elif defined(VK_KHR_XCB_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

#elif defined(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);

#elif defined(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);

#elif defined(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);

#elif defined(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME);

#elif defined(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME)
			requiredExtensions.emplace_back(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);

#endif

#ifdef MINTE_DEBUG
			// Emplace the required validation layer.
			m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

			// Create the debug messenger create info structure.
			const auto debugMessengerCreateInfo = CreateDebugMessengerCreateInfo();

			// Submit it to the instance.
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
			createInfo.pNext = &debugMessengerCreateInfo;

			requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
			createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#else
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = VK_NULL_HANDLE;
			createInfo.enabledExtensionCount = 0;
			createInfo.ppEnabledExtensionNames = VK_NULL_HANDLE;

#endif

			// Create the instance.
			MINTE_VK_ASSERT(vkCreateInstance(&createInfo, VK_NULL_HANDLE, &m_Instance), "Failed to create the instance!");

			// Load the instance functions.
			volkLoadInstance(m_Instance);

#ifdef MINTE_DEBUG
			// Create the debugger if possible.
			const auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
			MINTE_VK_ASSERT(vkCreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerCreateInfo, VK_NULL_HANDLE, &m_Debugger), "Failed to create the debug messenger.");

#endif
		}

		void VulkanInstance::setupDevice()
		{
			const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME };

			// Enumerate physical devices.
			uint32_t deviceCount = 0;
			MINTE_VK_ASSERT(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, VK_NULL_HANDLE), "Failed to enumerate physical devices.");

			// Throw an error if there are no physical devices available.
			if (deviceCount == 0)
				throw backend::BackendError("No physical devices found!");

			std::vector<VkPhysicalDevice> candidates(deviceCount);
			MINTE_VK_ASSERT(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, candidates.data()), "Failed to enumerate physical devices.");

			struct Candidate { VkPhysicalDeviceProperties m_Properties; VkPhysicalDevice m_Candidate; };
			std::array<Candidate, 6> priorityMap = { Candidate() };

			// Iterate through all the candidate devices and find the best device.
			for (const auto& candidate : candidates)
			{
				// Check if the device is suitable for our use.
				if (CheckDeviceExtensionSupport(candidate, deviceExtensions) &&
					CheckQueueSupport(candidate, VK_QUEUE_GRAPHICS_BIT) &&
					CheckQueueSupport(candidate, VK_QUEUE_COMPUTE_BIT) &&
					CheckQueueSupport(candidate, VK_QUEUE_TRANSFER_BIT))
				{
					VkPhysicalDeviceProperties physicalDeviceProperties = {};
					vkGetPhysicalDeviceProperties(candidate, &physicalDeviceProperties);

					// Sort the candidates by priority.
					uint8_t priorityIndex = 5;
					switch (physicalDeviceProperties.deviceType)
					{
					case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
						priorityIndex = 0;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
						priorityIndex = 1;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
						priorityIndex = 2;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_CPU:
						priorityIndex = 3;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_OTHER:
						priorityIndex = 4;
						break;

					default:
						priorityIndex = 5;
						break;
					}

					priorityMap[priorityIndex].m_Candidate = candidate;
					priorityMap[priorityIndex].m_Properties = physicalDeviceProperties;
				}
			}

			// Choose the physical device with the highest priority.
			for (const auto& candidate : priorityMap)
			{
				if (candidate.m_Candidate != VK_NULL_HANDLE)
				{
					m_PhysicalDevice = candidate.m_Candidate;
					m_PhysicalDeviceProperties = candidate.m_Properties;
					break;
				}
			}

			// Throw and error if a physical device was not found.
			if (m_PhysicalDevice == VK_NULL_HANDLE)
				throw backend::BackendError("Failed to find a suitable physical device!");

			// Setup device queues.
			constexpr float priority = 1.0f;
			std::set<uint32_t> uniqueQueueFamilies = {
				m_GraphicsQueue.m_Family,
				m_ComputeQueue.m_Family,
				m_TransferQueue.m_Family
			};

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = VK_NULL_HANDLE;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = 0;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &priority;

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			for (const auto& family : uniqueQueueFamilies)
			{
				queueCreateInfo.queueFamilyIndex = family;
				queueCreateInfos.emplace_back(queueCreateInfo);
			}

			// Setup all the required features.
			VkPhysicalDeviceFeatures features = {};
			// features.samplerAnisotropy = VK_TRUE;
			// features.sampleRateShading = VK_TRUE;
			// features.tessellationShader = VK_TRUE;
			// features.geometryShader = VK_TRUE;

			// Setup the device create info.
			VkDeviceCreateInfo deviceCreateInfo = {};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pNext = VK_NULL_HANDLE;
			deviceCreateInfo.flags = 0;
			deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceCreateInfo.enabledLayerCount = 0;
			deviceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
			deviceCreateInfo.pEnabledFeatures = &features;

#ifdef MINTE_DEBUG
			// Get the validation layers and initialize it.
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();

#endif

			// Create the device.
			MINTE_VK_ASSERT(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_LogicalDevice), "Failed to create the logical device!");

			// Load the device table.
			volkLoadDeviceTable(&m_DeviceTable, m_LogicalDevice);

			// Get the queues.
			m_DeviceTable.vkGetDeviceQueue(m_LogicalDevice, m_GraphicsQueue.m_Family, 0, &m_GraphicsQueue.m_Queue);
			m_DeviceTable.vkGetDeviceQueue(m_LogicalDevice, m_ComputeQueue.m_Family, 0, &m_ComputeQueue.m_Queue);
			m_DeviceTable.vkGetDeviceQueue(m_LogicalDevice, m_TransferQueue.m_Family, 0, &m_TransferQueue.m_Queue);
		}

		void VulkanInstance::setupAllocator()
		{
			// Setup the Vulkan functions needed by VMA.
			VmaVulkanFunctions functions = {};
			functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
			functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
			functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
			functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
			functions.vkAllocateMemory = m_DeviceTable.vkAllocateMemory;
			functions.vkFreeMemory = m_DeviceTable.vkFreeMemory;
			functions.vkMapMemory = m_DeviceTable.vkMapMemory;
			functions.vkUnmapMemory = m_DeviceTable.vkUnmapMemory;
			functions.vkFlushMappedMemoryRanges = m_DeviceTable.vkFlushMappedMemoryRanges;
			functions.vkInvalidateMappedMemoryRanges = m_DeviceTable.vkInvalidateMappedMemoryRanges;
			functions.vkBindBufferMemory = m_DeviceTable.vkBindBufferMemory;
			functions.vkBindImageMemory = m_DeviceTable.vkBindImageMemory;
			functions.vkGetBufferMemoryRequirements = m_DeviceTable.vkGetBufferMemoryRequirements;
			functions.vkGetImageMemoryRequirements = m_DeviceTable.vkGetImageMemoryRequirements;
			functions.vkCreateBuffer = m_DeviceTable.vkCreateBuffer;
			functions.vkDestroyBuffer = m_DeviceTable.vkDestroyBuffer;
			functions.vkCreateImage = m_DeviceTable.vkCreateImage;
			functions.vkDestroyImage = m_DeviceTable.vkDestroyImage;
			functions.vkCmdCopyBuffer = m_DeviceTable.vkCmdCopyBuffer;
			functions.vkGetBufferMemoryRequirements2KHR = m_DeviceTable.vkGetBufferMemoryRequirements2KHR;
			functions.vkGetImageMemoryRequirements2KHR = m_DeviceTable.vkGetImageMemoryRequirements2KHR;
			functions.vkBindBufferMemory2KHR = m_DeviceTable.vkBindBufferMemory2KHR;
			functions.vkBindImageMemory2KHR = m_DeviceTable.vkBindImageMemory2KHR;
			functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
			functions.vkGetDeviceBufferMemoryRequirements = m_DeviceTable.vkGetDeviceBufferMemoryRequirements;
			functions.vkGetDeviceImageMemoryRequirements = m_DeviceTable.vkGetDeviceImageMemoryRequirements;

			// Setup create info.
			VmaAllocatorCreateInfo createInfo = {};
			// createInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
			createInfo.flags = 0;
			createInfo.physicalDevice = m_PhysicalDevice;
			createInfo.device = m_LogicalDevice;
			createInfo.pVulkanFunctions = &functions;
			createInfo.instance = m_Instance;
			createInfo.vulkanApiVersion = VulkanVersion;

			MINTE_VK_ASSERT(vmaCreateAllocator(&createInfo, &m_Allocator), "Failed to create the allocator!");
		}
	}
}