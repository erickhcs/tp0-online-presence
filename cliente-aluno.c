#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 51511
#define STUDENT_PASSWORD "ALUNO"
#define MAX_BUFFER_SIZE 1000
#define READY_MESSAGE "READY"
#define SIGINT 2

int socket_to_close = 0;

void close_connection(char error_message[])
{
  printf("%s\n\n", error_message);

  if (socket_to_close != 0)
    close(socket_to_close);

  exit(0);
}

void verify_error_connection(int response_number, char method_name[])
{
  if (response_number < 0)
  {
    char error_message[MAX_BUFFER_SIZE];
    strcat(error_message, "Erro ao realizar ");
    strcat(error_message, method_name);

    close_connection(error_message);
  }
}

void verify_message(char received_message[], char expected_message[])
{
  if (strcmp(received_message, expected_message) != 0)
  {
    char error_message[MAX_BUFFER_SIZE];
    strcat(error_message, "Mensagem inesperada do servidor. O cliente esperava \"");
    strcat(error_message, expected_message);
    strcat(error_message, "\" e recebeu \"");
    strcat(error_message, received_message);
    strcat(error_message, "\"");

    close_connection(error_message);
  }
}

void verify_error_send(int response_number, int message_length, char method_name[])
{
  if (response_number != message_length)
    close_connection(method_name);
}

struct sockaddr_in get_socket_address_config()
{
  struct sockaddr_in socket_address;

  inet_pton(AF_INET, SERVER_ADDRESS, &socket_address.sin_addr.s_addr);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(SERVER_PORT);

  return socket_address;
}

void sigintHandler()
{
  close_connection("Operação abortada");
}

int main()
{
  printf("\n");

  signal(SIGINT, sigintHandler);

  int client_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  verify_error_connection(client_socket_number, "socket");
  socket_to_close = client_socket_number;

  struct sockaddr_in server_socket_address = get_socket_address_config();

  int server_socket_number = connect(client_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  verify_error_connection(server_socket_number, "connect");

  char received_message[MAX_BUFFER_SIZE];

  int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
  verify_error_connection(recv_number, "recv");
  verify_message(received_message, READY_MESSAGE);

  int send_number = send(client_socket_number, STUDENT_PASSWORD, sizeof(STUDENT_PASSWORD), 0);
  verify_error_send(send_number, sizeof(STUDENT_PASSWORD), "send");

  close(client_socket_number);

  printf("\n\n");
}