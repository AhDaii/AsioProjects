//
// Created by hunz1 on 2023/7/5.
//

#include "endpoint.h"
#include "boost/asio.hpp"
#include "iostream"
using namespace boost;

int client_end_point() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    boost::system::error_code ec;
    asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);

    if (ec.value() != 0) {
        std::cout << "Failed to parse the IP address. Error code = " << ec.value()
                  << ".Message is" << ec.message() << std::endl;
        return ec.value();
    }
    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int server_end_point() {
    unsigned short port_num = 3333;
    asio::ip::address ip_address = asio::ip::address_v6::any();
    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int create_tcp_socket() {
    asio::io_context ioc;
    asio::ip::tcp protocol = asio::ip::tcp::v4();
    asio::ip::tcp::socket socket(ioc);
    boost::system::error_code ec;

    socket.open(protocol, ec);
    if (ec.value() != 0) {
        std::cout << "Failed to open the socket. Error code = " << ec.value()
                  << ".Message is" << ec.message() << std::endl;
        return ec.value();
    }
    return 0;
}

int create_acceptor_socket() {
    asio::io_context ioc;
    //    asio::ip::tcp protocol = asio::ip::tcp::v6();
    //    asio::ip::tcp::acceptor acceptor(ioc);
    //    boost::system::error_code ec;
    //
    //    acceptor.open(protocol, ec);
    //    if (ec.value() != 0) {
    //        std::cout << "Failed to open the acceptor socket. Error code = " << ec.value()
    //                  << ".Message is" << ec.message() << std::endl;
    //        return ec.value();
    //    }
    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
    return 0;
}

int bind_acceptor_socket() {
    unsigned short port_num = 3333;
    asio::io_context ioc;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
    boost::system::error_code ec;

    acceptor.bind(ep, ec);
    if (ec.value() != 0) {
        std::cout << "Failed to bind the acceptor socket. Error code = " << ec.value()
                  << ".Message is" << ec.message() << std::endl;
        return ec.value();
    }
    return 0;

    return 0;
}

int connect_to_end() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}

int dns_connect_to_end() {
    std::string host = "llfc.club",
                port_num = "3333";
    asio::io_context ioc;
    asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);
    asio::ip::tcp::resolver resolver(ioc);

    try {
        asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);
        asio::ip::tcp::socket socket(ioc);
        asio::connect(socket, it);
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}

int accept_new_connect() {
    const int BACKLOG_SIZE = 30;
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ioc;

    try {
        asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);
        asio::ip::tcp::socket socket(ioc);
        acceptor.accept(socket);
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}

void use_const_buffer() {
    std::string buf = "Hello";
    asio::const_buffer asio_buf(buf.c_str(), buf.length());
    std::vector<asio::const_buffer> buffer_sequence;

    buffer_sequence.push_back(asio_buf);
    // ...
}

void use_buffer_str() {
    asio::const_buffers_1 output_str = asio::buffer("Hello");
    // ...
}

void use_buffer_array() {
    const size_t BUF_SIZE_BYTES = 20;
    std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);
    auto input_buf = asio::buffer(static_cast<void *>(buf.get()), BUF_SIZE_BYTES);
}

void write_to_socket(asio::ip::tcp::socket &sock) {
    std::string buf = "Hello";
    std::size_t tot_bytes_written = 0;

    // 循环发送
    // write_some返回每次写入的字节数
    while (tot_bytes_written != buf.length()) {
        tot_bytes_written += sock.write_some(asio::buffer(buf.c_str() + tot_bytes_written,
                                                          buf.length() - tot_bytes_written));
    }
}


void send_data_by_write_some() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        write_to_socket(socket);
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
    }
}

int send_data_by_send() {
    std::string raw_ip_address = "127.0.0.1",
                buf = "Hello";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        std::size_t send_length = socket.send(asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0) {
            return 0;
        }
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
    }
    return 0;
}

int send_data_by_write() {
    std::string raw_ip_address = "127.0.0.1",
                buf = "Hello";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        std::size_t send_length = asio::write(socket, asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0) {
            return 0;
        }
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
    }
    return 0;
}

std::string read_from_socket(asio::ip::tcp::socket &sock) {
    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];
    std::size_t tot_bytes_read = 0;

    while (tot_bytes_read != MESSAGE_SIZE) {
        tot_bytes_read += sock.read_some(asio::buffer(buf + tot_bytes_read, MESSAGE_SIZE - tot_bytes_read));
    }

    return {buf, tot_bytes_read};
}

int read_data_by_read_some() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        read_from_socket(socket);
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
}

int read_data_by_receive() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        const unsigned char MESSAGE_SIZE = 7;
        char buf_receive[MESSAGE_SIZE];
        std::size_t receive_length = socket.receive(asio::buffer(buf_receive, MESSAGE_SIZE));
        if (receive_length <= 0) {
            return 0;
        }
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
}

int read_data_by_read() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, ep.protocol());
        socket.connect(ep);
        const unsigned char MESSAGE_SIZE = 7;
        char buf_receive[MESSAGE_SIZE];
        std::size_t receive_length = asio::read(socket, asio::buffer(buf_receive, MESSAGE_SIZE));
        if (receive_length <= 0) {
            return 0;
        }
    } catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
}