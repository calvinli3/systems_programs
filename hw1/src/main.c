#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;
    mode = validargs(argc, argv);
    // printf("%x\n", mode );

    if(mode == 0) {
        printf("FAILURE\n");
        USAGE(*argv, EXIT_FAILURE);
    }

    // debug("Mode: 0x%X", mode);
    if(mode == 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
        printf("USAGE\n");
    }

    if(mode & 0x4000) {     // f flag
        // part 3
        // call make_table(fractionated_morse)
        make_table((char*)fm_alphabet);
        if(mode & 0x2000) {     //check for decryption
            fm_decrypt();
        } else {
            fm_encrypt();
        }
    } else {               // p flag
        // call make_table(polybius)
            int cols = (mode & 0x000F);             //get number of cols to pass into the thing
            make_table(polybius_alphabet);
            if(mode & 0x2000) {     //check for decryption
                polybius_decrypt(cols);
            } else {    //encryption
                polybius_encrypt(cols);
            }
    }
    return EXIT_SUCCESS;

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */