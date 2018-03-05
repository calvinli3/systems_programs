#ifndef HW_H
#define HW_H

#include "const.h"

int checkflags(char* args);
int parsekey(char* k, char* alphabet);
int optionalflag(char current_char, char* ptr_flags, char** argv,
                //delete this mode crap later and its references in hw1.c
                 unsigned short mode, int ptr_flag_offset, int alphabet_flag);
void polybius_encrypt(int cols);
void polybius_decrypt(int cols);
void fm_encrypt();
void fm_decrypt();
void make_table(char* alphabet);
int getindex(char c);
void stringcopy(char* targetstring, char* origstring);
char match_string(char* buffer, char** fractionated_table);
long space;
int stringcompare(char* str1, char* str2);
void clear_buffer();

#endif