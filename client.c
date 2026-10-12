#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080


int main(int argc, char *argv[])
{
   char buffer[500 + 1]; /* +1 so we can add null terminator */
   char filename[256];
   int len, mysocket;
   struct sockaddr_in dest;

   /* Prompt the user for a filename */
   printf("Enter filename: ");
   fgets(filename, sizeof(filename), stdin);
   filename[strcspn(filename, "\n")] = '\0';

   mysocket = socket(AF_INET, SOCK_STREAM, 0);

   memset(&dest, 0, sizeof(dest));                /* zero the struct */
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* localhost, 127.0.0.1 */
   dest.sin_port = htons(PORT);                /* port 8080 */

   connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

   /* Send the request packet (the filename) */
   send(mysocket, filename, strlen(filename), 0);

   /* Await the response */
    len = recv(mysocket, buffer, sizeof(buffer) - 1, 0);

   /* We have to null terminate the received data ourselves */
   buffer[len] = '\0';

   /* Print the response (or server error message) */
   printf("%s\n", buffer);

   close(mysocket);
   return EXIT_SUCCESS;
}