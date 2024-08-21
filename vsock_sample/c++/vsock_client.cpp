#include <iostream>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

class VsockStream {
public:
    VsockStream(int conn_tmo = 5) : conn_tmo(conn_tmo) {}

    void connect(int cid, int port) {
        sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_vm addr;
        memset(&addr, 0, sizeof(addr));
        addr.svm_family = AF_VSOCK;
        addr.svm_cid = cid;
        addr.svm_port = port;

        if (::connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("connect");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    void send_data(const std::string &data) {
        send(sockfd, data.c_str(), data.size(), 0);
    }

    ~VsockStream() {
        close(sockfd);
    }

private:
    int sockfd;
    int conn_tmo;
};

void client_handler(int cid, int port) {
    VsockStream client;
    client.connect(cid, port);
    std::string msg = "Hello, world!";
    client.send_data(msg);
}

int main(int argc, char *argv[]) {
    if (argc != 4 || std::string(argv[1]) != "client") {
        std::cerr << "Usage: " << argv[0] << " client <cid> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    int cid = std::stoi(argv[2]);
    int port = std::stoi(argv[3]);
    client_handler(cid, port);

    return 0;
}
