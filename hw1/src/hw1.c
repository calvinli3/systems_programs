#include "hw1.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
unsigned short validargs(int argc, char **argv) {
    unsigned short mode = 0;
    char* ptr_flags = *(argv+1);
    char current_char = *(ptr_flags+1);
    int opf_return = 0;

    if (current_char == 'h')
        return 0x8000;
    if (argc < 3)
        return 0;

    char* arg2 = *(argv+2);
    char arg2_flag = *(arg2+1);

    if (checkflags(arg2)) {        //check third argument
        if (arg2_flag == 'd') {
            mode = mode | 0x2000;
        } else if (arg2_flag == 'e') {
            //do nothing
        } else {
            return 0;
        }
    }else{
        return 0;
    }

    if (checkflags(ptr_flags)) {        //check second argument of CLI
        if (current_char == 'p') { // can be any # of flags [3-9]
            if (argc < 4) { //no optional args provided
            } else if (argc == 5) { // 1 optional arg, 2 additional elements
                ptr_flags = *(argv+3);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 4, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }
            } else if (argc == 7) { // 2 optional args, 4 additional elements
                ptr_flags = *(argv+3);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 4, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }

                ptr_flags = *(argv+5);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 6, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }
            } else if (argc == 9) { // 3 optional args, 6 additional elements
                ptr_flags = *(argv+3);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 4, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }

                ptr_flags = *(argv+5);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 6, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }

                ptr_flags = *(argv+7);
                current_char = *(ptr_flags+1);
                opf_return = optionalflag(current_char, ptr_flags, argv, mode, 8, 1);
                if(opf_return == 0) {
                    return 0;
                } else if (opf_return >= 900 && opf_return <= 1500) {
                    mode = mode | ((opf_return/100) << 4);
                } else if (opf_return > 8 && opf_return < 16) {
                    mode = mode | (opf_return << 0);
                } else if (opf_return != -1) {
                    return 0;
                }
            } else { //uneven # of args, must be an error somewhere
                return 0;
            }
            // since p flag was given, check for rows and columns to be set. if not, default to 10.
            int row_result = 0;
            int col_result = 0;
            row_result = ((mode & 0x00F0)/16); //check rows
            if (row_result == 0) {
                mode = mode | 0x00A0;
                row_result = 10;
            }
            col_result = mode & 0x000F; //check cols
            if (col_result == 0) {
                mode = mode | 0x000A;
                col_result = 10;
            }
            // ensure that row * col >= length of polybius alphabet
            // get length of polybius alphabet
            int poly_length = 0;
            char* poly_ptr = polybius_alphabet;
            while (*poly_ptr != '\0') {
                poly_length++;
                poly_ptr = (polybius_alphabet+poly_length);
            }
            if ((row_result * col_result) < poly_length) {
                return 0;
            }
        } else if (current_char == 'f') {
            if (argc > 5) {
                return 0;
            }
            mode = mode | 0x4000; //set 14th bit to 1 for -f
            // check if # args = 5 (bin, f, e, k, KEY)
            if (argc == 5) {
                // then check if flag is k
                ptr_flags = *(argv+3);
                current_char = *(ptr_flags+1);  //get position where k "would" be
                if(optionalflag(current_char, ptr_flags, argv, mode, 4, 0) != -1) {
                //if it doesnt return a successful key, error found in program
                    return 0;
                }
            }
        } else {
            return 0;
        }
    }
    return mode;
}

int checkflags(char* args) {
    char currentflag = *args;
    if (currentflag != '-') {
        return 0;
    }
    currentflag = *(args + 2);

    if (currentflag == '\0') {
        return 1;
    }
    return 0;
}

int parsekey(char* k, char* alphabet) {
    //if valid key, return 1
    //parse the key against the alphabet
    char* curr = k;
    int cm_counter = 1;
    int curr_counter = 0;

    while (*curr != '\0') {
        char cmpr = *(curr+cm_counter);
        while (cmpr != '\0') {
            if (*curr == cmpr) {
                return 0;       //invalid key if a substring exists
            }
            cm_counter++;
            cmpr = *(curr+cm_counter);
        }
        cm_counter = 1;
        curr_counter++;
        curr = k+curr_counter;
    }
    //substring checking is done
    //now do check key against alphabet
    char* k_alph = k;
    char* alph_ptr = alphabet;
    int kal_counter = 0;
    int alph_counter = 0;
    int matchfound = 0;     //flip to 1 if match is found. flip back to 0 when checking next keychar

    while (*k_alph != '\0') {
        while (*alph_ptr != '\0') {
            if (*k_alph == *alph_ptr) {
                matchfound = 1;
            }
            alph_counter++;
            alph_ptr = alphabet+alph_counter;
        }
        if (matchfound == 0) {
            return 0;
        } else{
            matchfound = 0;
            alph_counter = 0;
            alph_ptr = alphabet;
            kal_counter++;
            k_alph = k+kal_counter;
        }
    }
    return 1;
}

//optionalflag - return -1 for successful key, return row * 100 if successful row,
//              return col if successful col, return 0 for failure
// if return value is > 100, divide it by 100 and OR it into the ROW section of mode.
// else if return value is > 0, OR it into the COL section of mode.
int optionalflag(char current_char, char* ptr_flags, char** argv, unsigned short mode,
                 int ptr_flag_offset, int alphabet_flag){
    if (checkflags(ptr_flags)){
        if (current_char == 'k') {
            ptr_flags = *(argv+ptr_flag_offset);
            if (alphabet_flag == 0) {
                if(parsekey(ptr_flags, (char*)fm_alphabet)) {     //parsekey returns 1 if valid
                    key = ptr_flags;
                    return -1;
                } else {
                    return 0;
                }
            } else {
                if(parsekey(ptr_flags, (char*)polybius_alphabet)) {     //parsekey returns 1 if valid
                    key = ptr_flags;
                    return -1;
                } else {
                    return 0;
                }
            }
        } else if (current_char == 'r') {
            //do stuff to mode for r and ROW keys
            ptr_flags = *(argv+ptr_flag_offset);
            int row_int = ((*ptr_flags) - '0');
            if (*(ptr_flags+1) != '\0') {   // if there is a 2nd digit
                row_int = (row_int*10) + (*(ptr_flags+1) - '0');
                if (*(ptr_flags+2) != '\0') {
                    return 0;
                }
            }
            return row_int * 100;
        } else if (current_char == 'c') {
            //do stuff to mode for c and COL keys
            ptr_flags = *(argv+ptr_flag_offset);
            int col_int = ((*ptr_flags) - '0');
            if (*(ptr_flags+1) != '\0') {   // if there is a 2nd digit
                col_int = (col_int*10) + (*(ptr_flags+1) - '0');
                if (*(ptr_flags+2) != '\0') {
                    return 0;
                }
            }
            return col_int;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

//do i need to explicitly fill remaining spaces with '\0's
void polybius_encrypt(int cols) {
    FILE* file_in = stdin;
    char c = fgetc(file_in);
    int tableindex = 0;
    while(c != EOF){
        if (c == '\n' || c == '\t' || c == ' ') {
            fputc(c, stdout);
        } else {
            if (getindex(c) == -1) {
                exit(EXIT_FAILURE);
            }
            tableindex = getindex(c);       // need to get # rows and # cols from mode i guess
            int rowcoord = tableindex/cols+'0';
            if (rowcoord > 57) {
                rowcoord+=7;
            }
            int colcoord = tableindex%cols+'0';
            if (colcoord > 57) {
                colcoord+=7;
            }
            fputc(rowcoord, stdout);                //row coordinate = index/num_cols
            fputc(colcoord, stdout);                //col coordinate = index%num_cols
        }
        c = fgetc(file_in);
    }
}

//do i even need to pass in num of rows...
void polybius_decrypt(int cols) {
    //take (r,c) coordinates pair in stdin
    FILE* file_in = stdin;
    int row = 0;
    int col = 0;
    int tableindex;
    int counter = 0;
    char c = fgetc(file_in);
    char* p_string = polybius_table;

    while (c != EOF) {
        if (c == '\n' || c == '\t' || c == ' ') {
            fputc(c, stdout);
            c = fgetc(file_in);
        } else {
            row = c;
            if ((row - '7') > 9) {
                row = row - 55;
            } else {
                row = row - '0';
            }

            c = fgetc(file_in);
            col = c;
            if ((col - '7') > 9) {
                col = col - 55;
            } else {
                col = col - '0';
            }

            //translate to place in table
            tableindex = ((row * cols) + col+1);
            counter = 0;
            //output corresponding char
            while (counter != tableindex) {
                p_string = (polybius_table+counter);
                counter++;
            }
            fputc(*p_string, stdout);
            c = fgetc(file_in);
        }
    }
}

void make_table(char* alphabet) {
    char* p_string;
    char* fm = (char*)fm_key;
    char* pt = polybius_table;

    if (alphabet == polybius_alphabet) {
        p_string = pt;
    } else {
        p_string = fm;
    }

    if (key != NULL) {      //if key exists
        // put key in first
        char* k_string = (char*)key;
        int counter = 0;
        while (*k_string != '\0') {
            *p_string = *k_string;
            counter++;
            k_string = ((char*)key+counter);
            if (alphabet == polybius_alphabet) {
                p_string = (pt+counter);
            } else {
                p_string = (fm+counter);
            }
        }
        // then add in alphabet, and check for each char of alphabet if its already in poly table
        char* a_string = alphabet;
        k_string = (char*)key;
        int k_counter = 0;
        int a_counter = 0;
        int matchfound = 0;
        while (*a_string != '\0') {  //while there are things to add from the alphabet
            while (*k_string != '\0') {    //while there are key chars to check
                if (*a_string == *k_string) {   //if substring found, go to next alphabet char, reset p_string
                    matchfound = 1;
                }
                k_counter++;
                k_string = ((char*)key+k_counter);
            }
            if (matchfound == 1) {
                // if match is found, reset k string, reset k counter,
                // reset matchfound, go to next alphabet
                k_counter = 0;
                k_string = ((char*)key+k_counter);
                matchfound = 0;
                a_counter++;
                a_string=alphabet+a_counter;
            } else {
                //match not found- reset k counter, string,
                //add in alphabet char to p_string, go to next alphabet, go to next p_string
                k_counter=0;
                k_string= ((char*)key+k_counter);
                *p_string = *a_string;
                counter++;
                a_counter++;
                a_string=alphabet+a_counter;
                if (alphabet == polybius_alphabet) {
                    p_string=pt+counter;
                } else {
                    p_string=fm+counter;
                }
            }
        }
    } else {
        if (alphabet == polybius_table) {
            stringcopy(polybius_table, alphabet);
        } else {
            stringcopy(fm, alphabet);
        }
    }
}

int getindex(char c) {
    int index = 0;
    char* p_table = polybius_table;
    while (*p_table!='\0') {
        if (c == *p_table) {
            return index;
        }
        index++;
        p_table = (polybius_table+index);
    }
    return -1;
}

void stringcopy(char* targetstring, char* origstring) {
    char* t_string = targetstring;
    char* o_string = origstring;
    int counter = 0;
    while (*o_string != '\0') {
        *t_string = *o_string;
        counter++;
        o_string = (origstring+counter);
        t_string = (targetstring+counter);
    }
}

void fm_encrypt() {
    char* buffer = (char*)&space;
    int buff_counter = 0;
    FILE* file_in = stdin;
    char c = fgetc(file_in);
    int tableindex = 0;
    char* morse_string;
    char morse_char;
    int char_counter = 0;
    char output;

    while (c != EOF) {
        if (c == '\n' || c == '\t' || c == ' ') {
             //add x to buffer
            *(buffer+buff_counter) = 'x';
            buff_counter++;
            //check if buffer == 3, translate, reset buff_counter, reset buffer
            if (buff_counter == 3) {
                output = match_string(buffer, (char**)fractionated_table);
                fputc(output, stdout);
                buff_counter = 0;
            }
            while ((c == '\n' || c == '\t' || c == ' ') && c != EOF){
                if (c == '\n'){
                    buff_counter = 0;
                    fputc(c, stdout);
                }
                c = fgetc(file_in);
            }
        }
        // translate char c to morse
        // c - '!' = index in morse_table
        tableindex = c - '!';       // get correct morse_table index from char c
        morse_string = *((char**)morse_table+tableindex);      // get the morse string at the index
        if (stringcompare(morse_string, "")) {
            exit(EXIT_FAILURE);
        }
        char_counter = 0;
        morse_char = *(morse_string+char_counter);
        // while the char of morse string != '\0'
        while(morse_char != '\0') {
            // add into buffer, mor_counter++
            *(buffer+buff_counter) = morse_char;   //move the morse into the buffer
            buff_counter++;
            // if buffer == 3, translate; match the 3 morse codes into the fm_key
            if (buff_counter == 3) {
                output = match_string(buffer, (char**)fractionated_table);
                fputc(output, stdout);
                buff_counter = 0;
            }
            char_counter++;
            morse_char = *(morse_string+char_counter);
        }
        // after each character, add an x to buffer
        *(buffer+buff_counter) = 'x';
        buff_counter++;
        if (buff_counter == 3) {
                output = match_string(buffer, (char**)fractionated_table);
                fputc(output, stdout);
                buff_counter = 0;
            }
        c = fgetc(file_in);
    }
}

void fm_decrypt() {
    char* buffer = (char*)&space;
    int buff_counter = 0;
    FILE* file_in = stdin;
    char c = fgetc(file_in);
    int tableindex = 0;
    char* morse_string;
    char morse_char;
    int char_counter = 0;
    char output;
    char* fm_key_ptr = fm_key;
    int x_found = 0;

    while (c != EOF) {
        // if x indicator == 2
        //Account for newline
        if (c == '\n') {
            if (stringcompare(buffer, "") != 1) {
                output = match_string(buffer, (char**)morse_table);
                fputc(output, stdout);
                buff_counter = 0;
                clear_buffer();
            }
            fputc('\n', stdout);
            x_found = 0;
        } else {
            fm_key_ptr = fm_key;
            tableindex = 0;
            // match c to fm_key to get tableindex
            while (c != *fm_key_ptr) {
                tableindex++;
                fm_key_ptr = (fm_key+tableindex);
            }
            // translate tableindex of frac_table, get morse_string
            morse_string = *((char**)fractionated_table+tableindex);
            if (stringcompare(morse_string, "")) {
                exit(EXIT_FAILURE);
            }
            char_counter = 0;
            morse_char = *(morse_string+char_counter);
            // while the char of morse string != '\0'
            while(morse_char != '\0') {
                if (x_found == 2) {
                    fputc(' ', stdout);
                    x_found = 0;
                    buff_counter = 0;
                    clear_buffer();
                }
                if (morse_char == 'x') {        //signal end of morse_string. decode buffer
                    if (x_found == 1) {         //if this is the 2nd x found, put space
                        // if (*(morse_string+char_counter+1) == '\n') {
                        //     fputc('\n', stdout);
                        // } else {
                        //     fputc(' ', stdout);
                        //     x_found = 0;
                        // }
                        /*
                                chg x indicators to [0-2]

                        */
                        x_found++;
                    } else {                    //if this is 1st x found, decrypt buffer
                        output = match_string(buffer, (char**)morse_table);
                        fputc(output, stdout);
                        buff_counter = 0;
                        clear_buffer();
                        x_found++;
                    }
                } else {
                    // add into buffer, mor_counter++
                    x_found = 0;
                    *(buffer+buff_counter) = morse_char;   //move the morse into the buffer
                    buff_counter++;
                }
                char_counter++;
                morse_char = *(morse_string+char_counter);
            }
        }
        c = fgetc(file_in);
    }
}

char match_string(char* buffer, char** table){

    char* f_string;
    if (table == (char**)fractionated_table){   //encrypt
        f_string = *(char**)fractionated_table;
    } else {                                    //decrypt
        f_string = *(char**)morse_table;
    }
    int f_index = 0;
    char* buff = buffer;

    while (f_string != NULL) {
        if (stringcompare(buff, f_string)) {    //true if they match
            // return corresponding char of fm_alphabet
            if (table == (char**)fractionated_table){   //encrypt
                return (*(fm_key+f_index));
            } else {                                    //decrypt
                return (f_index + '!');
            }
        } else {
            // didn't match; get next f_string
            f_index++;
            f_string = *(table+f_index);
        }

    }
    return '\0';
}

int stringcompare(char* str1, char* str2) {
    char s1 = *str1;
    char s2 = *str2;
    int counter = 0;

    while (s1 != '\0') {
        if (s1 != s2) {
            return 0;
        } else {
            counter++;
            s1 = *(str1+counter);
            s2 = *(str2+counter);
        }
    }


    if (s2 == '\0') {
        return 1;
    } else {
        return 0;
    }
}

void clear_buffer() {
    space = 0;
}