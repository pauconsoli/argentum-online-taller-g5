#include "acceptor_thread.h"

#include <iostream>
#include <utility>

#include <sys/socket.h>

#include "client_handler.h"
#include "common/liberror.h"
#include "server.h"

AcceptorThread::AcceptorThread(Socket& listener_sock, Server& srv):
    listener(listener_sock), server(srv), clients() {}

void AcceptorThread::reap() noexcept {
    clients.remove_if([this](ClientHandler* handler) {
        if (!handler->is_dead()) {
            return false;
        }
        try {
            handler->join_threads();
            server.remove_client(&handler->get_player_connection());
        } catch (const std::exception& e) {
            std::cerr << "[ACCEPTOR] Error en reap: " << e.what() << "\n";
        }
        delete handler;
        return true;
    });
}

void AcceptorThread::clear() noexcept {
    for (ClientHandler* handler : clients) {
        try {
            handler->hard_kill();
            handler->join_threads();
            server.remove_client(&handler->get_player_connection());
        } catch (const std::exception& e) {
            std::cerr << "[ACCEPTOR] Error en clear: " << e.what() << "\n";
        }
        delete handler;
    }
    clients.clear();
}

AcceptorThread::~AcceptorThread() {
    clear();
}

void AcceptorThread::run() {
    try {
        while (should_keep_running()) {
            try {
                Socket client_socket = listener.accept();

                reap();

                ClientHandler* handler = nullptr;
                try {
                    handler = new ClientHandler(std::move(client_socket), server);
                    server.add_client(&handler->get_player_connection());
                    clients.push_back(handler);
                    handler->start_threads();
                } catch (const std::exception& e) {
                    std::cerr << "[ACCEPTOR] Error armando handler: " << e.what() << "\n";
                    delete handler;
                }
            } catch (const LibError& e) {
                break;
            } catch (const std::exception& e) {
                std::cerr << "[ACCEPTOR] Error aceptando cliente: " << e.what() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[ACCEPTOR] Error fatal en run: " << e.what() << "\n";
    }

    clear();
    std::cerr << "[ACCEPTOR] Thread finalizado\n";
}

void AcceptorThread::stop() {
    Thread::stop();
    try {
        listener.shutdown(SHUT_RDWR);
    } catch (const std::exception& e) {
        std::cerr << "[ACCEPTOR] Error en stop (shutdown): " << e.what() << "\n";
    }
    try {
        listener.close();
    } catch (const std::exception& e) {
        std::cerr << "[ACCEPTOR] Error en stop (close): " << e.what() << "\n";
    }
}
