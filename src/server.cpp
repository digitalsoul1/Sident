
#include "server.h"
#include "connection_handler.h"
#include "sidentd_signal.h"
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/trivial.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace boost::asio;

// https://stackoverflow.com/questions/48559892/boost-asio-1-66-0-boostasioio-service-has-no-member-get-io-service
// The get_io_service() function was deprecated in recent versions of Boost
// The code below is a workaround for this deprecation.
#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s)                                                      \
  ((boost::asio::io_context &)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif

void Server::start_accept() {
  // socket
  ConnectionHandler::pointer connection =
      ConnectionHandler::create(GET_IO_SERVICE(_acceptor), _config);
  // asynchronous accept operation and wait for a new connection.
  _acceptor.async_accept(connection->socket(),
                         boost::bind(&Server::handle_accept, this, connection,
                                     placeholders::error()));
}

Server::Server(boost::asio::io_service &io_service, SidentdConfig &&config) :
      _acceptor(basic_socket_acceptor<boost::asio::ip::tcp>(io_service)),  _config(std::move(config)) {
  ;

  // Register this server with the signal handler
  SidentdSignal::registerServer(*this);
  // Hadnle signals - closes the acceptor socket, which is not done
  // automatically in case of a signal
  SidentdSignal::handleSignals();
  // Print config settings
  BOOST_LOG_TRIVIAL(info) << "Using config settings";
  BOOST_LOG_TRIVIAL(info) << "Port: " << _config.getPort();
  BOOST_LOG_TRIVIAL(info) << "Generate random username: "
                          << _config.isGenRandomUsername();
  BOOST_LOG_TRIVIAL(info) << "Hidden users: "
                          << _config.getHiddenUsers().size();
  // Open acceptor socket
  _acceptor.open(tcp::v4());
  // Bind to port
  _acceptor.bind(tcp::endpoint(tcp::v4(), _config.getPort()));
  // Place the acceptor in a listening state
  _acceptor.listen();

  start_accept();

  BOOST_LOG_TRIVIAL(info) << "Server started on port "
                          << _acceptor.local_endpoint().port()
                          << " and ip address "
                          << _acceptor.local_endpoint().address();
  BOOST_LOG_TRIVIAL(info) << "Accepting connections...";
};

void Server::handle_accept(ConnectionHandler::pointer connection,
                           const boost::system::error_code &err) {
  if (!err) {
    connection->start();
  }
  // Accept another connection
  start_accept();
}
