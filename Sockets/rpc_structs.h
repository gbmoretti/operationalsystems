#ifndef _RPC_STRUCTS
#define _RPC_STRUCTS

#define FOPEN_REQUEST 1 /* requisição do cliente para um fopen */
#define FCLOSE_REQUEST 2 /* requisição do cliente para um fclose */
#define FREAD_REQUEST 3 /* requisição do cliente para um fread */
#define FWRITE_REQUEST 4 /* requisição do cliente para um fwrite */
#define FSEEK_REQUEST 5 /* requisição do cliente para um fseek */
#define FTELL_REQUEST 6 /* requisição do cliente para um ftell */
#define CONSULTA_REQUEST 7 /* requisição do cliente para uma consulta de função */

/* estrutura usada para enviar os argumentos da rpc_fopen() */
struct rpc_fopen_request {
  char path[255];
  char mode[8];
};

/* estrutura usada para enviar a resposta do rpc_fopen() */
struct rpc_fopen_reply {
  FILE* fp;
};

/* estrutura de requisição da rpc_fclose() */
struct rpc_fclose_request {
  FILE* fp;
};

/* estrutura de resposta da rpc_fclose() */
struct rpc_fclose_reply {
  int ret;
};

/* estrutura de requisção da rpc_fread() */
struct rpc_fread_request {
  int size;
  int nmemb;
  FILE* stream;
};

/* estrutura de resposta da rpc_fread() */
struct rpc_fread_reply {
  int ret;
};

/* estrutura de requisição da rpc_fwrite() */
struct rpc_fwrite_request {
  int size;
  int nmemb;
  FILE* stream;
};

/* estrutura de resposta da rpc_fwrite() */
struct rpc_fwrite_reply {
  int ret;
};

/* estrutura de requisição da rpc_fseek() */
struct rpc_fseek_request{
  FILE* stream;
  long offset;
  int whence;
};

/* estrutura de resposta da rpc_fseek() */
struct rpc_fseek_reply{
  long ret;
};

/* estrutura de requisição da rpc_ftell() */
struct rpc_ftell_request{
  FILE *stream;
};

/* estrutura de requisição da rpc_ftell() */
struct rpc_ftell_reply{
  int ret;
};

#endif

