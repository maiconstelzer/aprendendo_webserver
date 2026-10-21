#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define ADDRESS "0.0.0.0"
#define PORT 8080
#define BUFFER_SIZE 1024

char *parseRoute(const char *route) {
   if(strcmp(route, "index") == 0) {
      return "index.html";
   }

   return "404.html";
}

void sendHTML(int *sock, const char *filename) {
   FILE *file = fopen(filename, "r");
   if(!file) {
      perror("No file");
      return;
   }

   char buffer[BUFFER_SIZE] = {0};
   size_t read = 0;
   char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
   send(*sock, header, strlen(header), 0);

   while ((read = fread(buffer, sizeof(buffer[0]), BUFFER_SIZE, file)) > 0) {
      send(*sock, buffer, read, 0);
   }

   fclose(file);
}

int new_socket() {
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      perror("No socket");
      return 0;
   }

   struct sockaddr_in remote;
   remote.sin_family = AF_INET;
   inet_pton(AF_INET, ADDRESS, &remote.sin_addr);
   remote.sin_port = ntohs(PORT);

   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(int));
   if(bind(sock, (void*)&remote, sizeof(remote)) < 0) {
      perror("No bind");
      return 0;
   }

   if(listen(sock, 5) < 0) {
       perror("No listening");
       return 0;
   }

   return sock;
}

int main() {
   int sock = new_socket();

   while(1) {
      struct sockaddr_in connAddr;
      socklen_t connAddrLen = sizeof(connAddr);
      int *conn = malloc(sizeof(int));
      if((*conn = accept(sock, (struct sockaddr *) &connAddr, &connAddrLen)) < 0) {
         perror("No conn\n");
         continue;
      }

      printf("New conn\n");

      char recBuf[BUFFER_SIZE] = {0};
      recv(*conn, recBuf, BUFFER_SIZE, 0);
      printf("%s\n", recBuf);
      char *token = recBuf + 5;
      char *route = strtok(token, " ");
      sendHTML(conn, parseRoute(route));
      close(*conn);
      printf("End conn\n");

      free(conn);
   }

   close(sock);
   return 0;
}
