#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")
#define PORT 80
#define MESSAGESIZE 1024
#define MAXSOCKETS 10

SOCKET new_socket[MAXSOCKETS];
int qtdsockets = 0;

char * readFile(char name[]){
    FILE* file;
    char ch;
    char *fileString = malloc(9999 * sizeof(char));

    int i = 0;
    file = fopen(name, "r");

    if(file == NULL){
       fileString = "404.html";
    }

    while((ch = fgetc(file)) != EOF){
        fileString[i] = ch;
        i++;
    }

    fclose(file);
    return fileString;
}

void getdata(int pos) {
  char request[MESSAGESIZE];
  char header[MESSAGESIZE];
  char page[MESSAGESIZE];

  char *returnedFile;
  char filename[1000];
  int j = 0, i = 5, count = 0, len;

  time_t mytime;
  mytime  = time(NULL);
  struct tm tm = *localtime(&mytime);

  while(1) {
    request[0]='\0';
    len=recv(new_socket[pos],request,MESSAGESIZE,0);
    if(len>0) {

      printf("\nRequisicao do navegador: %s - (%d caracteres)",request,len);
      request[0]='\0';

      while(request[i] != 'H'){
        filename[j] = request[i];
        j++;
        i++;
      };

      if(request[5] == ' '){
        filename[0] = 'b';
        filename[1] = 'a';
        filename[2] = 'r';
        filename[3] = 'r';
        filename[4] = 'a';
        filename[5] = '.';
        filename[6] = 'h';
        filename[7] = 't';
        filename[8] = 'm';
        filename[9] = 'l';
      }

      len=0;
      returnedFile = readFile(filename);

      if(returnedFile == "404.html") {
        returnedFile  = readFile("404.html");
      }

      strcpy(page, returnedFile);

      printf("\n%s\n", header);
      sprintf(header,"HTTP/1.1 200 OK\r\nDate: %d/%d/%d\t %d:%d:%d GMT\r\nContent-Length: %d\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n",tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, strlen(page)+1);

      send(new_socket[pos], header, strlen(header)+1, 0);
      send(new_socket[pos], page, strlen(page)+1, 0);
    }
  }
}

int main(int argc , char *argv[]) {
  WSADATA wsa;
  SOCKET s;
  struct sockaddr_in server , client;
  int c, pos;
  char errormessage[MESSAGESIZE];

  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
    printf("\nFalha na inicializacao da biblioteca Winsock: %d",WSAGetLastError());
    exit(EXIT_FAILURE);
  }

  if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) {
    printf("\nNao e possivel inicializar o socket: %d" , WSAGetLastError());
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( PORT );
  if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
    printf("\nNao e possivel construir o socket: %d" , WSAGetLastError());
    exit(EXIT_FAILURE);
  }

  listen(s,3);
  c = sizeof(struct sockaddr_in);
  printf("*** SERVER ***\n\nAguardando conexoes...\n\n");

  while(1) {
    pos=qtdsockets;
    new_socket[qtdsockets++] = accept(s, (struct sockaddr *)&client, &c);
    if (new_socket[pos] == INVALID_SOCKET) {
      printf("\nConexao n�o aceita. Codigo de erro: %d" , WSAGetLastError());
    }
    else {
      puts("\nConexao aceita.");
      printf("\nDados do cliente - IP: %s -  Porta: %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));

      if (qtdsockets <= MAXSOCKETS) {
        _beginthread(getdata,NULL,pos);
      }
      else {
        puts("\nNumero maximo de conexoes excedido!");
        strcpy(errormessage,"HTTP/1.1 429\r\nToo Many Requests\r\nContent-Type: text/html\r\nRetry-After: 60\r\n\r\n<HTML><H1>Numero maximo de conexoes excedido!</H1></HTML>\r\n");
        send(new_socket[pos] , errormessage , strlen(errormessage)+1, 0);
        closesocket(new_socket[pos]);
        qtdsockets--;
      }
    }
  }
  closesocket(s);
  WSACleanup();
}
