#include "logger.h"
#include <stdio.h>
#include <stdlib.h>


static char path[20];

char* hct_get_date(){
   struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  char *hct_date = malloc(sizeof(char)*10);
  strftime(hct_date, 10, "%u.%m.%Y", localtime(&ts.tv_sec));
  return hct_date;
}


char* hct_get_logTime(){
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  char *hct_date = malloc(sizeof(char)*40);
  strftime(hct_date, 40, "%F %T", localtime(&ts.tv_sec));
  return hct_date;
}


HCT_Bool initialize_logging() {
  char* hct_date = hct_get_date();
  sprintf(path, "%s/%s.%s", "log", hct_date, ".log");
  FILE *file = fopen(path, "a");
  if (file == NULL) {
    fclose(file);
    return HCT_FALSE;
  } else {
    fclose(file);
    return HCT_TRUE;
  }
  free(hct_date);
};

void write_log(const char* hct_log_time, const char *priority_message, const char *message) {  
  FILE *file = fopen(path, "r+");
  fseek(file, 0, SEEK_END);
  fprintf(file, "%s  %s %s\n", hct_log_time, priority_message, message);
  fclose(file);
}

void log_output(LogPriority priority, const char *message, ...) {
  //getting time
  char* hct_date = hct_get_logTime();


  //priority and highlight
  const char *priority_message[6] = {
      "[TRACE]:", "[DEBUG]:", "[INFO]:", "[WARN]:", "[ERROR]:", "[FATAL]:"};
  const char *text_highlith[6] = {"\x1b[36m", "\x1b[34m", "\x1b[32m",
                                  "\x1b[33m", "\x1b[31m", "\x1b[41m"};
  
  //message
  char out_message[1000];
  va_list args;
  va_start(args, message);
  vsnprintf(out_message, sizeof(out_message), message, args);
  va_end(args);
  printf("%s : %s%s\x1b[0m %s\n",hct_date, text_highlith[priority], priority_message[priority], out_message);
  
  
  //writing to log file
  write_log(hct_date, priority_message[priority], out_message);
  free(hct_date);
  
}

