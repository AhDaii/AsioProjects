#include "boost/asio/detail/socket_ops.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <thread>

using namespace std;
using namespace boost::asio::ip;
using namespace chrono_literals;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_ID_LENGTH = 2;
const int HEAD_DATA_LENGTH = 2;
const int HEAD_TOTAL_LENGTH = 4;

int main() {
  try {
    boost::asio::io_context ioc;
    tcp::endpoint ep(address::from_string("127.0.0.1"), 10086);
    tcp::socket socket(ioc);
    boost::system::error_code ec = boost::asio::error::host_not_found;
    socket.connect(ep, ec);
    if (ec) {
      cout << "connect failed, code is: " << ec.value()
           << ", error msg is: " << ec.message();
      return 0;
    }

    thread send_thread([&socket] {
      while (true) {
        short id = 1001;
        this_thread::sleep_for(2ms);
        Json::Value root;
        root["id"] = 1001;
        root["data"] = "Hello World!";

        string request = root.toStyledString();
        short request_len = request.length();
        char send_data[MAX_LENGTH] = {0};
        short request_len_host =
            boost::asio::detail::socket_ops::host_to_network_short(request_len);
        short id_host =
            boost::asio::detail::socket_ops::host_to_network_short(id);
        memcpy(send_data, &id_host, HEAD_ID_LENGTH);
        memcpy(send_data + HEAD_ID_LENGTH, &request_len_host, HEAD_DATA_LENGTH);
        memcpy(send_data + HEAD_TOTAL_LENGTH, request.c_str(), request_len);
        boost::asio::write(
            socket,
            boost::asio::buffer(send_data, request_len + HEAD_TOTAL_LENGTH));
      }
    });

    thread recv_thread([&socket] {
      while (true) {
        this_thread::sleep_for(2ms);
        cout << "Begin to receive..." << endl;
        char reply_head[HEAD_TOTAL_LENGTH];
        boost::asio::read(socket,
                          boost::asio::buffer(reply_head, HEAD_TOTAL_LENGTH));
        short msg_id, data_len;
        memcpy(&msg_id, reply_head, HEAD_ID_LENGTH);
        memcpy(&data_len, reply_head + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);
        msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
        data_len =
            boost::asio::detail::socket_ops::network_to_host_short(data_len);
        char reply_msg[MAX_LENGTH] = {0};
        boost::asio::read(socket, boost::asio::buffer(reply_msg, data_len));
        cout << "Reply is: ";
        cout.write(reply_msg, data_len) << endl;
        cout << "Reply length is: " << data_len << ", msg id is: " << msg_id
             << endl;
      }
    });

    send_thread.join();
    recv_thread.join();
  } catch (std::exception &e) {
    cout << "Exception:" << e.what() << endl;
  }
  return 0;
}
