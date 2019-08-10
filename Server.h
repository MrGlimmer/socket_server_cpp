#ifndef SOCKET_SERVER_CPP_SERVER_H
#define SOCKET_SERVER_CPP_SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <exception>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include <algorithm>
#include <ctime>

const int MAX_SIZE = 1024;

enum ServerType : int {
    standard = 0,
    debug
};

class Server {
private:
    // Флаг состояния
    std::atomic<bool> isActive = ATOMIC_VAR_INIT(true);

    // Объект потока для обработчика UPD запросов
    std::thread udp_handler_thread;

    // Мьютекс для работы с консолью
    std::mutex  console_mutex;

    // Адреса
    sockaddr_in server_address = sockaddr_in(),
                client_address = sockaddr_in();

    // Сокеты
    int listener_tcp = 0,
        listener_udp = 0;

    // Вспомогательные атрибуты
    int address_length = 0,
        port = 0,
        connection_amount = 3;

    // Тип сервера
    ServerType type;

    // Методы запуска обработчиков
    void start_tcp_handler();
    void start_udp_handler();

    // Метод обработки строки
    static std::string find_numbers_in_string(char *data);

public:
    explicit Server(int port, ServerType type);
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

class SelectingException : public std::exception
{
public:
    SelectingException() = default;
    ~SelectingException() override = default;

    const char* what() const noexcept override {
        return "Problem in socket selection.";
    }
};

#endif //SOCKET_SERVER_CPP_SERVER_H
