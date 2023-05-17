#ifndef SS_WRAPPER_H
#define SS_WRAPPER_H

#include <string>

class LsofWrapper {
public:
  // Constructor
  LsofWrapper(unsigned int localPort, unsigned int remotePort)
      : _localPort(localPort), _remotePort(remotePort){};
  // Getters
  std::string getPortOwner() const;

private:
  // Private methods
  std::string parseOutput(const std::string &output) const;
  std::string getOutput() const;
  std::string getUsername(const std::string &userId) const;
  unsigned int _localPort;
  unsigned int _remotePort;
};

#endif