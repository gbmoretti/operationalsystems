#ifndef _CLIENTE_RPC
#define _CLIENTE_RPC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "rpc_structs.h"


int active_socket();
int read_socket(int,void*,int);
FILE* rpc_fopen(const char*,const char*);
int rpc_fclose(FILE*);
int rpc_fread(void*,int,int,FILE*);
int rpc_fwrite(const void*,int,int,FILE*);
int rpc_fseek(FILE*,long,int);
int rpc_ftell(FILE*);
int rpc_consulta(const char*);

#endif

