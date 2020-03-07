#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 51511
#define TEACHER_PASSWORD "PROFESSOR"
#define MAX_BUFFER_SIZE 1000

void close_connection(char methodName[], int socket_to_close)
{
  printf("Erro ao realizar %s!\n\n", methodName);

  if (socket_to_close != 0)
    close(socket_to_close);

  exit(0);
}

void verify_error_connection(int response_number, char methodName[], int socket_to_close)
{
  if (response_number < 0)
    close_connection(methodName, socket_to_close);
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

  int client_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  verify_error_connection(client_socket_number, "socket", 0);

  struct sockaddr_in server_socket_address = get_socket_address_config();

  int server_socket_number = connect(client_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  verify_error_connection(server_socket_number, "connect", client_socket_number);

  char received_message[MAX_BUFFER_SIZE];
  int recv_number = recv(server_socket_number, received_message, MAX_BUFFER_SIZE, 0);

  printf("\n\n");
}