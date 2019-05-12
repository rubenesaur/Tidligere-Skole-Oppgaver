#include "funksjoner.h"

int establish_connection(int*,char*);
void jobtype_char_to_bit_convert(unsigned char*);
unsigned char construct_jobinfo(unsigned char,unsigned int,char[]);
void send_quit_msg(int);
int get_job(FILE*,unsigned char*,unsigned int*,char**);
void send_msg(int,FILE*);
