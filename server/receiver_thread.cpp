#include "receiver_thread.h"

#include <sys/socket.h>

#include "player_connection.h"

void ReceiverThread::run() {
    //...
}

void ReceiverThread::stop() {
    Thread::stop();
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (const std::exception& e) {
        std::cerr << "[RECEIVER] Error stopping: " << e.what() << "\n";
    }
}
