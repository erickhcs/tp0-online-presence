#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_MAX_CONNECTIONS 10
#define SERVER_PORT 51511

void verify_error(int response_number, char methodName[], int socket_to_close)
{
  if (response_number < 0)
  {
    printf("Erro ao realizar %s!\n\n", methodName);

    if (socket_to_close != 0)
      close(socket_to_close);

    exit(0);
  }
}

struct sockaddr_in get_socket_address_config()
{
  struct sockaddr_in socket_address;

  inet_pton(AF_INET, SERVER_ADDRESS, &socket_address.sin_addr.s_addr);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(SERVER_PORT);

  return socket_address;
}

int main()
{
  printf("\n");

  int server_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  verify_error(server_socket_number, "socket", 0);

  struct sockaddr_in server_socket_address = get_socket_address_config();

  int bind_number = bind(server_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  verify_error(bind_number, "bind", server_socket_number);

  int listen_number = listen(server_socket_number, SERVER_MAX_CONNECTIONS);
  verify_error(listen_number, "listen", server_socket_number);

  struct sockaddr_in client_socket_address;
  int client_scoket_length = sizeof(struct sockaddr_in);

  int accept_number = accept(server_socket_number, (struct sockaddr *)&client_socket_address, (socklen_t *)&client_scoket_length);
  verify_error(accept_number, "accept", server_socket_number);

  close(server_socket_number);

  printf("\n\n");
}