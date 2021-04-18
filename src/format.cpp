#include <string>

#include "format.h"

using std::string;
#define SEC_IN_MIN  (60)
#define MIN_IN_HR   (60)
#define SEC_IN_HR (SEC_IN_MIN * MIN_IN_HR)

string Format::ElapsedTime(long seconds) { 
     long hours, minutes;
    hours = seconds/SEC_IN_HR;
    seconds = seconds%SEC_IN_HR;
    minutes = seconds/SEC_IN_MIN;
    seconds = seconds%SEC_IN_MIN;

    std::string hours_str = std::to_string(hours);
    if (hours_str.length() < 2) {
         hours_str.insert(0, 2 - hours_str.length(), '0');
    }
    
    std::string min_str = std::to_string(minutes);
    min_str.insert(0, 2 - min_str.length(), '0');
    
    std::string sec_str = std::to_string(seconds);
    sec_str.insert(0, 2 - sec_str.length(), '0');

    return (hours_str + ":" + min_str+ ":" +sec_str);
    }