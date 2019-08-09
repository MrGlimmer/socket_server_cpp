#include "Server.h"

Server::Server(int port_, char* ip_, int connection_amount_)
    : port(port_), ip(ip_), connection_amount(connection_amount_)
{
    bzero( &server_address, sizeof( sockaddr_in  ) );
    /* Инициализируем адрес, к которому привяжем сокет */
    server_address.sin_family = AF_INET,
    server_address.sin_port = htons(port),
    server_address.sin_addr.s_addr = strcmp(ip, "debug") == 0 ? htonl(INADDR_LOOPBACK) : inet_addr(ip);

    /* Инициализируем размер адреса */
    address_length = sizeof(server_address);
}

Server::~Server()
{
    this->stop();
}

/* Запуск сервера */
void Server::start()
{
    udp_handler_thread = std::thread(&Server::start_udp_handler, this);
    start_tcp_handler();
}

/* Остановка сервера */
void Server::stop()
{
    isActive = false;

    udp_handler_thread.join();

    close(listener_tcp);
    close(listener_udp);
}

int Server::getPort()
{
    return port;
}

void Server::start_tcp_handler() {
    /* Создание сокета */
    listener_tcp = socket(AF_INET, SOCK_STREAM, 0);

    /* Если не в порядке -> exception */
    if (listener_tcp == 0)
    {
        this->stop();
        throw SocketCreationException();
    }

    /* Связывание сокета с адресом, если не в порядке -> exception */
    if (bind(listener_tcp, (struct sockaddr *) &server_address, sizeof(server_address) ) < 0 )
    {
        this->stop();
        throw BindingException();
    }

    /* Начинаем прослушку порта, если не в порядке -> exception */
    if (listen(listener_tcp, connection_amount) < 0 )
    {
        this->stop();
        throw ListeningException();
    }

    while (isActive)
    {
        // Получаем соединение, если не в порядке -> exception
        int socket = accept(listener_tcp, (struct sockaddr *) &server_address, (socklen_t*) &address_length);
        if (socket < 0)
        {
            this->stop();
            throw AcceptingException();
        }

        while (true)
        {
            char buffer[MAX_SIZE];
            bzero(buffer, sizeof(buffer));

            int bytes = recv(socket, buffer, MAX_SIZE, 0);
            if (bytes <= 0) break;

            console_mutex.lock();
            std::cout << "************************************" << std::endl;
            std::cout << "Server received: " << buffer << std::endl;
            console_mutex.unlock();

            auto answer = strcpy(buffer, find_numbers_in_string(buffer).c_str());
            auto message_size = strlen(answer);
            message_size = message_size < MAX_SIZE ? message_size : MAX_SIZE;

            console_mutex.lock();
            std::cout << "Server sent back: " << answer << std::endl;
            std::cout << "************************************" << std::endl;
            console_mutex.unlock();

            send(socket, answer, message_size, 0);
        }

        /* Закрываем соединение */
        close(socket);
    }
}

void Server::start_udp_handler() {
    /* Создание сокета */
    listener_udp = socket(AF_INET, SOCK_DGRAM, 0);

    /* Если не в порядке -> exception */
    if (listener_udp == 0)
    {
        this->stop();
        throw SocketCreationException();
    }

    /* Связывание сокета с адресом, если не в порядке -> exception */
    if (bind(listener_udp, (struct sockaddr *) &server_address, sizeof(server_address) ) < 0 )
    {
        this->stop();
        throw BindingException();
    }

    while (isActive)
    {
        // Получаем данные и отправляем их обратно
        char buffer[MAX_SIZE];
        bzero(buffer, sizeof(buffer));
        bzero( &client_address, sizeof( sockaddr_in  ) );

        socklen_t client_address_size = sizeof client_address;
        int bytes = recvfrom(listener_udp, buffer, MAX_SIZE, 0,
                (struct sockaddr *) &client_address, &client_address_size);
        buffer[bytes] = '\0';
        if (bytes <= 0) break;

        console_mutex.lock();
        std::cout << "************************************" << std::endl;
        std::cout << "Server received: " << buffer << std::endl;
        console_mutex.unlock();

        auto answer = strcpy(buffer, find_numbers_in_string(buffer).c_str());
        auto message_size = strlen(answer);
        message_size = message_size < MAX_SIZE ? message_size : MAX_SIZE;


        sendto(listener_udp, answer, message_size, 0,
                (struct sockaddr *) &client_address, sizeof(client_address));

        console_mutex.lock();
        std::cout << "Server sent back: " << answer << std::endl;
        std::cout << "************************************" << std::endl;
        console_mutex.unlock();
    }
}

std::string Server::find_numbers_in_string(char *data)
{
    /* Инициализация переменных */
    auto numbers = new std::vector<long>();
    char *temp = data;

    /* Смотрим строку до конца */
    while (*temp != '\0')
    {
        int i = 0;
        /* Если доходим до конца строки -> выходим из циклов */
        while (temp[i] != '\0')
        {
            /* Если символ == цифра, то конвертируем его + следующие за ним цифры в число и записываем в вектор */
            if ( isdigit( temp[i] ) )
            {
                numbers->push_back( strtol(temp + i, &temp, 10) );
                break;
            }
            i++;
        }
        if (temp[i] == '\0') break;
    }

    /* Сортируем по возрастанию */
    std::sort(numbers->begin(), numbers->end());

    /* Инициализируем переменные для ответа */
    auto text = new std::string();
    long sum = 0;

    /* Складываем числа в строку + считаем их сумму */
    for (auto& number : *numbers)
    {
        text->append(std::to_string(number) + " ");
        sum += number;
    }
    /* Кладем сумму во строю строку */
    text->append( std::to_string(sum) );

    /* Чистим память */
    delete numbers;

    return *text;
}
