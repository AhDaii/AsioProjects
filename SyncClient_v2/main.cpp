#include <iostream>
#include <boost/asio.hpp>
#include <thread>

using namespace std;
using namespace boost::asio::ip;
using namespace chrono_literals;
const int MAX_LENGTH = 1024*2;
const int HEAD_LENGTH = 2;

int main() {
    try {
        boost::asio::io_context ioc;
        tcp::endpoint ep(address::from_string("127.0.0.1"), 10086);
        tcp::socket socket(ioc);
        boost::system::error_code ec = boost::asio::error::host_not_found;
        socket.connect(ep, ec);
        if (ec) {
            cout << "connect failed, code is: " << ec.value() << ", error msg is: " << ec.message();
            return 0;
        }

        thread send_thread([&socket] {
            while (true) {
                this_thread::sleep_for(2ms);
                const char* request = "Hello World!";
                short request_len = strlen(request);
                char send_data[MAX_LENGTH] = {0};
                short request_len_host = boost::asio::detail::socket_ops::host_to_network_short(request_len);
                memcpy(send_data, &request_len_host, HEAD_LENGTH);
                memcpy(send_data + HEAD_LENGTH, request, request_len);
                boost::asio::write(socket, boost::asio::buffer(send_data, request_len + HEAD_LENGTH));
            }
        });

        thread recv_thread([&socket] {
            while (true) {
                this_thread::sleep_for(2ms);
                cout << "Begin to receive..." << endl;
                char reply_head[HEAD_LENGTH];
                boost::asio::read(socket, boost::asio::buffer(reply_head, HEAD_LENGTH));
                short msg_len = 0;
                memcpy(&msg_len, reply_head, HEAD_LENGTH);
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                char reply_msg[MAX_LENGTH] = {0};
                boost::asio::read(socket, boost::asio::buffer(reply_msg, msg_len));
                cout << "Reply is: ";
                cout.write(reply_msg, msg_len) << endl;
                cout << "Reply length is: " << msg_len << endl;
            }
        });

        send_thread.join();
        recv_thread.join();
    } catch (std::exception& e) {
        cout << "Exception:" << e.what() << endl;
    }
    return 0;
}
