#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds)
{
  long HH, MM, SS;
  SS = seconds % 60;
  MM = (seconds / 60) % 60;
  HH = seconds / 3600;
        
  std::stringstream HHMMSSstream;
  
  HHMMSSstream << std::setw(2) << std::setfill('0') << HH 
        << ":" << std::setw(2) << std::setfill('0') << MM
        << ":" << std::setw(2) << std::setfill('0') << SS;
        
  return HHMMSSstream.str();
}