#include "sidentd_config.h"

void SidentdConfig::setGenRandomUsername(bool genRandomUsername) {
  _genRandomUsername = genRandomUsername;
}

void SidentdConfig::setHiddenUsers(std::vector<std::string> &&hiddenUsers) {
  _hiddenUsers = hiddenUsers;
}

void SidentdConfig::setPort(unsigned short port) { _port = port; }

bool SidentdConfig::isGenRandomUsername() const { return _genRandomUsername; }

std::vector<std::string> SidentdConfig::getHiddenUsers() const {
  return _hiddenUsers;
}

unsigned short SidentdConfig::getPort() { return _port; }