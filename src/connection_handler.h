#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "sidentd_config.h"
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>

using boost::asio::ip::tcp;

class ConnectionHandler
    : public boost::enable_shared_from_this<ConnectionHandler> {
public:
  ConnectionHandler(boost::asio::io_service &io_service,
                    const SidentdConfig &config);

  typedef boost::shared_ptr<ConnectionHandler> pointer;
  static pointer create(boost::asio::io_service &io_service,
                        const SidentdConfig &config);

  void start();

  tcp::socket &socket();
  ~ConnectionHandler();

private:
  // Sends output to the client
  void handle_write(const boost::system::error_code &err,
                    size_t bytes_transferred);
  void handle_read(const boost::system::error_code &err,
                   size_t bytes_transferred);
  void setTempResponse(std::string response);
  void getTempResponse(std::string &response);
  void clearTempResponse();
  tcp::socket _socket;
  std::string _tempResponse;
  char _buffer[1024];
  unsigned long _localPort;
  unsigned long _remotePoart;
  const SidentdConfig &_config;
  boost::mutex _mutex;
};
#endif
