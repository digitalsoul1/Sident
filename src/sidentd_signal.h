#ifndef SIDENTD_SIGNAL_H
#define SIDENTD_SIGNAL_H

// Forward declaration
class Server;

class SidentdSignal {
private:
  // Signal handler function
  static void handle_signal(int signal);
  // Pointer to Server instance
  static void *serverPtr;

  // Disable copy and move constructors and assignment operators
  SidentdSignal() = delete;
  SidentdSignal(const SidentdSignal &) = delete;
  SidentdSignal &operator=(const SidentdSignal &) = delete;
  SidentdSignal(SidentdSignal &&) = delete;
  SidentdSignal &operator=(SidentdSignal &&) = delete;

public:
  // Register server server instance
  static void registerServer(Server &server);
  // Handle signals
  static void handleSignals();
};

#endif