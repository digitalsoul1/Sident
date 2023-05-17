#include "sidentd_signal.h"
#include "server.h"
#include "signal.h"
#include <boost/log/trivial.hpp>
#include <cstdlib>

void *SidentdSignal::serverPtr = nullptr;

// Register server server instance
void SidentdSignal::registerServer(Server &server) {

  if (serverPtr == nullptr) {
    serverPtr = &server;
  } else {
    BOOST_LOG_TRIVIAL(debug) << "Server already registered";
  }
}
void SidentdSignal::handleSignals() {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
}

// Handle signals
void SidentdSignal::handle_signal(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    if (serverPtr != nullptr) {
      Server *server = static_cast<Server *>(serverPtr);
      BOOST_LOG_TRIVIAL(debug)
          << "Received signal: "
          << (signal == SIGINT ? "terminal interrupt signal"
                               : "terminate signal");
      if (server->_acceptor.is_open()) {
        BOOST_LOG_TRIVIAL(debug) << "Closing acceptor socket";
        server->_acceptor.close();
      }
      server = nullptr;
    }
    std::exit(0);
  }
}