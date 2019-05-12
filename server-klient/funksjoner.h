#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <errno.h>
#include <limits.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int get_port(char*,unsigned short*);
int set_addr(struct sockaddr_in*,char*,char*);
unsigned char calc_checksum(char*,unsigned int);
