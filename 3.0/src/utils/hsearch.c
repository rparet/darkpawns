/* hsearch.c - House searching */
#include "../conf.h"
#include "../sysdep.h"
    
#include "../structs.h"
#include "../utils.h"


int main (int argc, char *argv[])
{ 
  struct obj_file_elem object;
  FILE *fl;

  if (argc != 2)
  {
    printf("Usage: %s <house file>\n", argv[0]);
    return(1);
  }

  if (!(fl = fopen(argv[1], "rb")))
  {
    printf("Can't open %s\r\n", argv[1]);
    return(1);
  }

/*  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
*/
  
  while (!feof(fl)) 
  {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) 
    {
      printf("-E- ferror \r\n");
      fclose(fl);
      return(1);
    }
    if (!feof(fl))
    {
      printf("[%d]\t%s\r\n", object.item_number, object.name);
      printf("%s\t%s\r\n", object.shortd, object.desc);
      printf("Loc: %d Weight: %d Timer: %d \r\n", object.locate, object.weight, object.timer);
      printf("Vals: [%d] [%d] [%d] [%d] [%d]\r\n", object.value[0], object.value[1], object.value[2], 
             object.value[3], object.value[4]);
      printf("\r\n");
    }
  }
  fclose(fl);
  return(1);
}  

