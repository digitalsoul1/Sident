#ifndef SERVER_H
#define SERVER_H

#include "connection_handler.h"
#include "sidentd_config.h"
#include "sidentd_signal.h"
#include <boost/asio/signal_set.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <memory>

using namespace boost::asio;
using ip::tcp;

class Server {
private:
  tcp::acceptor _acceptor;
  void start_accept();
  SidentdConfig _config;
  friend class SidentdSignal;

public:
  Server() = delete;
  void handle_accept(ConnectionHandler::pointer connection,
                     const boost::system::error_code &err);
  Server(boost::asio::io_service &io_service, SidentdConfig &&config);
  ;
};

#endif