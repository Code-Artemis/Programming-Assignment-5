#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

int server_fd, new_socket = -1;

// Ctrl+C handler: close open sockets and exit
void shutdown_server(int sig)
{
    if (new_socket >= 0)
        close(new_socket);
    close(server_fd);
    exit(0);
}

int main(int argc, char const* argv[])
{
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char content[256];
    size_t n;
    FILE* fp;
    char* msg;

    signal(SIGINT, shutdown_server);
    signal(SIGPIPE, SIG_IGN);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket
             = accept(server_fd, (struct sockaddr*)&address,
                      &addrlen))
            < 0) {
            perror("accept");
            continue;
        }

        // subtract 1 for the null
        // terminator at the end
        valread = recv(new_socket, buffer,
                       sizeof(buffer) - 1, 0);
        if (valread <= 0) {
            close(new_socket);
            new_socket = -1;
            continue;
        }
        buffer[valread] = '\0';

        // strip trailing newline / carriage return
        while (valread > 0 && (buffer[valread - 1] == '\n'
                               || buffer[valread - 1] == '\r'))
            buffer[--valread] = '\0';

        printf("Requested file: %s\n", buffer);

        // Validate filename: not empty, max 255 bytes,
        // no embedded null bytes, no path separators,
        // no path traversal
        if (valread == 0 || valread > 255
            || strlen(buffer) != (size_t)valread
            || strchr(buffer, '/') != NULL
            || strstr(buffer, "..") != NULL) {
            msg = "Error: Invalid filename";
            send(new_socket, msg, strlen(msg), 0);
        } else if ((fp = fopen(buffer, "r")) == NULL) {
            msg = "Error: File Not Found";
            send(new_socket, msg, strlen(msg), 0);
        } else {
            // read up to 256 bytes to detect files over 255
            n = fread(content, 1, sizeof(content), fp);
            fclose(fp);
            if (n > 255) {
                msg = "Error: File exceeds 255 characters";
                send(new_socket, msg, strlen(msg), 0);
            } else {
                send(new_socket, content, n, 0);
            }
        }

        // closing the connected socket
        close(new_socket);
        new_socket = -1;
    }

    // closing the listening socket
    close(server_fd);
    return 0;
}