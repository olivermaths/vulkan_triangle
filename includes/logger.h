#ifndef _HCT_LOG_
#define _HCT_LOG_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

typedef enum HCT_Bool {
  HCT_FALSE,
  HCT_TRUE,

} HCT_Bool;

#if HCTRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum LogPriority {
  TracePriority,
  DebugPriority,
  InfoPriority,
  WarnPriority,
  ErrorPriority,
  CriticalPriority,
} LogPriority;

HCT_Bool initialize_logging();

void log_output(LogPriority priority, const char *message, ...);

#define HCT_FATAL(message, arg...) log_output(CriticalPriority, message, arg)
#define HCT_ERROR(message, arg...) log_output(ErrorPriority, message, arg)
#define HCT_WARN(message, arg...) log_output(WarnPriority, message, arg)
#define HCT_INFO(message, arg...) log_output(InfoPriority, message, arg)

#if LOG_DEBUG_ENABLED == 1
#define HCT_DEBUG(message, arg...) log_output(DebugPriority, message, arg)
#else
#define HCT_DEBUG(message, arg...)
#endif

#if LOG_TRACE_ENABLED == 1
#define HCT_TRACE(message, arg...) log_output(TracePriority, message, arg)
#else
#define HCT_TRACE(message, arg...)
#endif
#endif