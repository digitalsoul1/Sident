#include "connection_handler.h"
#include "lsof_wrapper.h"
#include "server.h"
#include "sidentd_config.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/regex/v4/regex_match.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#define VERSION "0.1.0"
#define NAME "sidentd"

namespace opt = boost::program_options;

int main(int argc, char *argv[]) {
  std::cout << NAME << " " << VERSION << " - simple and free ident server\n";
  opt::options_description desc("Usage: " + std::string(argv[0]) +
                                " [options]");

  SidentdConfig config;
  desc.add_options()
      // Start the server in the foreground
      ("start,s", "start the server")
      // Set custom port
      ("port,p", opt::value<unsigned short>(),
       "sets port to listen on, default is 113")
      // Send randomized user names to clients
      ("randomusers,r", opt::value<bool>(),
       "if a user if found send a random username instead [true, false]")(
          "hideusers,h", opt::value<std::string>(),
          "colon separated list of users to hide, default is none, for example "
          "root:docker:admin");

  opt::variables_map vm;
  try {
    opt::store(opt::parse_command_line(argc, argv, desc), vm);
    opt::notify(vm);
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  if (vm.count("help") || vm.empty()) {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("start")) {
    if (vm.count("port")) {
      auto port = vm["port"].as<unsigned short>();
      if (port < 1 || port > 65535) {
        BOOST_LOG_TRIVIAL(error) << "Invalid port number: " << port;
        return 1;
      } else {
        BOOST_LOG_TRIVIAL(info) << "User set the port to: " << port;
        config.setPort(port);
      }
    } else
      config.setPort(113);

    if (vm.count("randomusers") && vm.count("hideusers")) {
      BOOST_LOG_TRIVIAL(error)
          << "You can't use both randomusers and hideusers at the same time";
      return 1;
    }

    if (vm.count("randomusers")) {
      auto randomusers = vm["randomusers"].as<bool>();
      BOOST_LOG_TRIVIAL(info)
          << "User set the randomusers flag to: " << randomusers;
      config.setGenRandomUsername(randomusers);
    } else
      config.setGenRandomUsername(false);

    if (vm.count("hideusers")) {
      std::vector<std::string> hiddenusers;

      std::string hideusers =
          boost::any_cast<std::string>(vm["hideusers"].as<std::string>());

      boost::split(hiddenusers, hideusers, boost::is_any_of(":"));

      // https://unix.stackexchange.com/questions/157426/what-is-the-regex-to-validate-linux-users
      boost::regex expression("^[a-z_]([a-z0-9_-]{0,31}|[a-z0-9_-]{0,30}\\$)$");

      for (auto &user : hiddenusers) {
        if (!boost::regex_match(user, expression)) {
          BOOST_LOG_TRIVIAL(error) << "Invalid username to be hidden: " << user;
          return 1;
        } else {
          BOOST_LOG_TRIVIAL(info) << "User set the hidden user: " << user;
        }

        config.setHiddenUsers(std::move(hiddenusers));
      }

    } else
      config.setHiddenUsers({});
  }

  try {
    boost::asio::io_service io_service;
    Server server(io_service, std::move(config));
    io_service.run();

  } catch (std::exception &e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }
  return 0;
}