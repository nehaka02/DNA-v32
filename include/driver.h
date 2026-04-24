#ifndef DRIVER_H
#define DRIVER_H

#include "pipeline.h"

// This tells any file including driver.h that this function exists elsewhere
void single_clock_cycle(Pipeline* pipeline, bool cacheEnabled, bool pipelineEnabled);

#endif
