#ifndef SOCKET_SERVER_CPP_SERVER_H
#define SOCKET_SERVER_CPP_SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <exception>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <arpa/inet.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <future>

const int MAX_SIZE = 1024;

class Server {
private:
    // Флаг
    std::atomic<bool> isActive = ATOMIC_VAR_INIT(true);

    std::thread udp_handler_thread;

    std::mutex  console_mutex;

    sockaddr_in server_address = sockaddr_in(),
                client_address = sockaddr_in();

    int listener_tcp = 0,
        listener_udp = 0;

    int address_length = 0,
        port = 0,
        connection_amount = 3;

    char* ip = nullptr;

    void start_tcp_handler();
    void start_udp_handler();

    static std::string find_numbers_in_string(char *data);

public:
    explicit Server(int port, char* ip, int connection_amount = 0);
    ~Server();

    void start();
    void stop();

    int getPort();
};

class SocketCreationException : public std::exception
{
public:
    SocketCreationException() = default;
    ~SocketCreationException() override = default;

    const char* what() const noexcept override {
        return "Problem with socket creation.";
    }
};

class BindingException : public std::exception {
public:
    BindingException() = default;

    ~BindingException() override = default;

    const char *what() const noexcept override {
        return "Problem with binding address at server.";
    }
};

class ListeningException : public std::exception
{
public:
    ListeningException() = default;
    ~ListeningException() override = default;

    const char* what() const noexcept override {
        return "Problem with listening at TCP server.";
    }
};

class AcceptingException : public std::exception
{
public:
    AcceptingException() = default;
    ~AcceptingException() override = default;

    const char* what() const noexcept override {
        return "Problem accepting at TCP server.";
    }
};


#endif //SOCKET_SERVER_CPP_SERVER_H
