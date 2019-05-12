#include "funksjoner.h"

void close_pipe(int[]);
void print_meny();
int recieve_serv_msg(int,unsigned char*,unsigned int*,char**);
int write_pipe(int,int,char**);
void read_pipe(int,FILE*);
int establish_connection(int*,char*,char*);
void send_error_terminate(int);
void send_normal_terminate(int);
int get_and_do_job(int,int,int);

