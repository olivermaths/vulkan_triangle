#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "../includes/logger.h"


//defines
#define NDEBUG 0


enum TR_BOOL{
  TR_FALSE,
  TR_TRUE,
};

struct queue_family_indices{
  uint32_t gpuFamily;
  uint32_t presentFamily[2];
};

struct tr_window{
  GLFWwindow *window;
  int width;
  int height;
  const char* title;
};



struct tr_renderer{
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physicalDevice;
  VkQueue gpuQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;

};

struct tr_application{
  struct tr_window *p_window;
  struct tr_renderer *p_renderer;
  enum TR_BOOL isRunning;
  void(*run)(enum TR_BOOL* isRunning);
};



void createInstance(VkInstance *instance){
 
  // available extensions 
  uint32_t extensionCount = 0;
    
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  HCT_TRACE("available extensions count: %i", extensionCount);
  VkExtensionProperties extensions[extensionCount];
  HCT_TRACE("getting available extensions", 0);

  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

  for (int i = 0; i < extensionCount; i++) {
    HCT_INFO("available extension %s", extensions[i].extensionName);
  }



  

    //glfw Extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    
    HCT_TRACE("Getting glfwExtensions", 0);
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    HCT_TRACE("extensionsCount %i", glfwExtensionCount);
    
    for (int i = 0; i < glfwExtensionCount; i++) {
        HCT_INFO("extensions %i: %s",i, glfwExtensions[i]);
    }



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
      0,
      NULL,
      glfwExtensionCount,
      glfwExtensions
    };
    

    HCT_TRACE("creating vulkan instance", 0);
    if(vkCreateInstance(&createInfo, NULL, instance) == 0){
      HCT_INFO("Vulkan instance created!!", 0);
    }else{
      HCT_FATAL("Vulkan instance not created", 0);
      exit(1);
    }

}



struct queue_family_indices findQueueFamilies(struct tr_renderer *p_renderer){
    uint32_t queueFamilycount = 0;
    struct queue_family_indices indices;
    indices.gpuFamily = -1;
    indices.presentFamily[0] = -1;
    indices.presentFamily[1] = -1;
    vkGetPhysicalDeviceQueueFamilyProperties(p_renderer->physicalDevice, &queueFamilycount, 0);
    VkQueueFamilyProperties queueFamilies[queueFamilycount];
    
    HCT_TRACE("getting queue families", 0);
    vkGetPhysicalDeviceQueueFamilyProperties(p_renderer->physicalDevice, &queueFamilycount, queueFamilies);

    int presentIndex = 0;
    for (int i =0; i < queueFamilycount; i++) {
      if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
          indices.gpuFamily = i;
      }
      VkBool32 presentSupport = 0;
      vkGetPhysicalDeviceSurfaceSupportKHR(p_renderer->physicalDevice, i, p_renderer->surface, &presentSupport);
      if(presentSupport){
        HCT_INFO("the surface can be presented, %i", i);
        indices.presentFamily[presentIndex] = i;
        presentIndex++;
      }
    
    }

  return indices;
}


HCT_Bool isPhysicalDeviceSuitable(struct tr_renderer *p_renderer){
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(p_renderer->physicalDevice, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  HCT_TRACE("Picking physical device", 0);
  vkGetPhysicalDeviceFeatures(p_renderer->physicalDevice, &deviceFeatures);
  HCT_TRACE("device name: %s", deviceProperties.deviceName);
  HCT_TRACE("device is: %i", deviceProperties.deviceType);
 
  struct queue_family_indices indices = findQueueFamilies(p_renderer);
  
  if(indices.gpuFamily == -1){
    HCT_WARN("couldn't find a queue family", 0 );

  }else{
    HCT_INFO("succed! picked family %i", indices.gpuFamily );
  }

  return HCT_TRUE;
}







void pickPhysicalDevice(struct tr_renderer *trRenderer){
  uint32_t deviceCount = 0;
  
  
  vkEnumeratePhysicalDevices(trRenderer->instance,&deviceCount, 0);
  if(deviceCount==0){
    HCT_ERROR("couldn't find a device",0);
    exit(1);
  }

  vkEnumeratePhysicalDevices(trRenderer->instance,&deviceCount, &trRenderer->physicalDevice);
  isPhysicalDeviceSuitable(trRenderer);
};

void createLogicalDevice(struct tr_renderer *trRenderer){
    
    VkPhysicalDeviceFeatures deviceFeatures;
    HCT_TRACE("Picking physical device features", 0);
  
    vkGetPhysicalDeviceFeatures(trRenderer->physicalDevice, &deviceFeatures);
  

    
    struct queue_family_indices indices = findQueueFamilies(trRenderer);
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfos[3];

    VkDeviceQueueCreateInfo gpuQueueInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      NULL,
      0,
      indices.gpuFamily,
      1,
      &queuePriority
    };

    VkDeviceQueueCreateInfo presentQueueInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      NULL,
      0,
      indices.presentFamily[0],
      1,
      &queuePriority
    };
   VkDeviceQueueCreateInfo presentQueueInfo2 = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      NULL,
      0,
      indices.presentFamily[1],
      1,
      &queuePriority
    };



    queueCreateInfos[0] = gpuQueueInfo;
    queueCreateInfos[1] = presentQueueInfo;
    queueCreateInfos[2] = presentQueueInfo2;


    VkDeviceCreateInfo createInfo = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      NULL,
      0,
      3,
      queueCreateInfos,
      0,
      0,
      0,
      0,
      &deviceFeatures
    };

    HCT_TRACE("creating logical device", 0);
    if(vkCreateDevice(trRenderer->physicalDevice, &createInfo, 0, &trRenderer->device) == 0){
      HCT_INFO("logical device created!!", 0);
    }else{
      HCT_FATAL("logical device couldn't be created!!", 0);
    }

    HCT_TRACE("Retriving device queue family", 0);
    vkGetDeviceQueue(trRenderer->device, indices.gpuFamily, 0, &trRenderer->gpuQueue);
    vkGetDeviceQueue(trRenderer->device, indices.presentFamily[1], 0, &trRenderer->presentQueue);
}


void createSurface(struct tr_renderer *p_renderer, struct tr_window *p_window){
  HCT_TRACE("creating surface", 0);
  if(glfwCreateWindowSurface(p_renderer->instance, p_window->window, 0, &p_renderer->surface) == 0){
    HCT_INFO("surface created!!", 0);
    return;
  }
  HCT_FATAL("couldn't create surface!", 0);
}








//functions

void run(enum TR_BOOL *isRunning){
  while (isRunning) {
    glfwPollEvents();
  };
}



///


//callbacs

void setWindowCallbacks(){}


//
struct tr_window* createTrWindow(const char* title, int width, int height){
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  struct tr_window* trWindow = malloc(sizeof(struct tr_window));
  trWindow->window = glfwCreateWindow(width, height, title, 0, 0);
  trWindow->width = width;
  trWindow->height = height;
  trWindow->title = title;
  return trWindow;
}

struct tr_renderer* createTrRenderer(struct tr_window *p_window){
  struct tr_renderer* trRenderer = malloc(sizeof(struct tr_renderer));
  trRenderer->physicalDevice = VK_NULL_HANDLE;
  createInstance(&trRenderer->instance);
  createSurface(trRenderer, p_window);
  pickPhysicalDevice(trRenderer);
  createLogicalDevice(trRenderer);


  return trRenderer;
}


struct tr_application* createTrApplication(){
  struct tr_application *app = malloc(sizeof(struct tr_application)); 
  app->p_window = createTrWindow("Triangle", 800, 600);
  app->p_renderer = createTrRenderer(app->p_window);
  app->run = &run;



  return app;
}

void destroyApplication(struct tr_application *app){

  vkDestroySurfaceKHR(app->p_renderer->instance, app->p_renderer->surface, 0);
  vkDestroyInstance(app->p_renderer->instance, 0);
  vkDestroyDevice(app->p_renderer->device, 0);
  
  
  glfwDestroyWindow(app->p_window->window);
  glfwTerminate();


  free(app->p_window);
  free(app->p_renderer);
  free(app);
}

int main()
{

  initialize_logging();
  struct tr_application *app =createTrApplication();
  app->isRunning = TR_TRUE;
    while (!glfwWindowShouldClose(app->p_window->window)) {
      glfwPollEvents();
    }

  destroyApplication(app);
}