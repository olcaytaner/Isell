#include <stdio.h>
#include <time.h>
#include "command.h"
#include "datastructure.h"
#include "image.h"
#include "isell.h"
#include "lang.h"
#include "libmemory.h"
#include "librandom.h"
#include "messages.h"
#include "parameter.h"
#include "options.h"
#include "process.h"
#include "typedef.h"

#ifdef MPI
#include "mpi.h"
#endif

extern Stackptr ipstack;
extern FILE *output;
extern Command_nodeptr commandtree;
extern variableptr vars;
extern int varcount;
extern int availableproccount;
extern int rank;
extern char* order;

int main(int argc, char *argv[])
{
 FILE *myfile;
 int res;
 time_t t;
 ipstack = stack();
 mysrand(time(&t));
 output = stdout;
 printf(m1000);
 initialize_image();
 initialize_orders();
 commandtree = generate_command_tree();
 read_program_parameters();

#ifdef MPI
 MPI_Init(&argc, &argv);
 MPI_Comm_size(MPI_COMM_WORLD, &availableproccount);
 printf("Processor: %d\n", availableproccount);
 MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

 add_default_variables(&vars, &varcount);
 myfile = fopen("start.txt", "r");
 if (myfile)
  {
   fclose(myfile);
   res = compile_file("start.txt");
   if (res)
     load_file("start.txt");
  }
 if (argc > 1)
  {
   res = compile_file(argv[1]);
   if (res)
     load_file(argv[1]);
  }
 getorder();
 while ((res = get_order_index(order)) != 0)
  {
   process_order(res);
   getorder();
  }
 if (output != stdout)
   fclose(output);
 free_program_parameters();
 deallocate_command_tree(commandtree);
 deallocate_remaining_memory();
 free_stack(ipstack);
 printf(m1001);

#ifdef HEAP_DEBUG
 heap_debug_print_heap();
#endif

#ifdef MPI
 MPI_Finalize();
#endif
 return 1;
}
