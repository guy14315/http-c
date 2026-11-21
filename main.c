#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_html(int client_socket, const char *file) {
  FILE *html = fopen(file, "r");
  if (!html) {
    perror("Error opening HTML file.");
    return;
  }

  char buffer[BUFFER_SIZE] = {0};
  size_t read = 0;

  char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  send(client_socket, header, strlen(header), 0);

  while ((read = fread(buffer, 1, BUFFER_SIZE, html)) > 0) {
    send(client_socket, buffer, read, 0);
  }

  fclose(html);
}

int handle_client(int client_socket) {
  char buf[BUFFER_SIZE];

  ssize_t n = read(client_socket, buf, sizeof(buf) - 1);
  if (n < 0) {
    perror("read()");
    close(client_socket);
    return -1;
  }

  if (n > 0) {
    buf[n] = '\0';
    printf("REQUEST:\n%s", buf);
    send_html(client_socket, "static/index.html");
  }

  close(client_socket);
  return 0;
}

int main(void) {

  /* declare */
  int rc = 0;
  struct sockaddr_in bind_addr;
  int tcp_socket = 0;
  int ret = 0;
  int enabled = true;

  /* initialize */
  memset(&bind_addr, 0, sizeof(bind_addr));
  tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (tcp_socket == -1) {
    perror("socket()");
    return 1;
  }
  printf("socket creation succeeded\n");

  (void)setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enabled,
                   sizeof(enabled));

  bind_addr.sin_port = htons(PORT);
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = INADDR_ANY;

  rc = bind(tcp_socket, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
  if (rc < 0) {
    perror("bind()");
    ret = 1;
    goto exit;
  }
  printf("bind succeeded\n");

  rc = listen(tcp_socket, SOMAXCONN);
  if (rc < 0) {
    perror("listen()");
    ret = 1;
    goto exit;
  }
  printf("listen succeeded\n");

  while (1) {
    printf("wating for connections\n");
    int client_socket = accept(tcp_socket, NULL, NULL);

    printf("connected \n");
    rc = handle_client(client_socket);
  }

exit:
  close(tcp_socket);
  return ret;
}
