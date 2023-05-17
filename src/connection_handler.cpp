#include "connection_handler.h"
#include "rfc1413_response.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>
#include <boost/regex/v4/regex_match.hpp>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

using namespace boost::asio;
using ip::tcp;

ConnectionHandler::ConnectionHandler(boost::asio::io_service &io_service,
                                     const SidentdConfig &config)
    : _socket(io_service), _config(config) {
  // Init buffer
  memset(_buffer, 0, 1024);
};

ConnectionHandler::pointer
ConnectionHandler::create(boost::asio::io_service &io_service,
                          const SidentdConfig &config) {
  return pointer(new ConnectionHandler(io_service, config));
}

tcp::socket &ConnectionHandler::socket() { return _socket; }

void ConnectionHandler::start() {
  _socket.async_read_some(
      boost::asio::buffer(_buffer, 1024),
      boost::bind(&ConnectionHandler::handle_read, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void ConnectionHandler::handle_read(const boost::system::error_code &err,
                                    size_t bytes_transferred) {
  if (!err) {
    BOOST_LOG_TRIVIAL(info)
        << "Processing request from: " +
               _socket.remote_endpoint().address().to_string() + ":" +
               std::to_string(_socket.remote_endpoint().port());
    std::unique_ptr<RFC1413Response> rfc1413response =
        std::unique_ptr<RFC1413Response>(new RFC1413Response(_buffer, _config));
    std::string response = rfc1413response->getResponseString();
    _tempResponse = response;
    ;
    _socket.async_write_some(
        boost::asio::buffer(response, 1024),
        boost::bind(&ConnectionHandler::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    BOOST_LOG_TRIVIAL(error) << err.message();
    _socket.close();
  }
}

void ConnectionHandler::handle_write(const boost::system::error_code &err,
                                     size_t bytes_transferred) {
  if (!err) {
    BOOST_LOG_TRIVIAL(info)
        << "Finished processing request from: " +
               _socket.remote_endpoint().address().to_string() + ":" +
               std::to_string(_socket.remote_endpoint().port());
    BOOST_LOG_TRIVIAL(debug) << "Response sent: " << _tempResponse;
  } else {
    BOOST_LOG_TRIVIAL(error) << err.message();
    _socket.close();
  }

  // Get rid of the old data in the buffer
  memset(_buffer, 0, 1024);
  clearTempResponse();
}

ConnectionHandler::~ConnectionHandler() {
  if (_socket.is_open()) {
    BOOST_LOG_TRIVIAL(debug) << "Closing socket " << &_socket;
    _socket.close();
  }
}

void ConnectionHandler::setTempResponse(std::string response) {
  boost::mutex::scoped_lock lock(_mutex);
  _tempResponse = response;
}

void ConnectionHandler::getTempResponse(std::string &response) {
  boost::mutex::scoped_lock lock(_mutex);
  response = _tempResponse;
}

void ConnectionHandler::clearTempResponse() {
  boost::mutex::scoped_lock lock(_mutex);
  _tempResponse = "";
}