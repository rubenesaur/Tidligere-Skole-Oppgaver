#include "serverfunksjoner.h"

/*
  Denne funksjonen bruker systemkallene: socket,bind,listen og accept for
  aa lage en connection til klient. Bruker kall paa set_addr som er en
  funksjon jeg har laget selv som "bygger" en sockaddr_in struct. Det
  settes opp en request socket som som bindes og tar seg av innkommende
  connection requests. Til slutt lagres socketfildescriptoren som returneres
  ved godkjent accept i en int som sendes inn som parameter, og request
  socketen lukkes, slik at det ikke blir mer enn en connection.

  Input:
  sockd: en peker til en int hvor den "acceptede" tilkoblingen skal lagres
  port: en string representasjon av porten som socket skal aapnes paa

  Return:
  Metoden returnerer -1 dersom den feiler med noen av systemkallene, og
  returnerer 0 dersom alt gikk bra og det er kommet inn en connection. 
 */
int establish_connection(int* sockd,char* port){
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(struct sockaddr_in);
    int request_sockd;
    int ret = set_addr(&serveraddr,NULL,port);
    if(!ret){
        return -1;
    }
    request_sockd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(request_sockd == -1){
        perror("Socket creation failure");
        return -1;
    }
#ifdef DEBUG
    printf(">> %d << Succesfully created a request socket\n",getpid());
#endif

    ret = bind(request_sockd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr_in));
    if(ret == -1){
        close(request_sockd);
        perror("Socket bind failure");
        return -1;
    }
#ifdef DEBUG
    printf(">> %d << Succesfull bind to request socket at port [%s]\n",getpid(),port);
#endif

    ret = listen(request_sockd,1);
    if(ret == -1){
        close(request_sockd);
        perror("Socket listen failure");
        return -1;
    }
#ifdef DEBUG
    printf(">> %d << Started listening at request socket succesfully\n",getpid());
#endif


    *sockd = accept(request_sockd,(struct sockaddr*)&clientaddr,&clientaddrlen);
    if(*sockd == -1){
        close(request_sockd);
        perror("Socket accept failure");
        return -1;
    }

    close(request_sockd);
#ifdef DEBUG
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&(clientaddr.sin_addr),buf,INET_ADDRSTRLEN);
    printf(">> %d << Accepted connection request succesfully from address [%s]\n",getpid(),buf);
    printf(">> %d << Closed request socket to prevent more connections\n",getpid());
#endif
    return 0;

}

/*
  Denne funksjonen konverterer en char til en tre-bits representasjon av denne
  karakteren, oppgaven sier hvordan de skal vaere. Konverterer 'O' til 000,
  'E' til 001 og 'Q' til 111. Lagrer den nye verdien i den char'en som blir sendt
  inn.

  Input:
  jobt: En peker til unsigned char som skal konverteres, den nye verdien lagres ogsaa her
*/
void jobtype_char_to_bit_convert(unsigned char* jobt){
    unsigned char jobtype = *jobt;
    jobtype = ((~jobtype&8) | ((jobtype&16)|(jobtype&32)<<1) | ((jobtype&16)|(jobtype&32))) >>3;
    *jobt = jobtype;
}

/*
  Denne funksjonen konstruerer jobinfo karakteren som skal sendes til klienten. Den kalkulerer
  checksummen som skal vaere 5 av de 8 bitene i karakteren, deretter brukes jobtype_char_to_bit_convert
  for aa faa en 3bits representasjon av jobtype karakteren. Disse settes sammen slik at de 3-bitene
  for jobtypen kommer helt til venstre i karakteren, mens de resterende 5 er for checksum.

  Input:
  jobtype: Karakter som beskriver jobtypen til jobben(Q,E eller O)
  textlen: Heltall som beskriver lengden til teksten(brukes for aa regne checksum)
  jobtext: Teksten til jobben(brukes for aa regne checksum)

  Return:
  Funksjonen returnerer karakteren som har blitt konstruert, karakteren bestaar av 3 bit som
  beskriver jobtypen og 5 bit for checksummen.
*/
unsigned char construct_jobinfo(unsigned char jobtype,unsigned int textlen,char jobtext[]){
    unsigned char jobinfo = calc_checksum(jobtext,textlen);
    jobtype_char_to_bit_convert(&jobtype);
    jobinfo = (jobtype<<5) | jobinfo;
    return jobinfo;
}

/*
  Denne funksjonen sender en "quit-msg" til klienten, eller til socket.
  Denne meldingen skal bestaa av en jobinfo char med bitmonsteret til
  'Q' og skal ha 0 i checksum samt 0 i tekstlengde. Konstruerer da disse
  meldingene og writer til socket. Printer error dersom den faar det.

  Input:
  sockd: En fildeskriptor til en socket som det skal sendes melding til
*/
void send_quit_msg(int sockd){
    unsigned int textlen = 0;
    unsigned char jobinfo = construct_jobinfo('Q',textlen,NULL);
#ifdef DEBUG
    printf(">> %d << Sending message for client to terminate\n",getpid());
#endif
    if(write(sockd,&jobinfo,sizeof(unsigned char)) == -1){
        perror("Socket write error");
    }

    if(write(sockd,&textlen,sizeof(unsigned int)) == -1){
        perror("Socket write error");
    }

}

/*
  Denne funksjonen leser jobber fra fil, og lagrer jobtype, tekstlengdde og
  tekst i variable som sendes inn via parameter. Bruker blant annet metoden
  construct_jobinfo for aa lage en jobinfo karakter som "inneholder" checksum
  og bitmonster for jobtypen. Derfor vil verdiene i de variable som sendes inn
  vaere riktig og klare for aa sendes til socket, dersom denne metoden ikke feiler

  Input:
  jobfile: filen som jobber skal leses fra
  jobtype: peker til en unsigned char som det skal lagres verdi i
  textlen: peker til en unsigned int som det skal lagres verdi i
  jobtext: peker til en string som det skal lagres verdi i

  Return:
  Denne metoden returnerer -1 dersom det skjer feil, hovedsakelig fillessingsfeil.
  Returnerer 1 dersom alt gikk riktig, og verdiene skal da inneholde riktige verdier.
 */
int get_job(FILE* jobfile,unsigned char* jobtype,unsigned int* textlen,char** jobtext){
    size_t bread = fread(jobtype,1,1,jobfile);
    if(bread < 1){
        if(ferror(jobfile)){
            fprintf(stderr,"Error reading file");
        }
        return -1;
    }

    bread = fread(textlen,4,1,jobfile);
    if(bread < 1){
        if(ferror(jobfile)){
            fprintf(stderr,"Error reading file");
        }
        return -1;
    }

    *jobtext = malloc((*textlen)*sizeof(char));
    bread = fread(*jobtext,1,*textlen,jobfile);
    if(bread < *textlen){
        if(ferror(jobfile)){
            fprintf(stderr,"Error reading file");
        }
        free(*jobtext);
        return -1;
    }
    *jobtype = construct_jobinfo(*jobtype,*textlen,*jobtext);
    return 1;
}

/*
  Denne metoden bruker kall paa get_job for aa hente en jobb og
  vil deretter skrive denne jobben til socket. Dersom get_job feiler
  vil metoden i stedet kalle paa send_quit_msg som vil si ifra til den som
  leser paa socket at den skal avslutte.

  Input:
  sockd: Socketen som meldinger skal skrives til
  jobfile: Filen som jobber skal hentes fra.
 */
void send_msg(int sockd,FILE* jobfile){
    unsigned char jobinfo;
    unsigned int textlen;
    char* jobtext = { 0 };
    int ret = get_job(jobfile,&jobinfo,&textlen,&jobtext);
    if(ret == -1){
        send_quit_msg(sockd);
    }else{
#ifdef DEBUG
        printf(">> %d << Sending normal job to client\n",getpid());
#endif
        if(write(sockd,&jobinfo,sizeof(unsigned char)) == -1){
            perror("Socket write error");
        }
        if(write(sockd,&textlen,sizeof(unsigned int)) == -1){
            perror("Socket write error");
        }
        if(write(sockd,jobtext,textlen) == -1){
            perror("Socket write error");
        }
        free(jobtext);
    }
}
