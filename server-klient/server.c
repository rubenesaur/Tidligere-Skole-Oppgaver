#include "serverfunksjoner.h"

int interrupted = 0;



/*
  Setter en global int til 1 dersom programmet fanger opp et
  SIGINT signal. 
 */
void interrupt_handler(){
    interrupted = 1;
#ifdef DEBUG
    printf(">> %d << Recieved interrupt signal.\n",getpid());
#endif
}


/*
  Denne metoden aapner fil som sendes inn via argument naar
  programmet kjores, deretter kaller den paa establish_connection.
  Hvis begge disse gaar bra vil programmet begynne aa lese paa
  socket, og vil gjore forskjellige handlinger avhengig av hvilken
  melding den leser fra socket. Vil kalle paa metoden send_msg dersom
  den faar melding om det, andre meldinger den kan faa er at klient
  terminerte og at klient terminerte ved feil. I begge disse tilfellene
  vil programmet hoppe ut av loopen og lukke aapne filer og fildeskriptorer.

  Input:
  argc: antall argumenter ved oppstart av programmet(brukes til aa sjekke korrekt bruk)
  argv: String array med argumentene som sendes ved oppstart, argv[1]=jobfil argv[2]=port.

  Return:
  Metoden returnerer EXIT_FAILURE ved feil i filaapning eller metoden establish_connection
 */
int main(int argc, char* argv[]){

    struct sigaction sa;
    memset(&sa,0,sizeof(struct sigaction));
    sa.sa_handler = &interrupt_handler;
    sigaction(SIGINT,&sa,NULL);

    if(argc < 3){
        printf("Usage: %s [job-list] [port-num]",argv[0]);
        return EXIT_SUCCESS;
    }
#ifdef DEBUG
    printf(">> %d << Starting process in DEBUG-mode\n",getpid());
#endif

    int sockd;
    FILE* jobfile = fopen(argv[1],"rb");
    if(jobfile == NULL){
        perror("File open failure");
        return EXIT_FAILURE;
    }
#ifdef DEBUG
    printf(">> %d <<< Successfully opened file: %s\n",getpid(),argv[1]);
    printf(">> %d << Attempting to establish connection\n",getpid());
#endif


    if(establish_connection(&sockd,argv[2]) == -1){
        fclose(jobfile);
        return EXIT_FAILURE;
    }

    ssize_t bytes;
    int buf;
    int sent_quit = 0;

    while((bytes = read(sockd,&buf,sizeof(int))) != 0){
        if(interrupted){
            send_quit_msg(sockd);
            interrupted = 0;
            sent_quit = 1;
            continue;
        }

        if(buf == -1){
#ifdef DEBUG
            printf(">> %d << Recieved message that client terminated because of error\n",getpid());
#endif
            break;
        }
        if(buf == 0){
#ifdef DEBUG
            printf(">> %d << Recieved message that client terminated normally\n",getpid());
#endif
            break;
        }
        if(buf == 1 && !sent_quit){
#ifdef DEBUG
            printf(">> %d << Recieved message to send job\n",getpid());
#endif
            send_msg(sockd,jobfile);
        }

    }

#ifdef DEBUG
    printf(">> %d << Server is terminating\n",getpid());
#endif

    fclose(jobfile);
    close(sockd);
}
