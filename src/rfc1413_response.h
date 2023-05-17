#ifndef RFC1413_RESPONSE_H
#define RFC1413_RESPONSE_H

#include "sidentd_config.h"
#include <memory>
#include <string>

enum class RFC1413ResponseType {
  OK,
  HIDDEN_USER,
  INVALID_PORT,
  NO_USER,
  UNKNOWN_ERROR
};

class RFC1413Response {
public:
  RFC1413Response() = delete;
  RFC1413Response(std::string request, const SidentdConfig &config);
  std::string getResponseString(bool hideRequest = false);

private:
  // RFC1413 response strings elements
  // Used to generate the response string
  RFC1413ResponseType processRequest();
  std::string generateRandomUser();
  std::string portPairString() const;
  std::string portPairStringHidden(bool &hide);
  std::string osTypeString() const;
  std::string errorString() const;
  std::string hiddenUserString() const;
  std::string unknownErrorString() const;
  std::string invalidPortString() const;
  std::string responseTermintatorString() const;
  std::string noUserString() const;
  std::string userIdString() const;

  // Helper functions
  bool isUserHidden() const;

  std::string _request;
  std::string _response;
  std::string _username;
  unsigned int _localPort;
  unsigned int _remotePort;
  const SidentdConfig &_config;
};
#endif