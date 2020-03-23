#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SIGINT 2
#define TIMEOUT_SEC 1
#define OK_MESSAGE "OK"
#define SERVER_PORT 51511
#define MAX_BUFFER_SIZE 256
#define READY_MESSAGE "READY"
#define TIMEOUT_MESSAGE "TIMEOUT"
#define SERVER_ADDRESS "127.0.0.1"
#define REGISTRATION_MESSAGE "MATRICULA"

int socket_to_close = 0;

void close_connection()
{
  if (socket_to_close != 0)
    close(socket_to_close);

  exit(0);
}

void verify_error_connection(int response_number)
{
  if (response_number < 0)
  {
    // Verifica timeout
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      printf("%s\n", TIMEOUT_MESSAGE);
    }
    close_connection();
  }
}

// Verifica se recebeu a mensgaem desejada do servidor
void verify_message(char received_message[], char expected_message[])
{
  if (strcmp(received_message, expected_message) != 0)
    close_connection();
}

// Verifica se enviou mensagem completa
void verify_error_send(int response_number, int message_length)
{
  if (response_number != message_length)
    close_connection();
}

struct sockaddr_in get_socket_address_config()
{
  struct sockaddr_in socket_address;

  inet_pton(AF_INET, SERVER_ADDRESS, &socket_address.sin_addr.s_addr);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(SERVER_PORT);

  return socket_address;
}

void set_socket_options(int client_socket_number)
{
  struct timeval timeout;
  timeout.tv_sec = TIMEOUT_SEC;
  timeout.tv_usec = 0;

  // Opção de timeout
  int server_scoket_opt_timeout = setsockopt(client_socket_number, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  verify_error_connection(server_scoket_opt_timeout);
}

void sigintHandler()
{
  close_connection();
}

int main(int argc, char *argv[])
{
  char received_message[MAX_BUFFER_SIZE];

  // Verifica se recebeu (1) senha do aluno e (2) matrícula na linha de comando
  if (argc < 3)
    return;

  //Fecha a conexão quando o usuário fecha o programa com CTRL + C
  signal(SIGINT, sigintHandler);

  // Pega a senha do aluno via linha de comando
  char student_password[MAX_BUFFER_SIZE];
  strcpy(student_password, argv[1]);

  // Pega matrícula via linha de comando
  int registration_number = atoi(argv[2]);

  int client_socket_number = socket(AF_INET, SOCK_STREAM, 0);
  verify_error_connection(client_socket_number);

  // Guarda número do socket do cliente para fechar ao final da execução do programa
  socket_to_close = client_socket_number;

  // Configura o timeout
  set_socket_options(client_socket_number);

  // Pega as configurações de endereço do socket para fazer o bind
  struct sockaddr_in server_socket_address = get_socket_address_config();

  // Realiza conexão com o servidor e verifica erro
  int server_socket_number = connect(client_socket_number, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));
  verify_error_connection(server_socket_number);

  // Recebe READY
  int recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
  verify_error_connection(recv_number);
  verify_message(received_message, READY_MESSAGE);

  // Envia senha do aluno
  int send_number = send(client_socket_number, student_password, sizeof(student_password), 0);
  verify_error_send(send_number, sizeof(student_password));

  memset(received_message, 0, sizeof(received_message));

  // Recebe OK
  recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
  verify_error_connection(recv_number);
  verify_message(received_message, OK_MESSAGE);

  // Recebe MATRICULA
  recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
  verify_error_connection(recv_number);
  verify_message(received_message, REGISTRATION_MESSAGE);

  // Converte número de matrícula para network byte order
  int converted_registration_number = htonl(registration_number);

  // Envia matrícula convertida
  send_number = send(client_socket_number, &converted_registration_number, sizeof(converted_registration_number), 0);
  verify_error_send(send_number, sizeof(converted_registration_number));

  // Recebe OK
  recv_number = recv(client_socket_number, received_message, MAX_BUFFER_SIZE, 0);
  verify_error_connection(recv_number);
  verify_message(received_message, OK_MESSAGE);

  // Fecha conexão
  close(client_socket_number);
}