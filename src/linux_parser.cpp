#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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

float LinuxParser::MemoryUtilization() {
  float total_used_mem, mem_utilization;
  string ignore, mem_total, mem_free, mem_available;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> ignore >>mem_total;

    std::getline(stream, line);
    std::istringstream linestream2(line);
    linestream2 >> ignore >>mem_free;

    std::getline(stream, line);
    std::istringstream linestream3(line);
    linestream3 >> ignore >>mem_available;

    total_used_mem = std::stof(mem_total) - std::stof(mem_free);
    mem_utilization = total_used_mem / std::stof(mem_total);
  }
  return mem_utilization;
}

long LinuxParser::UpTime() { 
  string uptime, line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return std::stol(uptime);
}

long LinuxParser::Jiffies() { 
  long jiffies = sysconf(_SC_CLK_TCK) * UpTime();
  return jiffies; }

long LinuxParser::ActiveJiffies() { 
  long sum = 0;
  auto cpu_values = LinuxParser::CpuUtilization();
  for (const auto &i:cpu_values)  { sum += std::stol(i); }
  return sum; }

long LinuxParser::IdleJiffies() { 
  auto cpu_values = LinuxParser::CpuUtilization();
  long sum = std::stol(cpu_values.at(3)) + std::stol(cpu_values.at(4));
  return sum; }

vector<string> LinuxParser::CpuUtilization() { 
  string line, value;
  std::vector<string> cpuValues;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) 
  {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) 
    {
      if (value != "cpu") {
        cpuValues.push_back(value);
      }
    }
  }
  return cpuValues; 
  }

int LinuxParser::TotalProcesses() { 
  string name, total_processes;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    do
    {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> name;
    } while (name != "processes");
    
    std::istringstream linestream_final(line);
    linestream_final >> name >> total_processes;

  }
  return std::stoi(total_processes);
}

int LinuxParser::RunningProcesses() { 
  string name, running_processes;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    do
    {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> name;
    } while (name != "procs_running");
    
    std::istringstream linestream_final(line);
    linestream_final >> name >> running_processes;
  }
  return std::stoi(running_processes);
}

float LinuxParser::CpuUtilization(int pid) { 
  string line, value;
  std::vector<string> cpuValues;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);

  if (stream.is_open()) 
  {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while ((linestream >> value) && cpuValues.size() < 22) 
    {
        cpuValues.push_back(value);
    }
  }

  int utime = std::stol(cpuValues.at(13)); 
  int stime = std::stol(cpuValues.at(14)); 
  int cutime = std::stol(cpuValues.at(15)); 
  int cstime = std::stol(cpuValues.at(16)); 
  int starttime = std::stol(cpuValues.at(21)); 
  float hertz = sysconf(_SC_CLK_TCK);

  int total_time = utime + stime + cutime + cstime;
  float seconds = UpTime() - (starttime / hertz);
  float cpu_usage = (total_time / hertz) / seconds;

  return cpu_usage;  
}

string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(stream.is_open()) {
    std::getline(stream, line);
  }
  return line; 
  }

string LinuxParser::Ram(int pid) { 
 
  string name;
  string line;
  long ram;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
        while (std::getline(stream, line)) 
        {
          std::replace(line.begin(), line.end(), ':', ' ');
          std::istringstream linestream(line);
          
          while (linestream >> name >> ram) 
          {
            if (name == "VmSize") {
            return std::to_string(ram/1000);
            break;
          }
        }
      }
   }
  return "";
}

string LinuxParser::Uid(int pid) { 
  string name, uid;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    do
    {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> name;
    } while (name != "Uid:");
    
    std::istringstream linestream_final(line);
    linestream_final >> name >> uid;
  }
  return uid; 
  }

string LinuxParser::User(int pid) { 
  string uid_requested = Uid(pid);
  string name, x, uid;
  string line;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    do
    {
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> x >> uid;
    } while (uid != uid_requested);
    
    std::istringstream linestream_final(line);
    linestream_final >> name >> x >> uid;
  }
  return name; 
  }

long LinuxParser::UpTime(int pid) { 
  string line, value;
  std::vector<string> cpuValues;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);

  if (stream.is_open()) 
  {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while ((linestream >> value) && cpuValues.size() < 23) 
    {
        cpuValues.push_back(value);
    }
  }
  int starttime = std::stol(cpuValues.at(21)); 
  long hertz = sysconf(_SC_CLK_TCK);
  long uptime = (starttime / hertz);
  return uptime;
}
