#include <iostream>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

const int BUFFER_SIZE = 1024;

class VsockListener {
public:
    VsockListener(int conn_backlog = 128) : conn_backlog(conn_backlog) {}

    void bind(int port) {
        sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_vm addr;
        memset(&addr, 0, sizeof(addr));
        addr.svm_family = AF_VSOCK;
        addr.svm_cid = VMADDR_CID_ANY;
        addr.svm_port = port;

        if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (listen(sockfd, conn_backlog) < 0) {
            perror("listen");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        std::cout << "Server listening on port " << port << std::endl;
    }

    void recv_data() {
        while (true) {
            struct sockaddr_vm client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock < 0) {
                perror("accept");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            char buffer[BUFFER_SIZE];
            while (true) {
                ssize_t n = recv(client_sock, buffer, BUFFER_SIZE, 0);
                if (n <= 0) break;
                std::cout << std::string(buffer, n) << std::flush;
            }
            std::cout << std::endl;
            close(client_sock);
        }
    }

    ~VsockListener() {
        close(sockfd);
    }

private:
    int sockfd;
    int conn_backlog;
};

void server_handler(int port) {
    VsockListener server;
    server.bind(port);
    server.recv_data();
}

int main(int argc, char *argv[]) {
    if (argc != 3 || std::string(argv[1]) != "server") {
        std::cerr << "Usage: " << argv[0] << " server <port>" << std::endl;
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[2]);
    server_handler(port);

    return 0;
}
