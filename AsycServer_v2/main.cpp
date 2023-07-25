#include "CServer.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  try {
    boost::asio::io_context ioc;
    using namespace std;
    CServer s(ioc, 10086);
    ioc.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
