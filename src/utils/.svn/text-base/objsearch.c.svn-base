#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

#define PFILE	"../lib/etc/players"
#define PREFIX	"plrobjs"
#define SUFFIX	"objs"

char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVIS",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEU",
  "!MAGE",
  "!CLE",
  "!THI",
  "!WAR",
  "!SELL",
  "NAMED",
  "!PSI",
  "!NIN",   
  "!PAL",
  "!MAGUS",
  "!ASS",
  "!AVA",
  "RARE",
  "!LOCATE",
  "!RAN",
  "!MYS",
  "\n"
};

void sprintbitarray(int bitvector[], char *names[], int maxar, char *result) {
   int nr, teller, found = FALSE;

   *result = '\0'; 

   for(teller = 0; teller < maxar && !found; teller++)
      for(nr = 0; nr < 32 && !found; nr++) {
         if(IS_SET_AR(bitvector, (teller*32)+nr))
            if(*names[(teller*32)+nr] != '\n') {
               if(*names[(teller*32)+nr] != '\0') {
                  strcat(result, names[(teller *32)+nr]);
                  strcat(result, " ");
               }
            } else {
               strcat(result, "UNDEFINED ");
            }
         if(*names[(teller*32)+nr] == '\n')
            found = TRUE;   
      }
  
   if(!*result)   
      strcpy(result, "NOBITS ");
}


int get_fname(char *orig_name, char *filename)
{
  char *middle, *ptr, name[64];

  if (!*orig_name)
    return 0;
 
  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);
  
  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z"; 
    break;
  default:
    middle = "ZZZ";
    break;
  }
  
  sprintf(filename, "../lib/%s/%s/%s.%s", PREFIX, middle, name, SUFFIX);
  return 1;
}

void find_obj(char *player_name, int object_vnum)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH];
  struct obj_file_elem object;
  struct rent_info rent;
  char buf[256];    

  if (!get_fname(player_name, fname))
    return;
  if (!(fl = fopen(fname, "rb"))) /* no rent file */
    return;
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {   
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (object.item_number == object_vnum) 
      {
        sprintbitarray(object.extra_flags, extra_bits, EF_ARRAY_MAX, buf);
        printf("%s: [%d] %s %s\r\n", player_name, object.item_number, object.shortd, buf);
      }
  }
  fclose(fl);
}


int main (int argc, char *argv[])
{
  FILE *fl;
  struct char_file_u player;
  long size;

  if (argc != 2)
  {
    printf("Usage: %s <vnum>\n", argv[0]);
    return(1);
  }

  if (!(fl = fopen(PFILE, "r+"))) {
    printf("Can't open %s.", PFILE);
    exit(1);
  }

  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);

  if (size % sizeof(struct char_file_u)) 
  {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile.\r\n");
    fclose(fl);
    exit(1);
  }


  for (;;)
  {
    fread(&player, sizeof(struct char_file_u), 1, fl);

    if (feof(fl)) {
      fclose(fl);
      exit(0);
    }

    find_obj(player.name, atoi(argv[1]));

  }

}

