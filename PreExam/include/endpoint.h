//
// Created by hunz1 on 2023/7/4.
//

#ifndef ASIOPROJECT_NOSYNC_ENDPOINT_H
#define ASIOPROJECT_NOSYNC_ENDPOINT_H

extern int client_end_point();
extern int server_end_point();
extern int create_tcp_socket();
extern int create_acceptor_socket();
extern int bind_acceptor_socket();
extern int connect_to_end();
extern int dns_connect_to_end();
extern int accept_new_connect();
extern void use_const_buffer();
extern void use_buffer_str();
extern void use_buffer_array();


#endif//ASIOPROJECT_NOSYNC_ENDPOINT_H
