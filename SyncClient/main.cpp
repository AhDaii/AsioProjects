#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio::ip;
using namespace std;
const int MAX_LENGTH = 1024;

int main() {
    try {
        // 创建上下文
        boost::asio::io_context ioc;
        // 创建endpoint
        tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
        tcp::socket sock(ioc);
        boost::system::error_code ec = boost::asio::error::host_not_found;
        sock.connect(remote_ep, ec);
        if (ec) {
            cerr << "Connect failed, code is " << ec.value()
                 << "error msg is " << ec.message() << endl;
            return -1;
        }

        cout << "Enter message:";
        char request[MAX_LENGTH];
        cin.getline(request, MAX_LENGTH);
        cout << "Done";
        size_t request_length = strlen(request);
        boost::asio::write(sock, boost::asio::buffer(request, request_length));

        char reply[MAX_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, request_length));
        cout << "Reply is:";
        cout.write(reply, static_cast<streamsize>(reply_length));
        cout << endl;

    } catch (boost::system::system_error &e) {
        cerr << "Exception:" << e.what() << endl;
    }
    return 0;
}
