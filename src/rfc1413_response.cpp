#include "rfc1413_response.h"
#include "lsof_wrapper.h"
#include "sidentd_config.h"
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <boost/regex/v4/regex_match.hpp>
#include <boost/type_traits/is_empty.hpp>
#include <iostream>
#include <string>

RFC1413Response::RFC1413Response(std::string request,
                                 const SidentdConfig &config)
    : _config(config) {
  _request = request;
};

RFC1413ResponseType RFC1413Response::processRequest() {
  boost::regex expression("^([0-9]+),([0-9]+)");
  boost::match_results<std::string::const_iterator> results;
  boost::trim(_request);
  boost::erase_all(_request, " ");

  if (boost::regex_match(_request, results, expression)) {
    try {
      _localPort = std::stoi(results[1]);
      _remotePort = std::stoi(results[2]);

      if (_localPort < 1 || _localPort > 65535 || _remotePort < 1 ||
          _remotePort > 65535) {

        // Port pair is invalid
        return RFC1413ResponseType::INVALID_PORT;
      }

      std::unique_ptr<LsofWrapper> lsof = std::unique_ptr<LsofWrapper>(
          new LsofWrapper(_localPort, _remotePort));

      _username = lsof.get()->getPortOwner();

      // No exceptions were thrown at this point, so we can produce a response
      // depending on the otions chosen by the user:
      //
      // 1. Generate random username if a user was found
      // 2. Hide the user if it is on the hidden list
      // 3. Send the username if it was found
      //
      if (_config.isGenRandomUsername() == true) {
        _username = generateRandomUser();
      }

      // check if user is on the hidden list
      if (isUserHidden() == true) {
        return RFC1413ResponseType::HIDDEN_USER;
      }
      //  Workaround for UDACITY VM
      //  On stadard linux systems we would hide the root user
      //     if (_username == "root") {
      //       return RFC1413ResponseType::HIDDEN_USER;
      //     }
      return RFC1413ResponseType::OK;
    } catch (std::exception &e) {
      // Port pair are not owned by a local user
      return RFC1413ResponseType::NO_USER;
    }
  } else {
    // Could not parse request
    return RFC1413ResponseType::UNKNOWN_ERROR;
  }
}

std::string RFC1413Response::portPairString() const {
  std::string portPair =
      std::to_string(_localPort) + " , " + std::to_string(_remotePort);
  return portPair;
}

std::string RFC1413Response::userIdString() const { return "USERID"; };

std::string RFC1413Response::osTypeString() const { return "UNIX"; }

std::string RFC1413Response::hiddenUserString() const { return "HIDDEN-USER"; }

std::string RFC1413Response::noUserString() const { return "NO-USER"; }

std::string RFC1413Response::errorString() const { return "ERROR"; }

std::string RFC1413Response::unknownErrorString() const {
  return "UNKNOWN-ERROR";
}

std::string RFC1413Response::invalidPortString() const {
  return "INVALID-PORT";
}

std::string RFC1413Response::responseTermintatorString() const {
  // RFC 1413 requires that the response be terminated with a CRLF
  return "\r\n";
}

std::string RFC1413Response::getResponseString(bool hideRequest) {
  std::string responseString;

  RFC1413ResponseType response = processRequest();
  std::string portPair = hideRequest == true ? " " : portPairString();

  switch (response) {
  case RFC1413ResponseType::OK:
    responseString = portPair + " : " + userIdString() + " : " +
                     osTypeString() + " : " + _username +
                     responseTermintatorString();
    break;
  case RFC1413ResponseType::HIDDEN_USER:
    responseString = portPair + " : " + errorString() + " : " +
                     hiddenUserString() + responseTermintatorString();
    break;
  case RFC1413ResponseType::INVALID_PORT:
    responseString = portPair + " : " + errorString() + " : " +
                     invalidPortString() + responseTermintatorString();
    break;
  case RFC1413ResponseType::NO_USER:
    responseString = portPair + " : " + errorString() + " : " + noUserString() +
                     responseTermintatorString();
    break;
  case RFC1413ResponseType::UNKNOWN_ERROR:
    responseString = portPair + " : " + errorString() + " : " +
                     unknownErrorString() + responseTermintatorString();
    break;
  }

  return responseString;
}

std::string RFC1413Response::generateRandomUser() {
  std::string randomUser = "user" + std::to_string(rand() % 1000);
  return randomUser;
}

bool RFC1413Response::isUserHidden() const {
  bool isHidden = false;
  for (auto &user : _config.getHiddenUsers()) {
    if (user == _username) {
      isHidden = true;
      break;
    }
  }
  return isHidden;
}