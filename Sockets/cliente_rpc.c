#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "rpc_structs.h"
#include "cliente_rpc.h"

int active_socket() {
  struct sockaddr_in serv_addr;
  struct hostent *servidor;
  int sock, i, j, porta;
  FILE* conf;
  char linha[21], host[15], p[6];

  conf = fopen("rpc.conf","r"); /* le o arquivo de configuração */

  fgets(linha,21,conf);/*pega as configuraçoes do arquivo rpc.conf*/

  for(i=0;linha[i]!=':';i++) { /* endereço do host vai até o ':' */
    host[i] = linha[i];
  }
  host[i] = '\0';

  for(j=0,i++;linha[i]!='\0';j++,i++) { /* porta começa depois do ':' até o final da linha  */
    p[j] = linha[i];
  }
  p[i] = '\0';
  porta = atoi(p); /**/

  /*criando socket*/
  sock = socket(AF_INET,SOCK_STREAM,0);/*cria o socket*/
  if(sock < 0) {
    printf("Erro ao criar o socket\n");
    exit(1);
  }

  servidor = gethostbyname(host); /*verifica se o servidor é valido*/
  if(servidor == NULL) {
    printf("Host invalido!\n");
    exit(1);
  }

  /*montando endereco do servidor*/
  bzero((char *)&serv_addr,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)servidor->h_addr,(char *)&serv_addr.sin_addr.s_addr,servidor->h_length);
  serv_addr.sin_port = htons(porta);

  /*estabelecendo conexao com servidor*/
  if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
    printf("Erro ao conectar\n");
    exit(1);
  }

  return sock;
}

/* função que lê mensagem do socket */
int read_socket(int sock,void* buffer,int tam) {
  int r; /* inteiro q vai receber o retorno da funcao read */

  r = read(sock,buffer,tam); /* le do socket "sock" e escreve em "buffer" que tem o tamanho "tam" */
  if(r < 0) { /* se houve falha na leitura, fecha o programa */
    printf("Erro na leitura do socket...\n");
    exit(1);
  }
  return r;
}

int write_socket(int sock,const void* buffer,int tam) {/*escreve no socket*/
  int r;

  r = write(sock,buffer,tam);/*escreve*/
  if(r < 0) {/*verifica se ocorreu algum erro*/
    printf("Erro na escrita do socket...");
    exit(1);
  }
  return r;
}

FILE* rpc_fopen(const char* path, const char* mode) {/*abre um arquivo*/
  int sock, func_req;
  struct rpc_fopen_request rq;
  struct rpc_fopen_reply rp;

  sock = active_socket();/*cria um socket*/

  strcpy((char*)(rq.path),path);/*caminho do arquivo*/
  strcpy((char*)(rq.mode),mode);/*modo de abertura do arquivo*/

  func_req = FOPEN_REQUEST;/*para indica ao servidor a operação requerida é um fopen()*/
  write_socket(sock,&func_req,sizeof(int));/*escreve a função*/
  write_socket(sock,&rq,sizeof(struct rpc_fopen_request));/*esceve na estrutura de requisição*/
  read_socket(sock,&rp,sizeof(struct rpc_fopen_reply));/*le na estrutura de resposta*/

  close(sock);/*fecha o socket aberto*/
  return rp.fp;
}

int rpc_fclose(FILE* fp) {/*fecha arquivo*/
  int sock, func_req;
  struct rpc_fclose_request rq;
  struct rpc_fclose_reply rp;

  sock = active_socket();/*cria um socket*/

  rq.fp = fp;/*guarda o arquivo na estrutura*/

  func_req = FCLOSE_REQUEST;/*indica ao servidor q a função requerida é o fclose()*/
  write_socket(sock,&func_req,sizeof(int));/*escreve no socket a funcao*/
  write_socket(sock,&rq,sizeof(struct rpc_fclose_request));/*escreve no socket os parametros*/
  read_socket(sock,&rp,sizeof(struct rpc_fclose_reply));/*le o retorno do servidor no socket*/
  close(sock);/*fecha o socket*/
  return rp.ret;

}

int rpc_fread(void* ptr, int size, int nmemb, FILE* stream) {/*funçao de leitura*/
  int sock, func_req;
  struct rpc_fread_request rq;
  struct rpc_fread_reply rp;


  sock = active_socket();/*abre um socket*/

  rq.size = size;/*tamanho dos elementos*/
  rq.nmemb = nmemb;/*numero de elementos lidos*/
  rq.stream = stream;/*arquivo*/

  func_req = FREAD_REQUEST;/*indica ao servidor q a função requerida é o fread()*/
  write_socket(sock,&func_req,sizeof(int));/*escreve a funcao no socket*/
  write_socket(sock,&rq,sizeof(struct rpc_fread_request));/*escreve os parametros no socket*/
  read_socket(sock,&rp,sizeof(struct rpc_fread_reply));/*le no socket  o retorno do servidor*/

  read_socket(sock,ptr,size*nmemb);/*le no socket o tamanho do retorno*/

  close(sock);/*fecha o socket*/
  return rp.ret;
}

int rpc_fwrite(const void* ptr, int size, int nmemb, FILE* stream) {/*função de escrita*/
  int sock, func_req;
  struct rpc_fwrite_request rq;
  struct rpc_fwrite_reply rp;

  sock = active_socket();/*abre um socket*/

  rq.size = size;/*tamanho*/
  rq.nmemb = nmemb;/*quantidade*/
  rq.stream = stream;/*arquivo*/

  func_req = FWRITE_REQUEST;/*indica ao servidor q a função requerida é o fwrite()*/
  write_socket(sock,&func_req,sizeof(int));/*passa pro servidor a funcao*/


  write_socket(sock,&rq,sizeof(struct rpc_fwrite_request));/*passa os parametros*/
  write_socket(sock,ptr,size*nmemb);/*indica o tamanho da escrita*/

  read_socket(sock,&rp,sizeof(struct rpc_fwrite_reply));/*le o retorno do servidor*/
  close(sock);/*fecha o socket*/
  return rp.ret;
}

int rpc_fseek(FILE* stream, long offset, int whence) {/*seta uma possição pra escrita np arquivo bin*/
  int sock, func_req;
  struct rpc_fseek_request rq;
  struct rpc_fseek_reply rp;

  sock = active_socket();/*abre um socket*/

  rq.stream = stream;/*arquivo*/
  rq.offset = offset;
  rq.whence = whence;/*seta o offset no no inicio, na posicao corrente ou no fim do arquivo*/

  func_req = FSEEK_REQUEST;/*indica a função q sera utilizada é o fseek()*/
  write_socket(sock,&func_req,sizeof(int));/*escreve a função no sockete*/
  write_socket(sock,&rq,sizeof(struct rpc_fseek_request));/*escreve os parametros*/
  read_socket(sock,&rp,sizeof(struct rpc_fseek_reply));/*le o retorno do servidor*/
  close(sock);/*fecha o socket*/
  return rp.ret;
}

int rpc_ftell(FILE* stream) {/*função ftell*/
  int sock, func_req;
  struct rpc_ftell_request rq;
  struct rpc_ftell_reply rp;

  sock = active_socket();/*abre um socket*/

  rq.stream = stream;/*pega o arquivo*/

  func_req = FTELL_REQUEST;/*indica a a funcao requisitada é o ftell()*/
  write_socket(sock,&func_req,sizeof(int));/*escreve a função no socket*/
  write_socket(sock,&rq,sizeof(struct rpc_ftell_request));/*escreve os parametros no socket*/
  read_socket(sock,&rp,sizeof(struct rpc_ftell_reply));/*le o retondo do servidor no socket*/
  close(sock);/*fecha o socket*/
  return rp.ret;
}

int rpc_consulta(const char* proc) { /*verifica se uma função exite no servidor*/
  int sock, func_req, retorno;

  sock = active_socket();/*novo socket*/

  func_req = CONSULTA_REQUEST;/*indica a função a ser utilizada no servidor*/
  write_socket(sock,&func_req,sizeof(int));/*passa a função*/
  write_socket(sock,proc,strlen(proc));/*passa os parametros*/
  read_socket(sock,&retorno,sizeof(int));/*recebe o retorno*/
  close(sock);/*fecha o socket*/
  return retorno;
}

