#include<stdlib.h>
#include<stdio.h>
#include<dirent.h>
#include<sys/stat.h>
#include<pwd.h>
#include<string.h>
#include<unistd.h>

/* trata o caminho passado pelo usuário */
const char* caminhoAbsoluto(const char* dir_inicial) {
  FILE *fp;
  char* dir;
  int tam, i;



  if(dir_inicial[0] != '/') { /* se o caminho informado não for um caminho absoluto */
    system("pwd > tmp"); /* escreve o caminho absoluto do executavel do programa em tmp */

    dir = (char*)malloc(sizeof(char)*256); /* aloca string que armazenara o caminho absoluto */

    if(!(fp = fopen("tmp","r"))) {
      printf("Isso nao deveria acontecer :(\n");
      exit(1);
    }
    fgets(dir,255,fp); /* le o caminho absoluto e salva na string dir */
    fclose(fp);

    tam = strlen(dir); /* pega o tamanho da string */
    dir[tam-1] = '\0'; /* corta o caracter \n da string */

    system("rm tmp"); /* remove arquivo usado para leitura do caminho absoluto */

    if(dir_inicial[0] != '.') { /* se diretorio inicial nao é . nem .. */
      sprintf(dir,"%s/%s",dir,dir_inicial); /* concatena o caminho absoluto do executavel com o  */
    }else if(dir_inicial[1] == '.') { /* diretorio inicial é .. */
      for(i=tam-2;dir[i] != '/';i--); /* encontra ultima '/' do diretorio */
      dir[i] = '\0'; /* e remove */
    }
    return dir;
  }else { /* se caminho passado pelo usuario for absoluto, só retorna ele */
    return dir_inicial;
  }
}

/* função recursiva que percorre os diretórios */
void listar(const char* dir) {
  DIR *dirstream;
  struct dirent *direntry;
  struct stat infos;
  struct passwd *usuario;
  char nomeabs[256];
  int uid;

  dirstream = opendir(dir); /* abre o diretorio */
  if(dirstream != NULL) { /* se diretorio foi aberto com sucesso */
    while((direntry = readdir(dirstream))) { /* lê próxima entrada do diretório */
      sprintf(nomeabs,"%s/%s",dir,direntry->d_name); /* concatena o caminho absoluto com o nome do arquivo */
      if (direntry->d_type != DT_DIR) { /* se entrada não for um diretório */
        if(stat(nomeabs,&infos)) { /* verifica se o programa consegui obter as informações da entrada */
          /*printf("==== Erro ao tentar obter stat de %s\n",nomeabs);*/
        }else {
          uid = (int)infos.st_uid; /* pega uid do dono do entrada */
          usuario = getpwuid(uid); /* pega nome do usuário pelo uid */
          printf("%s %d %s\n",nomeabs,(int)infos.st_size,usuario->pw_name); /* imprime informações do arquivo */
        }
      }else if(strcmp(direntry->d_name,"..") && strcmp(direntry->d_name,".")) { /* se o diretório não for '.' nem '..' */
        listar(nomeabs); /* abre este diretório */
      }
    }
  }
  closedir(dirstream); /* fecha diretorio */

}

int main(int argc,char *argv[]) {
  if(argc < 2) {
    printf("%s <diretorio>\n",argv[0]);
    return 1;
  }

  listar(caminhoAbsoluto(argv[1]));

  return 0;
}

