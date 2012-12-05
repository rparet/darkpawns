#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"



void
list_player(int idnum; FILE *fl)
{
  struct char_file_u player;

  for (;;)
       {
         fread(&player, sizeof(struct char_file_u), 1, player_fl);
         if (feof(player_fl))
           return;
         if ((idnum == player.player_specials_saved.clan) &&
             (player.player_specials_saved.clan_rank != 0))
         {
           printf(buf, "%s \r\n", player.name);
         }
       }

}

int main(int argc, char *argv[])
{
  FILE *fl;

  if (!(fl = fopen("players", "r+"))) {
    printf("Can't open player file\r\n");
    exit(1);
  }

  if (argc != 2)
    printf("Usage: %s player-name\n", argv[0]);
  else
    list_player(argv[1], fl);

  return (0);
}
