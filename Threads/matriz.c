#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void *multiplica(void *l); /* função da thread que faz a multiplicação das matrizes */
void leArq(); /* le do arquivo a quantidade de matrizes e seu tamanho */
void alocaMt(); /* aloca memória para as matrizes mt1, mt2 e mtr */
void leitor(); /* faz a leitura das matrizes */
void *gravaMt(); /* função da thread que grava as matrizes resultantes no arquivo */

/* Variaveis globais. Compartilhadas entre as threads. */
int **mt1, **mt2, **mtr; /*Ponteiros para as matrizes*/
int size_mt, qtd_mt; /*Tamanho das matrizes e quantidade de matrizes*/
sem_t sem_grav, sem_leit; /*semaforos leitor gravador*/
sem_t *sem_calcs; /*semaforos para as threads calculadoras*/

int main(int argc, char** argv) {
    pthread_t gravadora, *calculadoras; /*threads*/
    int *args, i;

    leArq(); /*pega o numero de matrizes e seu tamanho*/
    alocaMt(); /*aloca as matrize mt1 mt2  e mtr com a dimensao lida do arquivo*/

    args =  malloc(sizeof(int)*size_mt); /*aloca um vetor para os arquimentos das threads calculadoras*/
    calculadoras = malloc(sizeof(pthread_t)*size_mt); /*aloca o numero de threads calculadoras == tamanho da matriz*/
    sem_calcs = malloc(sizeof(sem_t)*size_mt); /*cria um semaforo pra cada thread*/

    sem_init(&sem_leit,0,1); /*inicia o semaforo da thread (principal) leitora liberado*/
    sem_init(&sem_grav,0,0); /*inicia o semafoto da thread gravadora fechado*/
    for(i=0;i<size_mt;i++) /*inicia todos os semaforos das threads calculadoras*/
      sem_init(&sem_calcs[i],0,0);

    for(i=0;i<size_mt;i++) { /*cria todas as threadas calculadoras*/
        args[i] = i;
        if(pthread_create(&calculadoras[i],NULL,multiplica,(void*)&args[i])) {
          printf("Erro na criação das threads.\n");
          exit(-1);
        }
    }

    if(pthread_create(&gravadora,NULL,gravaMt,NULL)) { /*cria a thread de gravação*/
      printf("Erro na criação das threads.\n");
      exit(-1);
    }

    leitor(); /*inicia a leitura das matrizes*/
    pthread_join(gravadora,NULL); /* aguarda a thread gravadora finalizar. */

	return 0;
}

void leArq(){ /*le do arquivo a quantidade de matrizes e seu tamanho*/
    int aux;

    FILE *fp;
    fp = fopen ("input.data", "r");/*abre arquivo*/
    if (!fp)
        exit (-1);
    fscanf (fp, "%d", &aux);/*pega o tamanho das matrizes*/
    size_mt = aux;
    fscanf (fp, "%d", &aux);/*pega a quantidade de matrizes*/
    qtd_mt = aux;
    fclose(fp);
}

void alocaMt(){/*aloca as matrizes*/
    int i;
    /*=========aloca um vetor de ponteiros pra cada matriz*/
    mt1 = (int**)malloc(sizeof(int*)*size_mt);
    mt2 = (int**)malloc(sizeof(int*)*size_mt);
    mtr = (int**)malloc(sizeof(int*)*size_mt);
    /*aloca as linhas das matrizes*/
    for(i=0;i<size_mt;i++){
        mt1[i] = (int*)malloc(sizeof(int)*size_mt);
        mt2[i] = (int*)malloc(sizeof(int)*size_mt);
        mtr[i] = (int*)malloc(sizeof(int)*size_mt);
    }
}

void leitor(){/*faz a leitura das matrizes*/
    FILE *fp;
    int aux,i,j, k, matrizes;

    fp = fopen ("input.data", "r"); /*abre o arquivo*/
    if (!fp)
		exit (-1);
    fscanf (fp, "%d %d", &aux, &aux);/*anda as duas primeiras linhas*/

    matrizes = qtd_mt;/*pega a quantidade de matrizes para preservar o valor na variavel global*/

    do{
        sem_wait(&sem_leit); /*espera o semaforo de leitura ser liberado caso esteja fechado*/
        /*Le a primeira matriz*/
        for(i=0;i<size_mt;i++){
            for(j=0;j<size_mt;j++){
                fscanf(fp,"%d", &mt1[i][j]);
            }
        }
        /*le a segunda matriz*/
        for(i=0;i<size_mt;i++){
            for(j=0;j<size_mt;j++){
                fscanf(fp,"%d", &mt2[i][j]);
            }
        }
        matrizes -= 2;/*diminui a quantidade de matrizes a serem lidas*/
        /*libera para as threads calculadoras*/
        for(k=0;k<size_mt;k++) {
            sem_post(&sem_calcs[k]);
        }
    }while(matrizes != 0);/*repete enquanto ouverem matrizes a serem lidas no arquivo*/


    fclose(fp);/*fecha o arquivo*/
}

void *multiplica(void *l){ /*multiplica linha*coluna*/
    int j, k, *li, linha, matrizes;

    li = (int*)l;/*l é a linha a ser calculada*/
    linha = *li;/*indica a thread q vai calcular a linha*/

    matrizes = qtd_mt;/*copia a quantidade de matrizes para preservar o valor da variavel global*/

    do {
        sem_wait(&sem_calcs[linha]);/*verifica seu semaforo*/
        /*multiplica a linha e coluna*/
        for(j=0; j < size_mt; j++) {
            mtr[linha][j]=0;
            for(k=0; k < size_mt; k++) {
                mtr[linha][j] += mt1[linha][k] * mt2[k][j];
            }
        }

		sem_post(&sem_grav); /*libera para thread gravadora escrever no arquivo*/

        matrizes -= 2; /*diminui a quantidade de matrizes a serem calculadas*/
    }while(matrizes != 0);

    return NULL;
}

void *gravaMt() {
    FILE *fp;
    int i, j, k, matrizes;

    fp = fopen("output.data","w");

    matrizes = qtd_mt;

    do {
        for(k=0;k<size_mt;k++) { /*espera q todas as linhas sejam calculadas*/
            sem_wait(&sem_grav);
        }

        /*grava a matriz resultante*/
        for(i=0;i<size_mt;i++){
            for(j=0;j<size_mt;j++){
                fprintf(fp,"%d ", mtr[i][j]);
            }
            fprintf(fp,"\n");
        }
        fprintf(fp,"\n");
        sem_post(&sem_leit);/*libera a leitora para ler as proximas matrizes*/
        matrizes -= 2;/*diminui a quanteida de matrizes a serem gravadas*/
    }while(matrizes != 0);/*repete enquanto houverem matrizes a serem gravadas*/

    fclose(fp);
    pthread_exit(NULL); /* finaliza a thread e permite continuar a execução da thread principal */
}

