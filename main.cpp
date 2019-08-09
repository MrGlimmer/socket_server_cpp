#include <iostream>
#include "Server.h"

int main(int argc, char *argv[]) {
    // Инициализируем переменные
    int port = 3425;
    std::string address_type = "127.0.0.1";

    // Берем порт из параметра
    if (argc > 0) port = static_cast<int>(strtol(argv[1], nullptr, 0));
    // debug - локальный
    if (argc > 2) address_type = argv[2];


    try {
        // Создаем и запускаем сервер
        auto server = new Server(port, &address_type.c_str());
        std::cout << "Server has been launched at port " << server->getPort() << std::endl;
        server->start();

    } catch (SocketCreationException &e) {
        std::cout << "SocketCreationException: " << e.what() << std::endl;
    } catch (BindingException &e) {
        std::cout << "BindingException: " << e.what() << std::endl;
    } catch (ListeningException &e) {
        std::cout << "ListeningException: " << e.what() << std::endl;
    } catch (AcceptingException &e) {
        std::cout << "AcceptingException: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}