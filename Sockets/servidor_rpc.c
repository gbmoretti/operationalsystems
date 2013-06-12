#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rpc_structs.h"

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

/* função que envia mensagem via socket */
int write_socket(int sock,void* buffer,int tam) {
  int r;

  r = write(sock,buffer,tam); /* escreve no socket "sock" o conteudo de "buffer" que tem o tamanho "tam" */
  if(r < 0) {
    printf("Erro na escrita do socket...");
    exit(1);
  }
  return r;
}

/* cria um socket passivo, que vai ouvir na porta passada por argumento */
int passive_socket(int porta) {
  int sock; /* socket passivo */
  struct sockaddr_in serv_addr; /* estrutura do endereço do servidor */

  /*criando socket */
  sock = socket(AF_INET,SOCK_STREAM,0);

  /*montando endereço do servidor */
  serv_addr.sin_family = AF_INET; /* tipo do endereço */
  serv_addr.sin_port = htons(porta); /* porta */
  serv_addr.sin_addr.s_addr = INADDR_ANY; /*informando que o endereço é de servidor*/
  bzero(&(serv_addr.sin_zero),8); /* preenche o resto da estrutura com 0 para o tamanho ficar padronizado e o compilador permitir cast da estrutura sockaddr */

  /* vincula socket "sock" com o endereço "serv_addr" */
  if(bind(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
    printf("Binding error!\n");
    exit(1);
  }

  /*marcando socket como passivo (vai receber)*/
  listen(sock,5);

  return sock;
}

/* recebe os parametros da fopen() do cliente, executa a função e retorna o resultado para o cliente */
void rpc_fopen(int sock_conectado) {
  struct rpc_fopen_request rq; /* estrutura de requisição */
  struct rpc_fopen_reply rp; /* estrutura de resposta */


  read_socket(sock_conectado,&rq,sizeof(struct rpc_fopen_request)); /* lê a requisição */

  rp.fp = fopen(rq.path,rq.mode); /* executa o fopen() */

  write_socket(sock_conectado,&rp,sizeof(struct rpc_fopen_reply)); /* retorna o resultado para o cliente */
}

/* recebe os parametros da fclote() do cliente, executa a função e retorna o resultado */
void rpc_fclose(int sock_conectado) {
  struct rpc_fclose_request rq; /* estrutura de requisição */
  struct rpc_fclose_reply rp; /* estrutura de reposta */

  read_socket(sock_conectado,&rq,sizeof(struct rpc_fclose_request)); /* recebe a requisição */

  rp.ret = fclose(rq.fp); /* executa a função */

  write_socket(sock_conectado,&rp,sizeof(struct rpc_fclose_reply)); /* retorna o resultado */
}

/* recebe os parametros do cliente, executa a função e retorna o resultado */
void rpc_fread(int sock_conectado) {
  void* ptr; /* ponteiro para o resultado da leitura do arquivo */
  struct rpc_fread_request rq; /* estrutura de requisição */
  struct rpc_fread_reply rp; /* estrutura de reposta */


  read_socket(sock_conectado,&rq,sizeof(struct rpc_fread_request)); /* lê os parametros */

  ptr = malloc(sizeof(char) * rq.size * rq.nmemb); /* aloca espaço na memória para guardar os dados lidos do arquivo */
  rp.ret = fread(ptr,rq.size,rq.nmemb,rq.stream); /* lê o arquivo */

  write_socket(sock_conectado,&rp,sizeof(struct rpc_fread_reply)); /* envia resultado da execução da função */
  write_socket(sock_conectado,ptr,rq.size*rq.nmemb); /* envia conteudo lido do arquivo */

  free(ptr); /* depois de conteudo enviado, memoria pode ser liberada */
}

/* recebe os parametros do cliente, executa a fwrite() e retorna o resultado */
void rpc_fwrite(int sock_conectado) {
  struct rpc_fwrite_request rq; /* estrutura de requisição */
  struct rpc_fwrite_reply rp; /* estrutura de resposta */
  void* ptr_total; /* ponteiro para a area de memoria a ser gravada no arquivo */


  read_socket(sock_conectado,&rq,sizeof(struct rpc_fwrite_request)); /* recebe os parametros */

  ptr_total = malloc(rq.size*rq.nmemb); /* aloca para ptr_total, tamanho suficiente para receber os dados a serem gravados no arquivo */
  read_socket(sock_conectado,ptr_total,rq.size*rq.nmemb); /* recebe os dados a serem gravados */

  rp.ret = fwrite(ptr_total,rq.size,rq.nmemb,rq.stream); /* chama a função fwrite() */
  write_socket(sock_conectado,&rp,sizeof(struct rpc_fwrite_reply)); /* retorna o resultado para o cliente */

  free(ptr_total); /* libera memoria usada para guardar o dados escritos */

}

/* recebe parametros, executa e retorna o resultado da função fseek() */
void rpc_fseek(int sock_conectado) {
  struct rpc_fseek_request rq; /* requisição */
  struct rpc_fseek_reply rp; /* resposta */

  read_socket(sock_conectado,&rq,sizeof(struct rpc_fseek_request)); /* recebe os parametros */

  rp.ret = fseek(rq.stream,rq.offset,rq.whence); /* executa função */

  write_socket(sock_conectado,&rp,sizeof(struct rpc_fseek_reply)); /* retorna resultado */
}

/* recebe parametros, executa e retorna o resultado da função ftell() */
void rpc_ftell(int sock_conectado) {
  struct rpc_ftell_request rq; /* requisição */
  struct rpc_ftell_reply rp; /* resposta */

  read_socket(sock_conectado,&rq,sizeof(struct rpc_ftell_request)); /* lê os parametros enviado pelo cliente */

  rp.ret = ftell(rq.stream); /* executa a ftell() */

  write_socket(sock_conectado,&rp,sizeof(struct rpc_ftell_reply)); /* retorna o resultado da execução */
}

/* retorna 1 se a string enviada pelo cliente corresponde a uma função implementada pelo servidor, retorna 0 se nao for */
void rpc_consulta(int sock_conectado) {
  int retorno; /* valor a ser retornado para o cliente */
  char* proc; /* string com o nome da função consultada */

  proc = (char*) malloc(sizeof(char) * 255); /* aloca espaço na memoria para guardar a string com o nome da função */

  read_socket(sock_conectado,proc,255); /* le nome da funcao */

  if(strcmp("fopen",proc) == 0 ||
    strcmp("fclose",proc) == 0 ||
    strcmp("fread",proc) == 0 ||
    strcmp("fwrite",proc) == 0 ||
    strcmp("fseek",proc) == 0 ||
    strcmp("ftell",proc) == 0) { /* se a string proc for igual a uma das funções implementadas pelo servidor, retorna 1 */

    retorno = 1;
  }else { /* se nao, retorna 0 */
    retorno = 0;
  }

  write_socket(sock_conectado,&retorno,sizeof(int)); /* envia resultado da consulta para o cliente */
  free(proc); /* libera memoria utilizada para guardar nome da função */
}

int main(int argc, char** argv) {
  int sock, sock_conectado, porta; /* socket passivo, socket conectado com cliente, porta que o servidor vai ficar ouvindo */
  socklen_t sock_len; /* tamnho do endereço do cliente */
  int func_req; /* variavel que guarda a função requisitada pelo cliente */
  struct sockaddr_in cliente_addr; /* endereço do cliente */

  if(argc != 2) { /* verifica se a porta foi informada como argumento */
    printf("%s PORTA\n",argv[0]);
    exit(1);
  }else {
    porta = atoi(argv[1]);
    printf("Ouvindo a porta %d\n",porta);
  }

  sock = passive_socket(porta); /* cria um socket passivo e guarda em "sock" */

  while(1) { /* loop infinito, depois que o servidor responder a uma requisição, ja pode responder outra */
    sock_len = sizeof(cliente_addr); /* guarda em sock_len o tamanho do endereço do cliente */
    sock_conectado = accept(sock,(struct sockaddr*)&cliente_addr,&sock_len); /* aceita uma conexão do cliente.
                                                                                Enquanto nenhum cliente conectar com servidor. O programa fica parado aqui. */
    if(sock_conectado < 0) {
      printf("Erro ao aceitar conexão do cliente\n");
      exit(1);
    }

    read_socket(sock_conectado,&func_req,sizeof(int)); /* recebe do cliente qual função ele deseja executar */
    switch(func_req) { /* chama a respectiva função */
      case FOPEN_REQUEST:
        rpc_fopen(sock_conectado);
        printf ("fopen\n");
        break;
      case FCLOSE_REQUEST:
        rpc_fclose(sock_conectado);
        break;
      case FREAD_REQUEST:
        rpc_fread(sock_conectado);
        printf ("fread\n");
        break;
      case FWRITE_REQUEST:
        rpc_fwrite(sock_conectado);
        printf ("fwrite\n");
        break;
      case FSEEK_REQUEST:
        rpc_fseek(sock_conectado);
        break;
      case FTELL_REQUEST:
        rpc_ftell(sock_conectado);
        break;
      case CONSULTA_REQUEST:
        rpc_consulta(sock_conectado);
        break;
      default:
        printf("Requisicao %d nao reconhecida!\n",func_req);
        break;
    }

  }
  return 0;
}

