#ifndef _IMG_ANALYSIS_H_
#define _IMG_ANALYSIS_H_

#include <freertos/task.h>
#include "color.h"

TaskHandle_t img_analysis_taskhandle;

void img_analysis_task(void* pvParameters);
void img_analysis_init();
#endif
