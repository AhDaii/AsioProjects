#include "boost/asio/ip/address.hpp"
#include "boost/beast/core/ostream.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/read.hpp"
#include "boost/beast/http/write.hpp"
#include "boost/core/ignore_unused.hpp"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = beast::net;

using tcp = boost::asio::ip::tcp;

namespace my_program_state
{
    std::size_t request_count() {
        static size_t cnt = 0;
        return ++cnt;
    }

    std::time_t now() {
        return std::time(nullptr);
    }
}  // namespace my_program_state

class http_connection : public std::enable_shared_from_this<http_connection> {
private:
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
    net::steady_timer _deadline{_socket.get_executor(), std::chrono::seconds(60)};

    void read_request() {
        auto self = shared_from_this();

        http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (!ec)
                self->process_request();
        });
    }

    void process_request() {
        _response.version(_request.version());
        _response.keep_alive(false);

        switch (_request.method()) {
            case http::verb::get:
                _response.result(http::status::ok);
                _response.set(http::field::server, "Beast");
                create_response();
                break;

            case http::verb::post:
                _response.result(http::status::ok);
                _response.set(http::field::server, "Beast");
                create_post_response();
                break;

            default:
                _response.result(http::status::bad_request);
                _response.set(http::field::content_type, "text/plain");
                beast::ostream(_response.body()) << "Invalid request-method '"
                                                 << std::string(_request.method_string())
                                                 << "'";
                break;
        }
        write_response();
    }

    void create_response() {
        if (_request.target() == "/count") {
            _response.set(http::field::content_type, "text/html");
            beast::ostream(_response.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << my_program_state::request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (_request.target() == "/time") {
            _response.set(http::field::content_type, "text/html");
            beast::ostream(_response.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << my_program_state::now()
                << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "File not found\r\n";
        }
    }

    void create_post_response() {
        if (_request.target() == "/email") {
            auto& body = _request.body();
            auto body_str = boost::beast::buffers_to_string(body.data());
            std::cout << "receive body is " << body_str << std::endl;
            _response.set(http::field::content_type, "text/json");
            Json::Value root;
            Json::Reader reader;
            Json::Value src_root;
            bool parse_success = reader.parse(body_str, src_root);
            if (!parse_success) {
                std::cout << "Failed to parse JSON data!" << std::endl;
                root["error"] = 1001;
                std::string jsonstr = root.toStyledString();
                beast::ostream(_response.body()) << jsonstr;
                return;
            }

            auto email = src_root["email"].asString();
            std::cout << "email is " << email << std::endl;

            root["error"] = 0;
            root["email"] = src_root["email"];
            root["msg"] = "recevie email post success";
            std::string jsonstr = root.toStyledString();
            beast::ostream(_response.body()) << jsonstr;
        }
        else {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "File not found\r\n";
        }
    }

    void write_response() {
        auto self = shared_from_this();

        _response.content_length(_response.body().size());

        http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t) {
            self->_socket.shutdown(tcp::socket::shutdown_type::shutdown_send, ec);
            self->_deadline.cancel();
        });
    }

    void check_deadline() {
        auto self = shared_from_this();

        _deadline.async_wait([self](beast::error_code ec) {
            if (!ec) {
                self->_socket.close(ec);
            }
        });
    }

public:
    http_connection(tcp::socket socket)
        : _socket(std::move(socket)) {
        std::cout << "Connection created\n";
    }

    void start() {
        read_request();
        check_deadline();
    }
};

void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
    acceptor.async_accept(socket, [&](beast::error_code ec) {
        if (!ec)
            std::make_shared<http_connection>(std::move(socket))->start();
        http_server(acceptor, socket);
    });
}

int main() {
    try {
        auto const address = net::ip::make_address("127.0.0.1");
        unsigned short port = static_cast<unsigned short>(8080);

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);

        ioc.run();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
