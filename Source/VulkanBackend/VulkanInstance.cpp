// Copyright (c) 2022 Dhiraj Wishal

#include "VulkanInstance.hpp"
#include "VulkanMacros.hpp"

#include <fstream>
#include <array>
#include <set>

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
		static auto outputFile = std::ofstream("DebugOutput.txt");
		outputFile << "Vulkan Validation Layer : ";

		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
			outputFile << "GENERAL | ";

		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			outputFile << "VALIDATION | ";

		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			outputFile << "PERFORMANCE | ";

		outputFile << pCallbackData->pMessage << std::endl;
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
			throw minte::BackendError("Failed to get the queue family property count!");

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
			throw minte::BackendError("Failed to get the queue family property count!");

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
}

namespace minte
{
	VulkanInstance::VulkanInstance()
	{
		static StaticInitializer initializer;

		setupInstance();
		setupDevice();
	}

	VulkanInstance::~VulkanInstance()
	{
		vkDestroyDevice(m_LogicalDevice, VK_NULL_HANDLE);

#ifdef MINTE_DEBUG
		const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_Debugger, VK_NULL_HANDLE);

#endif

		vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
	}

	void VulkanInstance::setupInstance()
	{
		// Setup the application info.
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = VK_NULL_HANDLE;
		applicationInfo.apiVersion = VK_API_VERSION_1_3;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pApplicationName = "Minte";
		applicationInfo.pEngineName = "Lamiaceae";	// Mentha is a genus of plants in the family Lamiaceae.

		// Setup the instance create info.
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;

#ifdef MINTE_DEBUG
		// Emplace the required validation layer.
		m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

		// Create the debug messenger create info structure.
		const auto debugMessengerCreateInfo = CreateDebugMessengerCreateInfo();

		// Submit it to the instance.
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		createInfo.pNext = &debugMessengerCreateInfo;

		const std::vector<const char*> requiredExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
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
		// Enumerate physical devices.
		uint32_t deviceCount = 0;
		MINTE_VK_ASSERT(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, VK_NULL_HANDLE), "Failed to enumerate physical devices.");

		// Throw an error if there are no physical devices available.
		if (deviceCount == 0)
			throw BackendError("No physical devices found!");

		std::vector<VkPhysicalDevice> candidates(deviceCount);
		MINTE_VK_ASSERT(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, candidates.data()), "Failed to enumerate physical devices.");

		struct Candidate { VkPhysicalDeviceProperties m_Properties; VkPhysicalDevice m_Candidate; };
		std::array<Candidate, 6> priorityMap = { Candidate() };

		// Iterate through all the candidate devices and find the best device.
		for (const auto& candidate : candidates)
		{
			// Check if the device is suitable for our use.
			if (CheckQueueSupport(candidate, VK_QUEUE_GRAPHICS_BIT) &&
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
			throw BackendError("Failed to find a suitable physical device!");

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
		features.samplerAnisotropy = VK_TRUE;
		features.sampleRateShading = VK_TRUE;
		features.tessellationShader = VK_TRUE;
		features.geometryShader = VK_TRUE;

		// Setup the device create info.
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = VK_NULL_HANDLE;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.ppEnabledExtensionNames = VK_NULL_HANDLE;
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

}