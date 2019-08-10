#include "Server.h"

Server::Server(int port_, ServerType type_)
    : port(port_), type(type_)
{
    // Очищаем объект адреса
    bzero( &server_address, sizeof( sockaddr_in  ) );
    /* Инициализируем адрес, к которому привяжем сокет */
    server_address.sin_family = AF_INET,
    server_address.sin_port = htons(port),
    server_address.sin_addr.s_addr = (type == ServerType::standard)
                                    ? inet_addr("127.0.0.1")
                                    : htonl(INADDR_LOOPBACK);

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

    // Выключаем поток
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
    if (listener_tcp <= 0)
    {
        this->stop();
        throw std::runtime_error("Problem with socket creation.");
    }

    fcntl(listener_tcp, F_SETFL, O_NONBLOCK);

    /* Связывание сокета с адресом, если не в порядке -> exception */
    if (bind(listener_tcp, (struct sockaddr *) &server_address, sizeof(server_address) ) < 0 )
    {
        this->stop();
        throw std::runtime_error("Problem with binding address at server.");
    }

    /* Начинаем прослушку порта, если не в порядке -> exception */
    if (listen(listener_tcp, connection_amount) < 0 )
    {
        this->stop();
        throw std::runtime_error("Problem with listening at TCP server.");
    }

    /* Инициализируем список клиентов */
    auto clients = new std::set<int>();
    clients->clear();

    while (isActive)
    {
        // Заполняем множество сокетов
        fd_set read_set{};
        FD_ZERO(&read_set);
        FD_SET(listener_tcp, &read_set);

        for(auto& it : *clients) FD_SET(it, &read_set);

        // Ждём события в одном из сокетов
        int max_value = std::max(listener_tcp, *max_element(clients->begin(), clients->end()));
        if(select(max_value + 1, &read_set, nullptr, nullptr, nullptr ) <= 0 )
        {
            this->stop();
            throw std::runtime_error("Problem in socket selection.");
        }

        // Определяем тип события и выполняем соответствующие действия
        if(FD_ISSET(listener_tcp, &read_set))
        {
            // Поступил новый запрос на соединение, используем accept
            int socket = accept(listener_tcp, (struct sockaddr *) &server_address, (socklen_t*) &address_length);
            if(socket < 0)
            {
                this->stop();
                throw std::runtime_error("Problem accepting at TCP server.");
            }

            fcntl(socket, F_SETFL, O_NONBLOCK);

            clients->insert(socket);
        }

        for(auto& client : *clients)
        {
            if(FD_ISSET(client, &read_set))
            {
                // Инициализируем и чистим буфер
                char buffer[MAX_SIZE];
                bzero(buffer, sizeof(buffer));

                // Поступили данные от клиента, читаем их
                int bytes = recv(client, buffer, MAX_SIZE, 0);

                if(bytes <= 0)
                {
                    // Соединение разорвано, удаляем сокет из множества
                    close(client);
                    clients->erase(client);
                    continue;
                }

                console_mutex.lock();
                std::cout << "************************************" << std::endl;
                std::cout << "Server received: " << buffer << std::endl;
                console_mutex.unlock();

                // Генерация ответа
                auto answer = strcpy(buffer, find_numbers_in_string(buffer).c_str());
                auto message_size = strlen(answer);
                message_size = message_size < MAX_SIZE ? message_size : MAX_SIZE;

                // Отправляем ответ обратно клиенту
                send(client, answer, message_size, 0);

                console_mutex.lock();
                std::cout << "Server sent back: " << answer << std::endl;
                std::cout << "************************************" << std::endl;
                console_mutex.unlock();
            }
        }
    }
}

void Server::start_udp_handler() {
    /* Создание сокета */
    listener_udp = socket(AF_INET, SOCK_DGRAM, 0);

    /* Если не в порядке -> exception */
    if (listener_udp == 0)
    {
        this->stop();
        throw std::runtime_error("Problem with socket creation.");
    }

    /* Связывание сокета с адресом, если не в порядке -> exception */
    if (bind(listener_udp, (struct sockaddr *) &server_address, sizeof(server_address) ) < 0 )
    {
        this->stop();
        throw std::runtime_error("Problem with binding address at server.");
    }

    while (isActive)
    {
        // Инициализируем и очищаем буфер
        char buffer[MAX_SIZE];
        bzero(buffer, sizeof(buffer));
        // Очищаем объект адреса клиента
        bzero( &client_address, sizeof( sockaddr_in  ) );

        // Ожидаем запросы
        socklen_t client_address_size = sizeof client_address;
        int bytes = recvfrom(listener_udp, buffer, MAX_SIZE, 0,
                (struct sockaddr *) &client_address, &client_address_size);
        buffer[bytes] = '\0';
        if (bytes <= 0) break;

        console_mutex.lock();
        std::cout << "************************************" << std::endl;
        std::cout << "Server received: " << buffer << std::endl;
        console_mutex.unlock();

        // Генерация ответа
        auto answer = strcpy(buffer, find_numbers_in_string(buffer).c_str());
        auto message_size = strlen(answer);
        message_size = message_size < MAX_SIZE ? message_size : MAX_SIZE;

        // Отправляем ответ клиенту
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
