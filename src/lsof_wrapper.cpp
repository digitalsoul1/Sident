#include "lsof_wrapper.h"
#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#if BOOST_VERSION >= 106400
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/system.hpp>
#else
#include <cstdio>
#include <cstdlib>
#endif

#include <boost/regex.hpp>
#include <boost/regex/v4/regex_match.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <vector>

std::string LsofWrapper::getPortOwner() const {
  return parseOutput(getOutput());
}

// Parse the output of the lsof command
// The output is a string containing the output of the lsof command
//
// Two versions of the fuction are provided,
// the latter is used specifcally for UDACITY VM or systems with older
// versions of libboost
#if BOOST_VERSION >= 106400
std::string LsofWrapper::getOutput() const {
  std::string result{};
  // Get the pid of the current process
  __pid_t thisUid = getpid();

  boost::process::ipstream pipe_stream;

  std::string command = "lsof -p^" + std::to_string(thisUid) +
                        " -P -Fun -i :" + std::to_string(_localPort);

  // The output of the command is piped to the pipe_stream
  boost::process::system(command, boost::process::std_out > pipe_stream);

  std::string line;
  while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
    result += line + "\n";
  }

  return result;
};
#else
std::string LsofWrapper::getOutput() const {
  std::string result{};
  __pid_t thisPid = getpid();
  std::array<char, 128> buffer;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::string command = "lsof -p^" + std::to_string(thisPid) +
                        " -P -Fun -i :" + std::to_string(_localPort);
  FILE *pipe(popen(command.c_str(), "r"));

  if (!pipe) {
    BOOST_LOG_TRIVIAL(debug) << "Could not open pipe for lsof command.";
    throw std::runtime_error("Could not open pipe for lsof command.");
  }

  while (!feof(pipe)) {
    if (fgets(buffer.data(), 128, pipe) != nullptr) {
      result += buffer.data();
    }
  }

  pclose(pipe);

  if (result.empty()) {
    BOOST_LOG_TRIVIAL(debug) << "No process found on local port " << _localPort;
    throw std::runtime_error("No process found on local port " +
                             std::to_string(_localPort));
  }
  return result;
};
#endif

std::string LsofWrapper::parseOutput(const std::string &output) const {
  std::string result;

  std::vector<std::string> lines;
  boost::split(lines, output, boost::is_any_of("\n"));

  // Filtering out unwanted lines:
  // lines that start with 'p' return the pid of the process
  // lines that start with 'f' return the file descriptor
  for (auto it = lines.begin(); it != lines.end(); ++it) {
    if (it->size() > 0 && (it->at(0) == 'f' || it->at(0) == 'p')) {
      lines.erase(it);
    }
  }

  std::string userId = lines[0].substr(1, lines[1].size() - 1);
  std::vector<std::string> tokens;

  // First, the line is split by the '>' character, which is always present
  // Second, the last character of the first token is removed, as it is
  // always a '-' character.
  boost::split(tokens, lines[1], boost::is_any_of(">"));
  tokens[0].erase(tokens[0].size() - 1, tokens[1].size());

  // Remove empty tokens
  for (auto it = tokens.begin(); it != tokens.end(); ++it) {
    if (it->empty()) {
      tokens.erase(it);
    }
  }

  // check if the local port and remote port match
  boost::regex expresion(".*:" + std::to_string(_localPort));
  boost::regex expresion2(".*:" + std::to_string(_remotePort));

  if (boost::regex_match(tokens[0], expresion) &&
      boost::regex_match(tokens[1], expresion2)) {
    result = userId;
  } else {
    BOOST_LOG_TRIVIAL(debug) << "No process found on local port " << _localPort
                             << " and remote port " << _remotePort;
    std::runtime_error("No process found on local port " +
                       std::to_string(_localPort) + " and remote port " +
                       std::to_string(_remotePort));
  }

  return getUsername(result);
}

std::string LsofWrapper::getUsername(const std::string &userId) const {
  std::ifstream passwd_file("/etc/passwd");
  std::string line;
  std::string user;
  std::string uid;
  std::string unused;
  std::string result;
  while (std::getline(passwd_file, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream(line) >> user >> unused >> uid;

    if (userId == uid) {
      return user;
    }
  };

  if (result.empty()) {
    BOOST_LOG_TRIVIAL(debug) << "No user found for user id " << userId;
    std::runtime_error("No user found for user id " + userId);
  }

  return result;
}