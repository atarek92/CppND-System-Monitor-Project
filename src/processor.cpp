#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() { 
    float uti;
    long active = LinuxParser::ActiveJiffies();
    long idle = LinuxParser::IdleJiffies();
    if (active + idle > 0.0f)
        uti = (float)(active / (float)(active + idle));
    else
        uti =  0.0f;

    return uti; }