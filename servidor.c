#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SIGINT 2
#define TIMEOUT_SEC 1
#define OK_MESSAGE "OK"
#define SERVER_PORT 51511
#define PASSWORDS_LENGTH 8
#define MAX_BUFFER_SIZE 256
#define READY_MESSAGE "READY"
#define STUDENT_PASSWORD "ALUNO"
#define SERVER_MAX_CONNECTIONS 5
#define TIMEOUT_MESSAGE "TIMEOUT"
#define SERVER_ADDRESS "127.0.0.1"
#define TEACHER_PASSWORD "PROFESSOR"
#define REGISTRATION_MESSAGE "MATRICULA"

int socket_to_close = 0;

void close_connection()
{
  if (socket_to_close != 0)
    close(socket_to_close);

  exit(0);
}

bool verify_error_connection(int response_number)
{
  if (response_number < 0)
  {
    // Verifica TIMEOUT
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      printf("%s\n", TIMEOUT_MESSAGE);
    }
    return true;
  }
  return false;
}

// Verifica se recebeu a mensagem desejada do cliente
bool verify_message(char received_message[], char expected_message[])
{
  return (strcmp(received_message, expected_message) != 0);
}

// Verifica se enviou mensagem completa
bool verify_error_send(int response_number, int message_length)
{
  return (response_number != message_length);
}

struct sockaddr_in get_socket_address_config()
{
  struct sockaddr_in socket_address;

  inet_pton(AF_INET, SERVER_ADDRESS, &socket_address.sin_addr.s_addr);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(SERVER_PORT);

  return socket_address;
}

void set_socket_options(int server_socket_number)
{
  struct timeval timeout;
  timeout.tv_sec = TIMEOUT_SEC;
  timeout.tv_usec = 0;

  // Opção de timeout
  int server_scoket_opt_timeout = setsockopt(server_socket_number, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  if (verify_error_connection(server_scoket_opt_timeout) == true)
    exit(0);

  // Opção de reuso de endereço
  int server_socket_opt_reuse_addr = setsockopt(server_socket_number, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
  if (verify_error_connection(server_socket_opt_reuse_addr) == true)
    exit(0);
}

void sigintHandler()
{
  close_connection();
}

int main()
{
  int registration_number_count = 0;
  char received_message[MAX_BUFFER_SIZE];
  int registration_number_list[MAX_BUFFER_SIZE];

  //Fecha a conexão quando o usuário fecha o programa com CTRL + C
  signal(SIGINT, sigintHandler);

  int server_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  if (verify_error_connection(server_socket_number) == true)
    exit(0);

  // Configura o timeout e reuso de endereço
  set_socket_options(server_socket_number);

  // Guarda número do socket do servidor para fechar ao final da execução do programa
  socket_to_close = server_socket_number;

  // Pega as configurações de endereço do socket para fazer o bind
  struct sockaddr_in server_socket_address = get_socket_address_config();

  int bind_number = bind(server_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  if (verify_error_connection(bind_number) == true)
    exit(0);

  int listen_number = listen(server_socket_number, SERVER_MAX_CONNECTIONS);
  if (verify_error_connection(listen_number) == true)
    exit(0);

  // Variável que guarda o endereço do cliente conectado ao servidor
  struct sockaddr_in client_socket_address;
  int client_socket_length = sizeof(struct sockaddr_in);

  int client_socket_number;

  // Loop que aguarda novas conexões e executa o programa de fato.
  // As detecções de erros permitem que o servidor não pare seu funcionamento,
  // e continue para a próxima iteração, aceitando novos clientes
  while (client_socket_number = accept(server_socket_number, (struct sockaddr *)&client_socket_address, (socklen_t *)&client_socket_length))
  {
    if (verify_error_connection(client_socket_number) == true)
    {
      close(client_socket_number);
      continue;
    }
    // Envia READY
    int send_number = send(client_socket_number, READY_MESSAGE, sizeof(READY_MESSAGE), 0);
    if (verify_error_send(send_number, sizeof(READY_MESSAGE)) == true)
    {
      close(client_socket_number);
      continue;
    }

    // Recebe a senha do usuário
    int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
    if (verify_error_connection(recv_number) == true)
    {
      close(client_socket_number);
      continue;
    }

    // Executa serviço do aluno
    if (strcmp(received_message, STUDENT_PASSWORD) == 0)
    {
      // Envia OK
      send_number = send(client_socket_number, OK_MESSAGE, sizeof(OK_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(OK_MESSAGE)) == true)
      {
        close(client_socket_number);
        continue;
      }
      // Envia MATRICULA
      send_number = send(client_socket_number, REGISTRATION_MESSAGE, sizeof(REGISTRATION_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(REGISTRATION_MESSAGE)) == true)
      {
        close(client_socket_number);
        continue;
      }

      // Recebe número de matrícula
      int registration_number;
      recv_number = recv(client_socket_number, &registration_number, MAX_BUFFER_SIZE, 0);
      if (verify_error_connection(recv_number) == true)
      {
        close(client_socket_number);
        continue;
      }

      // Decodifica o número de matrícula em network byte order e guarda em memória
      registration_number = ntohl(registration_number);
      registration_number_list[registration_number_count] = registration_number;
      registration_number_count++;

      // Envia OK
      send_number = send(client_socket_number, OK_MESSAGE, sizeof(OK_MESSAGE), 0);
      if (verify_error_send(send_number, sizeof(OK_MESSAGE)) == true)
      {
        close(client_socket_number);
        continue;
      }
    }
    // Executa serviço do professor
    else if (strcmp(received_message, TEACHER_PASSWORD) == 0)
    {
      char registration_message[MAX_BUFFER_SIZE];
      memset(registration_message, 0, sizeof(registration_message));

      // Loop que gera a string com as matrículas para enviar ao cliente
      for (int i = 0; i < registration_number_count; i++)
      {
        char current_registration_number[MAX_BUFFER_SIZE];

        sprintf(current_registration_number, "%d", registration_number_list[i]);
        strcat(registration_message, current_registration_number);
        strcat(registration_message, "\n");
      }

      // Envia matrículas
      send_number = send(client_socket_number, registration_message, sizeof(registration_message), 0);
      if (verify_error_send(send_number, sizeof(registration_message)) == true)
      {
        close(client_socket_number);
        continue;
      }

      // Envia caractere nulo
      send_number = send(client_socket_number, "\0", sizeof("\0"), 0);
      if (verify_error_send(send_number, sizeof("\0")) == true)
      {
        close(client_socket_number);
        continue;
      }

      // Recebe OK
      int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
      if ((verify_message(received_message, OK_MESSAGE) || verify_error_connection(recv_number)) == true)
      {
        close(client_socket_number);
        continue;
      }
    }

    close(client_socket_number);
  }

  close(socket_to_close);
}