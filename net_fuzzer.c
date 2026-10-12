
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define FLOOD_SIZE 10000

/* Connect to a local TCP server */
int connect_server(int port) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_IP,
                  &server.sin_addr) != 1) {
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

/* Send exactly the specified number of bytes */
void send_payload(int port, const void *data,
                  size_t length) {
    int sock = connect_server(port);
    if (sock < 0)
        return;

    const unsigned char *p = data;
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(sock, p + sent,
                         length - sent, 0);

        if (n <= 0) {
            perror("send");
            break;
        }

        sent += (size_t)n;
    }

    printf("Sent %zu of %zu bytes\n", sent, length);
    close(sock);
}

/* Test 1: Path traversal and absolute paths */
void test_path_traversal(int port) {
    printf("\n[TEST 1] Path Traversal\n");

    const char *paths[] = {
        "../../etc/passwd\n",
        "/etc/passwd\n",
        "../test.txt\n",
        "folder/../../../test.txt\n"
    };

    for (size_t i = 0;
         i < sizeof(paths) / sizeof(paths[0]);
         i++) {
        printf("Testing: %s", paths[i]);
        send_payload(port, paths[i], strlen(paths[i]));
    }
}

/* Test 2: 10,000 bytes without a terminator */
void test_buffer_flood(int port) {
    printf("\n[TEST 2] Buffer Flooding\n");

    char payload[FLOOD_SIZE];

    memset(payload, 'A', sizeof(payload));

    /* No newline or null terminator is sent */
    send_payload(port, payload, sizeof(payload));
}

/* Test 3: Abrupt disconnections */
void test_disconnections(int port) {
    printf("\n[TEST 3] Abrupt Disconnections\n");

    int sock = connect_server(port);

    if (sock >= 0) {
        printf("Closing connection immediately\n");
        close(sock);
    }

    sock = connect_server(port);

    if (sock >= 0) {
        const char partial[] = "partial/request";

        send(sock, partial, sizeof(partial) - 1, 0);

        printf("Disconnecting mid-request\n");
        close(sock);
    }
}

/* Test 4: Embedded binary null bytes */
void test_null_injection(int port) {
    printf("\n[TEST 4] Null Byte Injection\n");

    const unsigned char payload[] = {
        't', 'e', 's', 't',
        '.', 't', 'x', 't',
        '\0',
        '.', '.', '/',
        's', 'e', 'c', 'r', 'e', 't',
        '\n'
    };

    send_payload(port, payload, sizeof(payload));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *end;
    errno = 0;
    long port = strtol(argv[1], &end, 10);

    if (errno != 0 || *end != '\0' ||
        port < 1 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return EXIT_FAILURE;
    }

    printf("Starting local TCP fuzz tests...\n");

    test_path_traversal((int)port);
    test_buffer_flood((int)port);
    test_disconnections((int)port);
    test_null_injection((int)port);

    printf("\nFuzz testing completed.\n");

    return EXIT_SUCCESS;
}
