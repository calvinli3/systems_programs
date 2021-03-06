#include "debug.h"
#include "utf.h"
#include "wrappers.h"
// #include <stdlib.h>

int main(int argc, char *argv[]) {
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  lseek(infile, program_state->bom_length, SEEK_SET); /* Discard BOM */
  get_encoding_function()(infile, outfile);

  if(program_state != NULL) {
    close(outfile);
    close(infile);
    free((void*)program_state);
  }
  return EXIT_SUCCESS;
}
