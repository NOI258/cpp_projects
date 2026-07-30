#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <fstream>

#define PORT 8080
#define BUFFER_SIZE 4096

std::string get_response() {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html; charset=utf-8\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << "<!DOCTYPE html><html><head><title>Hello</title></head>";
    oss << "<body><h1>Hello from C++ HTTP Server!</h1>";
    oss << "<p>This is a simple server running on your phone.</p>";
    oss << "</body></html>";
    return oss.str();
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 1. 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "socket failed\n";
        return 1;
    }

    // 2. 设置 SO_REUSEADDR 以便快速重启
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. 绑定
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed\n";
        return 1;
    }

    // 4. 监听
    if (listen(server_fd, 3) < 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // 5. 主循环：接受连接并处理
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "accept failed\n";
            continue;
        }

        // 读取请求（简单忽略）
        read(new_socket, buffer, BUFFER_SIZE);
        // 发送响应
        std::string response = get_response();
        send(new_socket, response.c_str(), response.size(), 0);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
