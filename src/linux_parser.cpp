#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization()
{
  // total used memory: MemTotal (first row) - MemFree (second row)
  const std::string kMemTotal{"MemTotal"};
  const std::string kMemFree{"MemFree"};
  string line;
  string key;
  float value;
  float total_value = 0;
  float free_value = 0;
  int necessary_keywords_found = 0;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && necessary_keywords_found < 2) {
      // Line structure:
      // MemTotal:      1337 kB
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == kMemTotal) {
        total_value = value;
        ++necessary_keywords_found;
      }
      else if (key == kMemFree){
        free_value = value;
        ++necessary_keywords_found;
      }
    }
    assert (necessary_keywords_found == 2);
    return (total_value - free_value)/total_value;
  }
  
  return 0.0;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long uptime = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    if (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> uptime;
    }
  }
  return uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
 long sum = 0;
 for (auto elem : CpuUtilization()){
   sum += std::stol(elem);
 }
 return sum;
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  constexpr int kFirstRelevantIndex = 13;
  constexpr int kNrOfRelevantValues = 4;
  string statfile = kProcDirectory + to_string(pid) + kStatFilename;
  string line;
  string buffer;
  long sum = 0;
  long jiffy_elem;
  
  
  std::ifstream filestream(statfile);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      
      std::istringstream linestream(line);
      
      // remove irrelevant values from string stream
      for(int i = 0; i<kFirstRelevantIndex; ++i) linestream >> buffer;
      
      // add up relevant values
      for(int i = 0; i<kNrOfRelevantValues; ++i)
      {
        linestream >> jiffy_elem;
        sum += jiffy_elem;
      }
    }
  }
  return sum;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {  
  return Jiffies() - IdleJiffies();
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  const int idle_index = 3;
  return std::stol(CpuUtilization()[idle_index]);
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  // one needs to read out the third numeric value the line starting with cpu in /proc/stat (should be the first line)
  const std::string kCpu{"cpu"};
  const int expected_values = 10;
  string line;
  string line_element;
  vector<string> utilization;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    if (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> line_element;
      assert(line_element == kCpu);
      while(linestream >> line_element)
      {
        utilization.push_back(line_element);
      }
    }
  }
  assert(utilization.size() == expected_values);
  return utilization;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  // one needs to read out the value the line starting with processes in /proc/stat
  const std::string kProcesses{"processes"};
  string line;
  string line_element;
  int nrOfProcesses = 0;
  
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> line_element;
      if(line_element == kProcesses){
        linestream >> nrOfProcesses;
        break;
      }
    }
  }
  return nrOfProcesses;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  // one needs to read out the value the line starting with procs_running in /proc/stat
  const std::string kProcesses{"procs_running"};
  string line;
  string line_element;
  int nrOfProcesses = 0;
  
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> line_element;
      if(line_element == kProcesses){
        linestream >> nrOfProcesses;
        break;
      }
    }
  }
  return nrOfProcesses;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string filename = kProcDirectory + to_string(pid) + kCmdlineFilename;
  string line("");
  std::ifstream filestream(filename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  // one needs to read out the value the line starting with VmSize in /proc/[pid]/stat
  const std::string kVmSize{"VmSize"};
  string filename = kProcDirectory + to_string(pid) + kStatusFilename;
  string line;
  string buffer;
  long size = 0;
  
  std::ifstream filestream(filename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      
      linestream >> buffer;
      
      if (buffer == kVmSize) {
        linestream >> size;
        linestream >> buffer;
        // function relies on reported value to be in kB
        assert(buffer == "kB");
        break;
      }
    }
  }
  // size conversion from kB to mB
  return to_string(size / 1024);
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
// one needs to read out the value the line starting with VmSize in /proc/[pid]/stat
  const std::string kUid{"Uid"};
  string filename = kProcDirectory + to_string(pid) + kStatusFilename;
  string line;
  string buffer;
  string uid("");
  
  std::ifstream filestream(filename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      
      linestream >> buffer;
      
      if (buffer == kUid) {
        linestream >> uid;
        break;
      }
    }
  }
  return uid;
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  
  string uid = Uid(pid);
  string line;
  string user("");
  string x;
  string uid_found;
      
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      
      // each line has following structure (and we have just replaced the ':' with spaces):
      // root:x:0:0:root:/root:/bin/bash      
      linestream >> user >> x >> uid_found;
      
      if (uid_found == uid) break;
    }
  }
  return user;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  constexpr int kRelevantIndex = 21;
  string statfile = kProcDirectory + to_string(pid) + kStatFilename;
  string line;
  string buffer;
  long starttime = 0;
  
  std::ifstream filestream(statfile);
  if (filestream.is_open()) {
    if (std::getline(filestream, line)) {
      
      std::istringstream linestream(line);
      
      // remove irrelevant values from string stream
      for(int i = 0; i<kRelevantIndex; ++i) linestream >> buffer;
      
      linestream >> starttime;
    }
  }
  // process uptime = system uptime - start time
  return UpTime()-(starttime/sysconf(_SC_CLK_TCK));
}
