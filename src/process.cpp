#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;


Process::Process(int pid_in) : pid(pid_in) {}

// DONE: Return this process's ID
int Process::Pid() {
  return pid;
}

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() const {
  return (float)LinuxParser::ActiveJiffies(pid) / (float) LinuxParser::Jiffies();
}

// DONE: Return the command that generated this process
string Process::Command() { 
  return LinuxParser::Command(pid);
}

// DONE: Return this process's memory utilization
string Process::Ram() {
  return LinuxParser::Ram(pid);
}

// DONE: Return the user (name) that generated this process
string Process::User() {
  return LinuxParser::User(pid);
}

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() {
  return LinuxParser::UpTime(pid);
}

// DONE: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const { 
  return a.CpuUtilization() < CpuUtilization();
}