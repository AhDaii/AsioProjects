#include <iostream>
#include <set>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

const int MAX_LENGTH = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr;
set<shared_ptr<std::thread>> thread_set;


void session(const socket_ptr& sock) {
    try {
        while (true) {
            char data[MAX_LENGTH];
            memset(data, 0, MAX_LENGTH);
            boost::system::error_code ec;
            // size_t length = boost::asio::read(sock, boost::asio::buffer(data, MAX_LENGTH), ec);
            size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), ec);
            if (ec == boost::asio::error::eof) {
                cout << "Connect closed by peer" << endl;
                break;
            } else if (ec) {
                throw boost::system::system_error(ec);
            }
            cout << "Receive from " << sock->remote_endpoint().address().to_string() << endl;
            cout << "Receive message is " << data << endl;
            // 回传
            boost::asio::write(*sock, boost::asio::buffer(data, length));
        }
    } catch (exception &e) {
        cerr << "Exception in thread: " << e.what() << endl;
    }
}

[[noreturn]] void server(boost::asio::io_context& ioc, unsigned short port) {
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));
    while (true) {
        socket_ptr sock(new tcp::socket(ioc));
        acceptor.accept(*sock);
        auto t = std::make_shared<std::thread>(session, sock);
        // 放在集合里保证不会被立马释放
        thread_set.insert(t);
    }
}

int main() {
    try {
        boost::asio::io_context ioc;
        cout << "Server started" << endl;
        server(ioc, 10086);
        for (auto& t : thread_set)
            t->join();
    } catch (std::exception& e){

    }
    return 0;
}
