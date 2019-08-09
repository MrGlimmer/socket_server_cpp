#include <iostream>
#include <cstring>
#include "Server.h"

int main(int argc, char *argv[]) {
    int     port = 3425,
            connection_amount = 3;
    char*   ip = nullptr;
    // INADDR_LOOPBACK - для теста в рамках одной машины
    // INADDR_ANY - любые сетевые адреса

    if (argc != 0)
    {
        port = static_cast<int>(strtol(argv[1], nullptr, 0));

        if (argc > 2)
        {
            connection_amount = static_cast<int>(strtol(argv[2], nullptr, 0));
        }

        // Debug - looppack
        if (argc > 3)
        {
            ip = argv[3];
        }
    }

    try {
        auto server = new Server(port, ip, connection_amount);
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