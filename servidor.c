#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SIGINT 2
#define OK_MESSAGE "OK"
#define SERVER_PORT 51511
#define MAX_BUFFER_SIZE 256
#define READY_MESSAGE "READY"
#define STUDENT_PASSWORD "ALUNO"
#define SERVER_MAX_CONNECTIONS 10
#define SERVER_ADDRESS "127.0.0.1"
#define TEACHER_PASSWORD "PROFESSOR"
#define REGISTRATION_MESSAGE "MATRICULA"

int socket_to_close = 0;
int registration_number_count = 0;
char received_message[MAX_BUFFER_SIZE];
int registration_number_list[MAX_BUFFER_SIZE];

void close_connection(char error_message[])
{
  printf("%s\n\n", error_message);

  if (socket_to_close != 0)
    close(socket_to_close);

  exit(0);
}

bool verify_error_connection(int response_number, char method_name[])
{
  if (response_number < 0)
  {
    char error_message[MAX_BUFFER_SIZE];
    strcat(error_message, "Erro ao realizar ");
    strcat(error_message, method_name);

    printf("%s\n\n", error_message);
    return true;
  }
  return false;
}

bool verify_message(char received_message[], char expected_message[])
{
  if (strcmp(received_message, expected_message) != 0)
  {
    char error_message[MAX_BUFFER_SIZE];
    strcat(error_message, "Mensagem inesperada do cliente. O servidor esperava \"");
    strcat(error_message, expected_message);
    strcat(error_message, "\" e recebeu \"");
    strcat(error_message, received_message);
    strcat(error_message, "\"");

    printf("%s\n\n", error_message);
    return true;
  }
  return false;
}

bool verify_error_send(int response_number, int message_length, char method_name[])
{
  if (response_number != message_length)
  {
    char error_message[MAX_BUFFER_SIZE];
    strcat(error_message, "Erro ao realizar ");
    strcat(error_message, method_name);

    printf("%s\n\n", error_message);
    return true;
  }
  return false;
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
  signal(SIGINT, sigintHandler);

  int server_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  if (verify_error_connection(server_socket_number, "socket") == true)
    exit(0);
  socket_to_close = server_socket_number;

  struct sockaddr_in server_socket_address = get_socket_address_config();

  int bind_number = bind(server_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  if (verify_error_connection(bind_number, "bind") == true)
    exit(0);

  int listen_number = listen(server_socket_number, SERVER_MAX_CONNECTIONS);
  if (verify_error_connection(listen_number, "listen") == true)
    exit(0);

  struct sockaddr_in client_socket_address;
  int client_socket_length = sizeof(struct sockaddr_in);

  int client_socket_number;

  while (client_socket_number = accept(server_socket_number, (struct sockaddr *)&client_socket_address, (socklen_t *)&client_socket_length))
  {
    if (verify_error_connection(client_socket_number, "accept") == true)
      continue;

    int send_number = send(client_socket_number, READY_MESSAGE, sizeof(READY_MESSAGE), 0);
    if (verify_error_send(send_number, sizeof(READY_MESSAGE), "send") == true)
    {
      close(client_socket_number);
      continue;
    }

    int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
    if (verify_error_connection(recv_number, "recv") == true)
    {
      close(client_socket_number);
      continue;
    }

    if (strcmp(received_message, STUDENT_PASSWORD) == 0)
    {
      send_number = send(client_socket_number, OK_MESSAGE, sizeof(OK_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(OK_MESSAGE), "send") == true)
      {
        close(client_socket_number);
        continue;
      }

      send_number = send(client_socket_number, REGISTRATION_MESSAGE, sizeof(REGISTRATION_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(REGISTRATION_MESSAGE), "send") == true)
      {
        close(client_socket_number);
        continue;
      }

      int registration_number;
      recv_number = recv(client_socket_number, &registration_number, MAX_BUFFER_SIZE, 0);
      if (verify_error_connection(recv_number, "recv") == true)
      {
        close(client_socket_number);
        continue;
      }

      registration_number = ntohl(registration_number);
      registration_number_list[registration_number_count] = registration_number;
      registration_number_count++;

      send_number = send(client_socket_number, OK_MESSAGE, sizeof(OK_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(OK_MESSAGE), "send") == true)
      {
        close(client_socket_number);
        continue;
      }
    }
    else if (strcmp(received_message, TEACHER_PASSWORD) == 0)
    {
      char registration_message[MAX_BUFFER_SIZE];
      memset(registration_message, 0, sizeof(registration_message));

      for (int i = 0; i < registration_number_count; i++)
      {
        char current_registration_number[MAX_BUFFER_SIZE];

        sprintf(current_registration_number, "%d", registration_number_list[i]);
        strcat(registration_message, current_registration_number);
        strcat(registration_message, "\n");
      }

      send_number = send(client_socket_number, registration_message, sizeof(registration_message), 0);
      if (verify_error_send(send_number, sizeof(registration_message), "send") == true)
      {
        close(client_socket_number);
        continue;
      }

      send_number = send(client_socket_number, "\0", sizeof("\0"), 0);
      if (verify_error_send(send_number, sizeof("\0"), "send") == true)
      {
        close(client_socket_number);
        continue;
      }

      int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
      if ((verify_message(received_message, OK_MESSAGE) || verify_error_connection(recv_number, "recv")) == true)
      {
        close(client_socket_number);
        continue;
      }
    }

    close(client_socket_number);
  }

  close(socket_to_close);
}