#include "klientfunksjoner.h"

int interrupted = 0;


/*
  Setter en global int til 1 dersom programmet fanger opp et
  SIGINT signal
*/
void interrupt_handler(){
    interrupted = 1;
#ifdef DEBUG
    printf(">> %d << Recieved interrupt signal. \n",getpid());
#endif
}

/*
  Denne metoden setter opp signal-handler for SIGINT og lager pipes.
  Deretter forkes det to ganger for aa lage to barneprosesser. Disse
  barneprossessene kaller paa read_pipe og naar de blir ferdige med
  read_pipe metoden vil de terminere. Deretter vil det kalles paa
  establish_connection med argumentene til programmet som parameter
  til metoden og hvis det gaar vil det vaere en ordrelokke
  hvor bruker kan be om jobber fra socket. Dersom det skjer en feil
  naar det proves aa hentes en jobb vil det hoppes ut av ordrelokken
  og programmet vil terminere.

  Input:
  argc: antall argumenter som sendes med programmet(brukes til aa sjekke riktig bruk)
  argv: string array som inneholder argumentene som ble sendt ved start av programmet
  argv[1]=addresse og argv[2]=port.

  Return:
  Returnerer EXIT_FAILURE ved feil med pipe-kall,fork-kall eller establish_connection-kall
*/
int main(int argc,char* argv[]){

    struct sigaction sa;
    memset(&sa,0,sizeof(struct sigaction));
    sa.sa_handler = &interrupt_handler;
    sigaction(SIGINT,&sa,NULL);

    if(argc < 3){
        printf("Usage: %s [serv-addr] [port-num]\n",argv[0]);
        return EXIT_SUCCESS;
    }
#ifdef DEBUG
    printf(">> %d << Starting process in DEBUG-mode\n",getpid());
#endif

    int pipefd1[2];
    int pipefd2[2];
    if(pipe(pipefd1) == -1 || pipe(pipefd2) == -1){
        perror("Pipe creation failure:");
        return EXIT_FAILURE;
    }
#ifdef DEBUG
    printf(">> %d << Succesfully created pipes\n",getpid());
#endif

    pid_t retv = fork();
    if(retv == -1){
        perror("Fork failure");
        close_pipe(pipefd1);
        close_pipe(pipefd2);
        return EXIT_FAILURE;
    }else if(retv == 0){
#ifdef DEBUG
        printf(">> %d << Started process\n",getpid());
#endif
        close(pipefd1[1]);
        close_pipe(pipefd2);
        read_pipe(pipefd1[0],stdout);
        return EXIT_SUCCESS;
    }

    retv = fork();
    if(retv == -1){
        perror("Fork failure");
        close_pipe(pipefd1);
        close_pipe(pipefd2);
        return EXIT_FAILURE;
    }else if(retv == 0){
#ifdef DEBUG
        printf(">> %d << Started process\n",getpid());
#endif
        close(pipefd2[1]);
        close_pipe(pipefd1);
        read_pipe(pipefd2[0],stderr);
        return EXIT_SUCCESS;
    }

    int socketd;
    int ret = establish_connection(&socketd,argv[1],argv[2]);
    if(ret == -1){
        close_pipe(pipefd1);
        close_pipe(pipefd2);
#ifdef DEBUG
        printf(">> %d << Terminating client\n",getpid());
#endif
        return EXIT_FAILURE;
    }


    char buf[64] = { 0 };
    int indeks = 0;
    while(indeks != 4){
        if(interrupted){
            send_error_terminate(socketd);
            break;
        }
        sleep(1);
        print_meny();
        fgets(buf,4,stdin);
        indeks = atoi(buf);
        if(indeks == 1){
            int retv = get_and_do_job(pipefd1[1],pipefd2[1],socketd);
            if(retv == -1){
                break;
            }
        }else if(indeks == 2){
            int length;
            printf("Skriv inn antall jobber du vil hente: ");
            fgets(buf,64,stdin);
            length = atoi(buf);
            int retv = 1;
            for(int i = 0; (i < length)&&(retv == 1)&&(!interrupted); i++){
                retv = get_and_do_job(pipefd1[1],pipefd2[1],socketd);
            }
            if(retv == -1){
                break;
            }
        }else if(indeks == 3){
            int retv = 1;
            while((retv == 1) && !interrupted){
                retv = get_and_do_job(pipefd1[1],pipefd2[1],socketd);
            }
            if(retv == -1){
                break;
            }
        }else if(indeks != 4){
            printf("Velg hva du vil gjore med indeks[1-4]\n");
        }
    }
    if(indeks == 4){
        send_normal_terminate(socketd);
    }


#ifdef DEBUG
    printf(">> %d << Terminating client\n",getpid());
#endif
    close(socketd);
    close_pipe(pipefd1);
    close_pipe(pipefd2);
}
