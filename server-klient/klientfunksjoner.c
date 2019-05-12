#include "klientfunksjoner.h"


/*
  Denne funksjonen closer en pipe i read end
  og write end.

  Input:
  pipefd[]: Pipe filedescriptoren som skal lukkes
*/
void close_pipe(int pipefd[]){
    close(pipefd[0]);
    close(pipefd[1]);
}

/*
  Denne funksjonen printer en meny til stdout,
  brukes til ordrelokken i dette programmet
*/
void print_meny(){
    printf("(1) Hent én jobb fra serveren\n(2) Hent X antall jobber fra serveren\n");
    printf("(3) Hent alle jobber fra serveren\n(4) Avslutt programmet\n");
}

/*
  Denne funksjonen skal lese fra socket og legge verdiene den leser inn i variable
  den faar fra parametere. Leser forst jobinfo,deretter length ogsaa msg. Hvis ingen
  av disse feilet saa vil metoden deretter hente ut checksum verdien fra jobinfo
  karakteren, og sammenligne denne med det den regner ut selv. Til slutt vil
  jobinfo karakteren shiftes 5 ganger til hoyre, ettersom checksummen ikke trengs etter
  dette.

  Input:
  sockd: Fildeskriptor for socket som det skal leses fra
  jobinfo: peker til unsigned char som det skal lagres verdi i lest fra socket
  length: peker til unsigned int som det skal lagres verdi i lest fra socket
  msg: peker til string som det skal lagres verdi i ilest fra socket

  Return:
  Denne metoden returnerer -1 dersom det skjer en feil, den vil returnere 1 dersom
  checksum ikke stemmer dette er ikke en stor feil saa programmet kan forstatt kjore.
  Den returnerer 0 dersom alt gikk bra, eller ved slutt paa lesing ved at f.eks socket
  er lukket, men da vil variablene ha verdier som tilsvarer at dette programmet skal
  avsluttes ogsaa.
*/
int recieve_serv_msg(int sockd,unsigned char* jobinfo,unsigned int* length, char** msg){
    ssize_t bytes;
    if((bytes=read(sockd,jobinfo,sizeof(unsigned char))) <= 0){
        if(bytes == -1){
            perror("Socket read error");
            return -1;
        }
        *jobinfo = 6;
        return 0;
    }
    if((bytes=read(sockd,length,sizeof(unsigned int))) <= 0){
        if(bytes == -1){
            perror("Socket read error");
            return -1;
        }
        return 0;
    }
    *msg = malloc((*length)*sizeof(char));
    if((bytes=read(sockd,*msg,*length)) <= 0){
        if(bytes == -1){
            perror("Socket read error");
            free(*msg);
            return -1;
        }
        return 0;
    }
    unsigned char old_checksum = ((1<<5)-1) & *jobinfo;
    unsigned char new_checksum = calc_checksum(*msg,*length);
    if(old_checksum != new_checksum){
        fprintf(stderr,"Checksum error\n");
        free(*msg);
        return 1;
    }
    *jobinfo = (*jobinfo)>>5;
#ifdef DEBUG
    printf(">> %d << Recieved job from server\n",getpid());
#endif
    return 0;
}

/*
  Denne metoden skal skrive en melding/jobb til pipe, som barneprossessene i dette
  programmet skal gjore. Den skriver forst lengden av meldingen, deretter
  skriver den selve jobteksten

  Input:
  pfd: Fildeskriptor for pipe det skal skrives til
  length: lengden av meldingen som skal sendes
  msg: peker til meldingen som skal skrives til pipe

  Return:
  Denne metoden returnerer -1 dersom det skjer feil i
  en av pipeskrivingene, og 0 dersom alt gikk bra.
*/
int write_pipe(int pfd, int length, char** msg){
    ssize_t bytes;

    if((bytes = write(pfd,&length,sizeof(length))) == -1){
        perror("Pipe writing failure");
        return -1;
    }
    if((bytes = write(pfd,*msg,length)) == -1){
        perror("Pipe writing failure");
        return -1;
    }
    return 0;
}

/*
  Denne metoden blir kalt av barneprossessene som skal lese fra en pipe. Her
  er det en lokke som prosessene blir vaerende i helt til pipen de leser fra
  lukkes. De leser forst lengden av meldingen, som da sier hvor lang neste
  lesing skal vaere. Etter at meldingen er lest skriver den meldingen til
  den outputfilen de har faatt tildelt.

  Input:
  pfd: En fildeskriptor som sier hvilken pipe det skal leses fra
  outputfil: FILE* som sier hvilken fil det skal skrives ut til(f.eks stderr eller stdout).
*/
void read_pipe(int pfd,FILE* outputfil){
    unsigned int length = 0;
    ssize_t bytes;
#ifdef DEBUG
    printf(">> %d << Reading pipe for messages\n",getpid());
#endif
    while((bytes = read(pfd,&length,4)) != 0){
        if(bytes == -1){
            perror("Pipe reading failure");
            break;
        }

        unsigned char* msg = malloc(length*sizeof(char) + 1);
        if((bytes = read(pfd,msg,length)) == -1){
            perror("Pipe reading failure");
            free(msg);
            break;
        }
#ifdef DEBUG
        printf(">> %d << Recieved message\n",getpid());
#endif
        msg[bytes] = '\0';
        fprintf(outputfil,"%s\n",msg);
        free(msg);
    }

    close(pfd);
#ifdef DEBUG
    printf(">> %d << Terminating\n",getpid());
#endif
}


/*
  Denne metoden skal prove aa connnecte til en socket og bruker blant annet
  metoden set_addr for aa bygge en sockaddr_in struct ut fra ip-addresse
  og port gitt som parametere. Bruker systemkallene socket og connect
  for aa faa til en connection.

  Input:
  socketd: En peker til en int som skal faa en fildeskriptor fra connect-kallet
  servIP: en string representasjon av addressen som det skal connectes til
  port: en string representasjon av porten paa addressen man skal connectes til

  Return:
  Metoden returnerer -1 hvis det skjer feil i set_addr metoden eller i
  noen av systemkallene. Returnerer 0 dersom alt gikk bra.
*/
int establish_connection(int* socketd,char* servIP,char* port){
    struct sockaddr_in serveraddr;

    int ret = set_addr(&serveraddr,servIP,port);
    if(!ret){
        return -1;
    }

    *socketd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(*socketd == -1){
        perror("Socket creation failure");
        return -1;
    }

    ret = connect(*socketd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr_in));
    if(ret == -1){
        close(*socketd);
        perror("Socket connection failure");
        return -1;
    }
#ifdef DEBUG
    printf(">> %d << Established connection at address[%s] and port[%s]\n",getpid(),servIP,port);
#endif

    return 0;
}

/*
  Denne metoden skal skrive en melding til socket om at programmet terminerer paa
  grunn av en feil, her er denne meldingen "-1".

  Input:
  socketd: socket som feilmeldingen skal skrives til
*/
void send_error_terminate(int socketd){
    int terminatemsg = -1;
    if(write(socketd,&terminatemsg,sizeof(int)) == -1){
        perror("Socket write error");
    }
}

/*
  Denne metoden skal skrive en melding til socket om at programmet
  terminerer normalt, her er denne meldingen "1".

  Input:
  socketd: socket som meldingen skal skrives til
*/
void send_normal_terminate(int socketd){
    int buffer = 0;
    if(write(socketd,&buffer,sizeof(int)) == -1){
        perror("Socket write error");
    }
}

/*
  Denne metoden skal be om jobb fra server via socket og motta jobben
  ved bruk av metoden recieve_serv_msg. Deretter skal den bestemme
  hvilken pipe jobben/meldingen skal sendes til og gjore dette. Hvis
  den faar melding om at server avslutter skal den sende en melding
  om at denne gjor det ogsa og programmet skal avslutte. Dersom det
  skjer feil i noen av write/read kallene prover metoden aa sende
  melding til socket om at denne avslutter pga. feil og programmet vil
  avslutte.

  Input:
  pipefd1: Fildeskriptor for en pipe
  pipefd2: Fildeskriptor for en pipe
  socketd: Fildeskriptor for en socket

  Return:
  Returnerer -1 ved feil eller hvis metoder som kaller paa denne skal
  avslutte, og returner 1 hvis alt gikk bra i lesing og sending av jobber
*/
int get_and_do_job(int pipefd1,int pipefd2,int socketd){
    unsigned char jobinfo;
    unsigned  int joblength;
    char* jobtext = { 0 };
    int ret;

    int msg = 1;
    if(write(socketd,&msg,sizeof(int)) == -1){
        perror("Socket write error");
        return -1;
    }
    if((ret=recieve_serv_msg(socketd,&jobinfo,&joblength,&jobtext)) == 1){
        fprintf(stderr,"Can't do job\n");
        return 1;
    }else if(ret == -1){
        send_error_terminate(socketd);
        return -1;
    }
    if(jobinfo == 0){
        if(write_pipe(pipefd1,joblength,&jobtext) == -1){
            send_error_terminate(socketd);
            return -1;
        }
#ifdef DEBUG
        printf(">> %d << Sent message for child to print at stdout\n",getpid());
#endif
    }else if(jobinfo == 1){
        if(write_pipe(pipefd2,joblength,&jobtext) == -1){
            send_error_terminate(socketd);
            return -1;
        }
#ifdef DEBUG
        printf(">> %d << Sent message for child to print at stderr\n",getpid());
#endif
    }else{
#ifdef DEBUG
        printf(">> %d << Recieved message that server is terminating\n",getpid());
#endif
        free(jobtext);
        send_normal_terminate(socketd);
        printf(" -- Exiting -- \n");
        return -1;
    }
    free(jobtext);
    return 1;
}
