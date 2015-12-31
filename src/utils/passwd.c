#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"


void passwd(char *filename, char *playername, char *newpasswd)
{
  FILE *fl;
  FILE *outfile;
  struct char_file_u player;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    printf("Can't open %s.", filename);
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile.\r\n");
    fclose(fl);
    exit(1);
  }

  outfile = fopen("players.new", "w");

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);

    if (feof(fl)) {
      fclose(fl);
      fclose(outfile);
      printf("Done.\n");
      exit(0);
    }
      if (!strcmp(player.name, playername))
      {
        strncpy(player.pwd, CRYPT(newpasswd, player.name), MAX_PWD_LENGTH);
        *(player.pwd + MAX_PWD_LENGTH) = '\0';
      }
      fwrite(&player, sizeof(struct char_file_u), 1, outfile);


  }
}



int main(int argc, char *argv[])
{
  if (argc != 4)
    printf("Usage: %s playerfile-name player-name new-password\n", argv[0]);
  else
    passwd(argv[1], argv[2], argv[3]);

  return (0);
}
