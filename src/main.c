#include <GL/gl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>


void recordCommandBuffer(VkPipeline *gpuPipeline, VkOffset2D* offset,  VkExtent2D *extend, VkFramebuffer *swapChainFrammeBuffers, VkCommandBuffer *commandBuffer,VkRenderPass *renderPass, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(*commandBuffer, &beginInfo) != VK_SUCCESS) {
        exit(1);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = *renderPass;
    renderPassInfo.framebuffer = swapChainFrammeBuffers[imageIndex];
    renderPassInfo.renderArea.offset = *offset;
    renderPassInfo.renderArea.extent = *extend;
    

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *gpuPipeline);

    vkCmdDraw(*commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(*commandBuffer);
    if (vkEndCommandBuffer(*commandBuffer) != VK_SUCCESS) {
       exit(1);
    }


}
void drawFrame(VkQueue *presentQueue, VkQueue* gpuQueue, VkPipeline *gpuPipeline, VkOffset2D* offset,  VkExtent2D *extend, VkFramebuffer *swapChainFrammeBuffers, VkCommandBuffer *commandBuffer,VkRenderPass *renderPass, VkDevice *device, VkFence *inFlightFence, VkSwapchainKHR* swapChain, VkSemaphore *imageAvailableSemaphore,VkSemaphore * renderFinishedSemaphore){

      uint32_t imageIndex;
    vkAcquireNextImageKHR(*device, *swapChain, UINT64_MAX, *imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);


    vkResetCommandBuffer(*commandBuffer, 0);
    vkWaitForFences(*device, 1, inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(*device, 1, inFlightFence);
    recordCommandBuffer(gpuPipeline, offset, extend, swapChainFrammeBuffers, commandBuffer, renderPass,  imageIndex);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {*imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;
    VkSemaphore signalSemaphores[] = {*renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(*gpuQueue, 1, &submitInfo, *inFlightFence) != VK_SUCCESS) {
      exit(1);
    }


    VkPresentInfoKHR presentInfo= {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {*swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL; 
    vkQueuePresentKHR(*presentQueue, &presentInfo);

};



int main(){
    GLFWwindow *window ;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface;
    VkQueue gpuQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain; 
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormart;
    VkSurfaceCapabilitiesKHR capabilities;
    VkPipeline gpuPipeline;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);



    const char* validationLayers[layerCount];
    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (int i =0; i < layerCount; i++) {
  
          validationLayers[i] = availableLayers[i].layerName;


    }





    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;


    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "Triangle", 0, 0);


    //instance
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    //creating app info
    VkApplicationInfo appInfo = {
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      NULL,
      "Hello Triangle",
      VK_MAKE_VERSION(1, 0, 0),
      "No Engine",
      VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo createInfo = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &appInfo,
      layerCount,
      validationLayers,
      glfwExtensionCount,
      glfwExtensions
    };
    
    if(vkCreateInstance(&createInfo, NULL, &instance) != 0){
      exit(1);
    }
    printf("created instance\n");


    //surface 
    if(glfwCreateWindowSurface(instance, window, 0, &surface) != 0){
            exit(1);

    }
    printf("created surface\n");






    uint32_t deviceCount = 0;
  
  
    vkEnumeratePhysicalDevices(instance,&deviceCount, NULL);
    if(deviceCount==0){
        exit(1);
    }
    if(vkEnumeratePhysicalDevices(instance,&deviceCount, &physicalDevice) != 0){
        exit(1);
    }
    
    printf("created physical device\n");





    //logical device


    VkPhysicalDeviceFeatures deviceFeatures;
   
  
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
  
    const char* deviceExtensions[256] = {"VK_KHR_swapchain", "VK_KHR_pipeline_executable_properties", "VK_KHR_create_renderpass2", "VK_KHR_pipeline_library", "VK_KHR_dynamic_rendering", "VK_KHR_spirv_1_4", "VK_KHR_swapchain_mutable_format", "VK_EXT_pci_bus_info"};
    float queuePriority = 1.0f;



    uint32_t queueCountProperties;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCountProperties, NULL);




    VkDeviceQueueCreateInfo gpuQueueInfo = {};
      gpuQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      gpuQueueInfo.queueCount = 1;
      gpuQueueInfo.queueFamilyIndex = 0;
      gpuQueueInfo.pQueuePriorities = &queuePriority;

   VkDeviceQueueCreateInfo presentQueueInfo = {};
      presentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      presentQueueInfo.queueCount = 1;
      presentQueueInfo.queueFamilyIndex = 1;
      presentQueueInfo.pQueuePriorities = &queuePriority;


    VkDeviceQueueCreateInfo queueCreateInfos[] = {gpuQueueInfo, presentQueueInfo};


    VkDeviceCreateInfo createDeviceInfo = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      NULL,
      0,
      2,
      queueCreateInfos,
      0,
      0,
      1,
      deviceExtensions,
      &deviceFeatures
    };

    if(vkCreateDevice(physicalDevice, &createDeviceInfo, 0, &device) != 0){
          exit(1);
    }
    
    printf("created device\n");
   

    vkGetDeviceQueue(device, 0, 0, &gpuQueue);
    vkGetDeviceQueue(device, 1, 0, &presentQueue);
    if(gpuQueue == VK_NULL_HANDLE || presentQueue == VK_NULL_HANDLE){
        exit(1);
    }

    printf("created queues\n");





        ///swap
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);


    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    VkSurfaceFormatKHR formats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);



  
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
  VkPresentModeKHR presentmodes[presentModeCount];
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,  presentmodes);


  for (int i =0; i < formatCount; i++) {
    if(formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
        surfaceFormart = formats[i];
    }
  }

   for (int i =0; i < presentModeCount; i++) {
    if(presentmodes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
        presentMode = presentmodes[i];
    }
   }

    VkExtent2D extend = {
        800,600
    };
    uint32_t imageCount  = capabilities.minImageCount + 2;
    const uint32_t indices[2] = {0, 1};
  
    VkSwapchainCreateInfoKHR createSwapInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        0,
        1,
        surface,
        imageCount,
        surfaceFormart.format,
        surfaceFormart.colorSpace,
        extend,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_CONCURRENT,
        2,
        indices,
        capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };

    vkCreateSwapchainKHR(device, &createSwapInfo, NULL, &swapChain);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    
    VkImage swapChainImages[imageCount];
    VkImageView swapChainImageViews[imageCount];

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);
    
    for (int i = 0; i < imageCount; i++) {
      
      VkImageViewCreateInfo createImageViewInfo = {};
        createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createImageViewInfo.image = swapChainImages[i];
        createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createImageViewInfo.format = surfaceFormart.format;
        createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createImageViewInfo.subresourceRange.baseMipLevel = 0;
        createImageViewInfo.subresourceRange.levelCount = 1;
        createImageViewInfo.subresourceRange.baseArrayLayer = 0;
        createImageViewInfo.subresourceRange.layerCount = 1;
        if(vkCreateImageView(device, &createImageViewInfo, NULL, &swapChainImageViews[i]) != 0){
            printf("not created image view");
        
        }
    }
    printf("created image view\n");
    FILE* vertx = fopen("vert.spv", "rb");
    FILE* fragx = fopen("frag.spv", "rb");

    if(vertx == NULL || fragx == NULL){
      exit(1);
    }

    printf("created file stream\n");

    fseek(vertx, 0L, SEEK_END);
    long int vertxSize = ftell(vertx);
    fseek(vertx, 0L, SEEK_SET);

    fseek(fragx, 0L, SEEK_END);
    long int fragxSize = ftell(fragx);
    fseek(fragx, 0L, SEEK_SET);

    printf("vertx size is %li and frag size is %li\n", vertxSize, fragxSize);


    char vertxBuffer[vertxSize];
    char fragxBuffer[fragxSize];

    fread(vertxBuffer, 1, vertxSize, vertx);
    fread(fragxBuffer, 1, fragxSize, fragx);
    fclose(vertx);
    fclose(fragx);

    VkShaderModuleCreateInfo createVertxModuleInfo = {};
    createVertxModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createVertxModuleInfo.codeSize = vertxSize;
    createVertxModuleInfo.pCode = (const uint32_t *)vertxBuffer;

    
    VkShaderModuleCreateInfo createFragxModuleInfo = {};
    createFragxModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createFragxModuleInfo.codeSize = fragxSize;
    createFragxModuleInfo.pCode = (const uint32_t *)fragxBuffer;  

    VkShaderModule vertxShaderModule;
    VkShaderModule fragxShaderModule;



    VkResult resultVertx = vkCreateShaderModule(device, &createVertxModuleInfo, NULL, &vertxShaderModule);
    VkResult resultFragx = vkCreateShaderModule(device, &createFragxModuleInfo, NULL, &fragxShaderModule);
   
   
   
    if(resultVertx != 0 || resultFragx != 0){
      exit(1);
    }
    printf("modules created\n");



    VkPipelineShaderStageCreateInfo vertxShaderStageInfo = {};
    vertxShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertxShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertxShaderStageInfo.module = vertxShaderModule;
    vertxShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragxShaderStageInfo = {};
    fragxShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragxShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragxShaderStageInfo.module = fragxShaderModule;
    fragxShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[2]= {vertxShaderStageInfo, fragxShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL; 
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extend.width;
    viewport.height = (float) extend.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;


    VkOffset2D offset= {0, 0};

    VkRect2D scissor = {};
    scissor.offset = offset;
    scissor.extent = extend;

    VkPipelineViewportStateCreateInfo viewportState  = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional



    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending  = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional




    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;




    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = surfaceFormart.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef ={};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) {
        exit(1);
    }
    printf("render pass created\n");



    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = NULL; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        exit(1);
    }
    printf("pipeline created\n");



	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = NULL;

	pipelineInfo.stageCount = (const uint32_t)2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
  pipelineInfo.pDynamicState = NULL;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.pTessellationState = NULL; 

    
    if(vkCreateGraphicsPipelines(device, NULL, 1, &pipelineInfo, NULL, &gpuPipeline) != VK_SUCCESS){
      exit(1);
    };

    printf("pipeline gpu created\n");

    VkFramebuffer swapChainFrammeBuffers[imageCount];
    for (int i = 0; i < imageCount; i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
      };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extend.width;
        framebufferInfo.height = extend.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFrammeBuffers[i]) != VK_SUCCESS) {
            exit(1);
        }
    }
    printf("created frame buffer\n");


    VkCommandPool commandPool;


    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = 0;

    if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
    exit(1);
    }


    printf("created command pool\n");


    VkCommandBuffer commandBuffer;


    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        exit(1);
    }
       printf("created command buffer\n");




    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, NULL, &inFlightFence) != VK_SUCCESS) {
       exit(1);
    }






  while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        

         drawFrame(&presentQueue, &gpuQueue, &gpuPipeline, &offset, &extend, swapChainFrammeBuffers, &commandBuffer, &renderPass, &device, &inFlightFence, &swapChain, &imageAvailableSemaphore, &renderFinishedSemaphore); 
         
         
           }

        vkDeviceWaitIdle(device);

    return 0;
}



