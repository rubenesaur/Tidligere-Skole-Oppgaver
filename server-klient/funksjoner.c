#include "funksjoner.h"

/*
  Denne funksjonen gjor en string-representasjon av en port
  om til en unsigned short

  Input:
  portstr: en string-representasjon av en port
  port: en peker til en unsigned short hvor porten skal lagres

  Return:
  returnerer 0 ved feil, f.eks at stringen ikke kan tolkes som
  en tallverdi, eller hvis verdien er for stor for en unsigned short.
  returner 1 ved suksess
 */
int get_port(char* portstr,unsigned short* port){
    char *endptr;
    long result;

    errno = 0;
    result = strtol(portstr,&endptr,10);
    if(errno){
        perror("Strtol error");
        return 0;
    }
    if(result > USHRT_MAX || result < 0){
        return 0;
    }
    *port = (unsigned short) result;
    return 1;
}

/*
  Denne funksjonen "bygger" en struct sockaddr_in, ved aa sette
  addresse familien, addressen og port inn i structen.

  Input:
  servaddr: En peker til structen som skal "bygges".
  ip: String-representasjon av addressen som skal settes
  inn i structen. Dersom denne er NULL settes addressen til
  INADDR_ANY.
  portstr: String-representasjon av porten som skal settes
  inn i structen.

  Return:
  Funksjonen returnerer 0 ved feil, f.eks. hvis addressen eller porten
  ikke er gyldig. Returnerer 1 hvis alt gikk bra
 */
int set_addr(struct sockaddr_in* servaddr, char* ip, char* portstr){
    unsigned short port;
    int ret;
    memset(servaddr,0,sizeof(struct sockaddr_in));
    (*servaddr).sin_family = AF_INET;
    if(ip != NULL){
        ret = inet_pton(AF_INET,ip,&(*servaddr).sin_addr);
        if(ret < 1){
            fprintf(stderr,"Address not valid\n");
            return 0;
        }
    }else{
        (*servaddr).sin_addr.s_addr = INADDR_ANY;
    }
    ret = get_port(portstr,&port);
    if(!ret){
        fprintf(stderr,"Could not get port, make sure it is in range[0-%d]",USHRT_MAX);
        return 0;
    }
    (*servaddr).sin_port = htons(port);
    return 1;
}

/*
  Denne funksjonen kalkulerer en checksum, som i dette tilfellet
  er summen av alle karakterene i en string, og deretter modulo 32
  paa denne summen.

  Input:
  msg: En string som skal summeres.
  length: Lengden av stringen msg.

  Return:
  Funksjonen returnerer summen av alle karakterene i msg modulo 32.
  Den vil derfor returnere en unsigned char med verdi mellom 0-31.
 */
unsigned char calc_checksum(char* msg, unsigned int length){
    unsigned int sum = 0;
    for(unsigned int i = 0; i < length; i++){
        sum += msg[i];
    }
    return (unsigned char)(sum % 32);
}
