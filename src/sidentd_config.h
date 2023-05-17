#ifndef SIDENTD_CONFIG_H
#define SIDENTD_CONFIG_H

#include <string>
#include <vector>

// At this point, the server supports only 3 config options
// 1. Port number to listen on
// 2. Generate random username if a user was found
// 3. Hidden users - users that appear hidden to the client
class SidentdConfig {
public:
  SidentdConfig(unsigned short port = 113, bool genRandomUsername = false,
                std::vector<std::string> hiddenUsers = {})
      : _port(port), _genRandomUsername(genRandomUsername),
        _hiddenUsers(hiddenUsers){};

  // Setters
  void setPort(unsigned short port);
  void setGenRandomUsername(bool genRandomUsername);
  void setHiddenUsers(std::vector<std::string> &&hiddenUsers);

  // Getters
  unsigned short getPort();
  bool isGenRandomUsername() const;
  std::vector<std::string> getHiddenUsers() const;

private:
  // Config options
  unsigned short _port;
  bool _genRandomUsername;
  std::vector<std::string> _hiddenUsers;
};

#endif