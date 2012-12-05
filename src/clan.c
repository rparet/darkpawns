/**************************************************************************
 * File: clan.c                       Intended to be used with CircleMUD  *
 * Usage: This is the code for clans                                      *
 * By Mehdi Keddache (Heritsun on Eclipse of Fate eclipse.argy.com 7777)  *
 * CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 * CircleMUD (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 **************************************************************************/

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-1999 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

/* $Id: clan.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "clan.h"

int num_of_clans;
struct clan_rec *clan;
extern struct descriptor_data *descriptor_list;
extern FILE *player_fl;
extern struct room_data *world;
void string_write(struct descriptor_data *d, char **writeto, size_t len,
                  long mail_to, void *data);

struct char_data *is_playing(char *vict_name); /* utils.c */
void save_char_file_u(struct char_file_u st); /* db.c */

char clan_privileges[NUM_CP+1][20] =
{
  "setplan",
  "enroll",
  "expel",
  "promote",
  "demote",
  "setfees",
  "withdraw",
  "setapplev"
};

void
send_clan_format(struct char_data *ch)
{
  int c,r;

  send_to_char("Clan commands available to you:\n\r"
	       "   clan status\r\n"
	       "   clan info <clan>\r\n",ch);
  if(GET_LEVEL(ch)>=LVL_CLAN_GOD) 
    send_to_char("   clan create     <leader> <clan name>\r\n"
		 "   clan destroy    <clan>\r\n"
                 "   clan rename     <#> <name>\r\n"
		 "   clan enroll     <player> <clan>\r\n"
		 "   clan expel      <player> <clan>\r\n"
		 "   clan promote    <player> <clan>\r\n"
		 "   clan demote     <player> <clan>\r\n"
		 "   clan withdraw   <amount> <clan>\r\n"
		 "   clan deposit    <amount> <clan>\r\n"
		 "   clan set ranks  <rank>   <clan>\r\n"
		 "   clan set appfee <amount> <clan>\r\n"
		 "   clan set dues   <amount> <clan>\r\n"
		 "   clan set applev <level>  <clan>\r\n"
		 "   clan set plan   <clan>\r\n"
                 "   clan private <clan>\r\n"
		 "   clan set privilege  <privilege>   <rank> <clan>\r\n"
		 "   clan set title  <clan number> <rank> <title>\r\n",ch);
  else
    {
      c=find_clan_by_id(GET_CLAN(ch));
      r=GET_CLAN_RANK(ch);
      if(!GET_CLAN(ch))
	send_to_char("   clan apply      <clan>\r\n",ch);
      if(r > 0)
	{
          send_to_char("   clan who\r\n", ch);
          send_to_char("   clan members\r\n", ch);
          send_to_char("   clan quit\r\n", ch);
	  send_to_char("   clan deposit    <amount>\r\n",ch);
	  if(r>=clan[c].privilege[CP_WITHDRAW])
	    send_to_char("   clan withdraw   <amount>\r\n" ,ch);
	  if(r>=clan[c].privilege[CP_ENROLL])
	    send_to_char("   clan enroll     <player>\r\n" ,ch);
	  if(r>=clan[c].privilege[CP_EXPEL])
	    send_to_char("   clan expel      <player>\r\n" ,ch);
	  if(r>=clan[c].privilege[CP_PROMOTE])
	    send_to_char("   clan promote    <player>\r\n",ch);
	  if(r>=clan[c].privilege[CP_DEMOTE])
	    send_to_char("   clan demote     <player>\r\n",ch);
	  if(r>=clan[c].privilege[CP_SET_APPLEV])
	    send_to_char("   clan set applev <level>\r\n",ch);
	  if(r>=clan[c].privilege[CP_SET_FEES])
	    send_to_char("   clan set appfee <amount>\r\n"
			 "   clan set dues   <amount>\r\n",ch);
	  if(r>=clan[c].privilege[CP_SET_PLAN])
	    send_to_char("   clan set plan\r\n",ch);
	  if(r==clan[c].ranks)
	    send_to_char("   clan private\r\n"
                         "   clan set ranks  <rank>\r\n"
			 "   clan set title  <rank> <title>\r\n"
			 "   clan set privilege  <privilege> <rank>\r\n",ch);
	}
    }
}

void
do_clan_rename (struct char_data *ch, char *arg)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int clan_num;

  half_chop(arg,arg1,arg2);

  if (!is_number(arg1)) 
  {
    send_to_char("You need to specify a clan number.\r\n",ch);
    return;
  }
  if ((clan_num=atoi(arg1))<0 || clan_num > num_of_clans) 
  {
      send_to_char("There is no clan with that number.\r\n",ch);
      return;
  }

  if (!*arg2)
  {
    stc("What do you want to rename it?\r\n", ch);
    return;
  }
  strncpy(clan[clan_num].name, CAP((char *)arg2), 32);
  save_clans();
  stc("Clan renamed.\r\n", ch);
}

void
do_clan_create (struct char_data *ch, char *arg)
{
  struct char_data *leader = NULL;
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  int new_id=0,i;
  
  if (!*arg)
    {
      send_clan_format(ch);
      return;
    }

  if (GET_LEVEL(ch) < LVL_CLAN_GOD)
    {
      send_to_char("You are not mighty enough to create new clans!\r\n", ch);
      return;
    }

  if(num_of_clans == MAX_CLANS)
    {
      send_to_char("Max clans reached. WOW!\r\n",ch);
      return;
    }

  half_chop(arg, arg1, arg2);

  if(!(leader=get_char_vis(ch,arg1)))
    {    
      send_to_char("The leader of the new clan must be present.\r\n",ch);
      return;
    }

  if(strlen(arg2)>=32) {
    send_to_char("Clan name too long! (32 characters max)\r\n",ch);
    return; }

  if(GET_LEVEL(leader)>=LVL_IMMORT) {
    send_to_char("You cannot set an immortal as the leader of a clan.\r\n",ch);
    return; }
  
  if(GET_CLAN(leader)!=0 && GET_CLAN_RANK(leader)!=0) {
    send_to_char("The leader already belongs to a clan!\r\n",ch);
    return; }
  
  if(find_clan(arg2)!=-1) {
    send_to_char("That clan name already exists!\r\n",ch);
    return; }
  
  strncpy(clan[num_of_clans].name, CAP((char *)arg2), 32);
  for(i=0;i<num_of_clans;i++)
    if(new_id<clan[i].id)
      new_id=clan[i].id;
  clan[num_of_clans].id=new_id+1;
  clan[num_of_clans].ranks =  2;
  strcpy(clan[num_of_clans].rank_name[0],"Member");
  strcpy(clan[num_of_clans].rank_name[1],"Leader");
  clan[num_of_clans].treasure = 0;
  clan[num_of_clans].members = 1;
  clan[num_of_clans].power = GET_LEVEL(leader);
  clan[num_of_clans].app_fee = 0;
  clan[num_of_clans].dues = 0;
  clan[num_of_clans].app_level = DEFAULT_APP_LVL;
  clan[num_of_clans].private = CLAN_PUBLIC;
  for(i=0;i<CLAN_PLAN_LENGTH;i++)
    clan[num_of_clans].description[i] = '\0';
  clan[num_of_clans].plan = NULL;
  for(i=0;i<5;i++)
    clan[num_of_clans].spells[i]=0;
  for(i=0;i<20;i++)
    clan[num_of_clans].privilege[i]=clan[num_of_clans].ranks;
  for(i=0;i<4;i++)
    clan[num_of_clans].at_war[i]=0;
  num_of_clans++;
  save_clans();
  send_to_char("Clan created.\r\n", ch);
  GET_CLAN(leader)=clan[num_of_clans-1].id;
  GET_CLAN_RANK(leader)=clan[num_of_clans-1].ranks;
  save_char(leader, leader->in_room);

  return;
}


void
do_clan_destroy (struct char_data *ch, char *arg)
{

  int i,j;
  extern int top_of_p_table;
  extern struct player_index_element *player_table;
  struct char_file_u chdata;
  struct char_data *victim=NULL;
  
  if (!*arg) {      
    send_clan_format(ch);
    return; }

  if ((i = find_clan(arg)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; }
    
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("Your not mighty enough to destroy clans!\r\n", ch);
    return; }

  for (j = 0; j <= top_of_p_table; j++){
    if((victim=is_playing((player_table +j)->name))) {
      if(GET_CLAN(victim)==clan[i].id) {
	GET_CLAN(victim)=0;
	GET_CLAN_RANK(victim)=0;
	save_char(victim, victim->in_room); } }
    else {
      load_char((player_table + j)->name, &chdata);
      if(chdata.player_specials_saved.clan==clan[i].id) {
	chdata.player_specials_saved.clan=0;
	chdata.player_specials_saved.clan_rank=0;
	save_char_file_u(chdata); } } }

  memset(&clan[i], sizeof(struct clan_rec), 0);
      
  for (j = i; j < num_of_clans - 1; j++)
    clan[j] = clan[j + 1];
  
  num_of_clans--;

  send_to_char("Clan deleted.\r\n", ch);
  save_clans();
  return;
}

void
do_clan_enroll (struct char_data *ch, char *arg)
{  
  extern int top_of_p_table;
  extern struct player_index_element *player_table;
  struct char_file_u chdata;
  struct char_data *victim;
  int clan_num, immcom=0, j;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_ENROLL] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }


  if (!*arg) 
  {
    stc("The following players have applied to your clan:\r\n"
        "-----------------------------------------------\r\n", ch);
    for (j = 0; j <= top_of_p_table; j++)
    {
       if ((victim = is_playing((player_table + j)->name))) 
       {
         if ((GET_CLAN(victim) == GET_CLAN(ch)) && 
             (GET_CLAN_RANK(victim) == 0)) 
         {
           sprintf(buf, "%s\r\n", GET_NAME(victim));
           stc(buf, ch);
         } 
       }
       else 
       {
         load_char((player_table + j)->name, &chdata);
         if ((chdata.player_specials_saved.clan == GET_CLAN(ch)) &&
             (chdata.player_specials_saved.clan_rank == 0)) 
         {
           sprintf(buf, "%s\r\n", chdata.name);
	   stc(buf, ch);
         } 
       } 
    }
    return; 
  }

  
  if(!(victim=get_char_room_vis(ch,arg))) {    
    send_to_char("Er, Who ??\r\n",ch);
    return;
  }
  else {
    if(GET_CLAN(victim)!=clan[clan_num].id) {
      if(GET_CLAN_RANK(victim)>0) {
	send_to_char("They're already in a clan.\r\n",ch);
	return;
      }
      else {
	send_to_char("They didn't request to join your clan.\r\n",ch);
	return;
      }
    }
    else 
      if(GET_CLAN_RANK(victim)>0) {
	send_to_char("They're already in your clan.\r\n",ch);
	return;
      }
    if(GET_LEVEL(victim)>=LVL_IMMORT) {
      send_to_char("You cannot enroll immortals in clans.\r\n",ch);
      return; }
  }

  GET_CLAN_RANK(victim)++;
  save_char(victim, victim->in_room);
  clan[clan_num].power += GET_LEVEL(victim);
  clan[clan_num].members++;
  send_to_char("You've been enrolled in the clan you chose!\r\n",victim);
  send_to_char("Done.\r\n",ch);
  return;
}

void
do_clan_expel (struct char_data *ch, char *arg)
{  
  struct char_data *vict=NULL;
  int clan_num,immcom=0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return; } }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return; } }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_EXPEL] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return; }

  if(!(vict=get_char_room_vis(ch,arg))) {    
    send_to_char("Er, Who ??\r\n",ch);
    return; }
  else {
    if(GET_CLAN(vict)!=clan[clan_num].id) {
      send_to_char("They're not in your clan.\r\n",ch);
      return; }
    else {
      if(GET_CLAN_RANK(vict)>=GET_CLAN_RANK(ch) && !immcom) {
	send_to_char("You cannot kick out that person.\r\n",ch);
	return; } } }

  GET_CLAN(vict)=0;
  GET_CLAN_RANK(vict)=0;
  save_char(vict, vict->in_room);
  clan[clan_num].members--;
  clan[clan_num].power-=GET_LEVEL(vict);
  send_to_char("You've been kicked out of your clan!\r\n",vict);
  send_to_char("Done.\r\n",ch);
  return;
}

void
do_clan_demote (struct char_data *ch, char *arg)
{  
  struct char_data *vict=NULL;
  int clan_num,immcom=0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return; } }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return; } }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_DEMOTE] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return; }

  if(!(vict=get_char_room_vis(ch,arg))) {    
    send_to_char("Er, Who ??\r\n",ch);
    return; }
  else {
    if(GET_CLAN(vict)!=clan[clan_num].id) {
      send_to_char("They're not in your clan.\r\n",ch);
      return; }
    else {
      if(GET_CLAN_RANK(vict)==1) {
	send_to_char("They can't be demoted any further, use expel now.\r\n",ch);
	return; }
      if(GET_CLAN_RANK(vict)>=GET_CLAN_RANK(ch) && !immcom) {
	send_to_char("You cannot demote a person of this rank!\r\n",ch);
	return; } } }

  GET_CLAN_RANK(vict)--;
  save_char(vict, vict->in_room);
  send_to_char("You've demoted within your clan!\r\n",vict);
  send_to_char("Done.\r\n",ch);
  return;
}

void
do_clan_promote (struct char_data *ch, char *arg)
{  
  struct char_data *vict=NULL;
  int clan_num,immcom=0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return; } }
  else {
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return; } }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_PROMOTE] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return; }

  if(!(vict=get_char_room_vis(ch,arg))) {    
    send_to_char("Er, Who ??\r\n",ch);
    return; }
  else {
    if(GET_CLAN(vict)!=clan[clan_num].id) {
      send_to_char("They're not in your clan.\r\n",ch);
      return; }
    else {
      if(GET_CLAN_RANK(vict)==0) {
	send_to_char("They're not enrolled yet.\r\n",ch);
	return; }
      if((GET_CLAN_RANK(vict)+1)>GET_CLAN_RANK(ch) && !immcom) {
	send_to_char("You cannot promote that person over your rank!\r\n",ch);
	return; }
      if(GET_CLAN_RANK(vict)==clan[clan_num].ranks) {
	send_to_char("You cannot promote someone over the top rank!\r\n",ch);
	return; } } }

  GET_CLAN_RANK(vict)++;
  save_char(vict, vict->in_room);
  send_to_char("You've been promoted within your clan!\r\n",vict);
  send_to_char("Done.\r\n",ch);
  return;
}

void
do_clan_who (struct char_data *ch)
{  
  struct descriptor_data *d;
  struct char_data *tch;
  char line_disp[MAX_STRING_LENGTH];
  int clan_num = find_clan_by_id(GET_CLAN(ch));

  if (!GET_CLAN_RANK(ch))
    {
      send_to_char("You do not belong to a clan!\r\n",ch);
      return;
    }

  send_to_char("\r\nClan members online\r\n",ch);
  send_to_char("-------------------------\r\n",ch);
  
  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected)
      continue;
    if ((tch = d->character))  /* need double b/c truth val */
      if (GET_CLAN(tch)==GET_CLAN(ch) && GET_CLAN_RANK(tch)>0 
          && CAN_SEE(ch, tch)) 
      {
        sprintf(line_disp,"%s %s\r\n",clan[clan_num].rank_name[GET_CLAN_RANK(tch)-1], 
                GET_NAME(tch));
        send_to_char(line_disp,ch); 
      }
  }
  return;
}

void
do_clan_members (struct char_data *ch)
{
  struct char_file_u player;
  int clan_num = find_clan_by_id(GET_CLAN(ch));


  if (!GET_CLAN(ch) || GET_CLAN_RANK(ch) == 0)
    {
      stc("You aren't in a clan!\r\n", ch);
      return;
    }


  send_to_char("\r\nList of your clan members\r\n",ch);
  send_to_char("-------------------------\r\n",ch);
  rewind(player_fl);  /* probably not a good idea -rparet */
  for (;;)
     {
       fread(&player, sizeof(struct char_file_u), 1, player_fl);
       if (feof(player_fl))
         return;
       if ((GET_CLAN(ch) == player.player_specials_saved.clan) &&
           (player.player_specials_saved.clan_rank != 0))
       {
         sprintf(buf, "%s %s \r\n",
                 clan[clan_num].rank_name[player.player_specials_saved.clan_rank-1], 
                 player.name);
         stc(buf, ch);
       }
     }
}

void
do_clan_quit (struct char_data *ch)
{
  int clan_num, j;
  bool disbanded = FALSE;

  if (GET_LEVEL(ch) >= LVL_IMMORT)
  {
    stc("You cannot quit any clan!\r\n", ch);
    return;
  }

  clan_num = find_clan_by_id(GET_CLAN(ch));

  GET_CLAN(ch) = 0;
  GET_CLAN_RANK(ch) = 0;
  save_char(ch, ch->in_room);
  clan[clan_num].members--;
  clan[clan_num].power -= GET_LEVEL(ch);

  if (clan[clan_num].members == 0)
  {
    memset(&clan[clan_num], sizeof(struct clan_rec), 0);
    for (j = clan_num; j < num_of_clans - 1; j++)
      clan[j] = clan[j + 1];
    num_of_clans--;
    disbanded = TRUE;
  }

  save_clans();
  
  sprintf(buf, "You've quit your clan.\r\n");
  sprintf(buf1, "You've quit your clan and it has been disbanded.\r\n");
  if (disbanded) 
    stc(buf1, ch); 
  else 
    stc(buf, ch);
}


void
do_clan_status (struct char_data *ch)
{  
  char line_disp[MAX_STRING_LENGTH];
  int clan_num;

  if(GET_LEVEL(ch)>=LVL_IMMORT) {
    send_to_char("You are immortal and cannot join any clan!\r\n",ch);
    return; }

  clan_num=find_clan_by_id(GET_CLAN(ch));

  if(GET_CLAN_RANK(ch)==0)
  {
    if (clan_num>=0) 
    {
      sprintf(line_disp,"You applied to %s\r\n",clan[clan_num].name);
      send_to_char(line_disp,ch);
      return; 
    }
    else 
    {
      send_to_char("You do not belong to a clan!\r\n",ch);
      return; 
    }
  }
  sprintf(line_disp,"You are %s (Rank %d) of %s\r\n",
	  clan[clan_num].rank_name[GET_CLAN_RANK(ch)-1],GET_CLAN_RANK(ch),
	  clan[clan_num].name);
  send_to_char(line_disp,ch);

  return;
}

void
do_clan_apply (struct char_data *ch, char *arg)
{  
  int clan_num;

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)>=LVL_IMMORT) {
    send_to_char("Gods cannot apply for any clan.\r\n",ch);
    return; }

  if(GET_CLAN_RANK(ch)>0) {
    send_to_char("You already belong to a clan!\r\n",ch);
    return; }
  else {
    if ((clan_num = find_clan(arg)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return; } }

  if(GET_LEVEL(ch) < clan[clan_num].app_level) {
    send_to_char("You are not mighty enough to apply to this clan.\r\n",ch);
    return; }

  if(GET_GOLD(ch) < clan[clan_num].app_fee) {
    send_to_char("You cannot afford the application fee!\r\n", ch);
    return; }

  GET_GOLD(ch) -= clan[clan_num].app_fee;
  clan[clan_num].treasure += clan[clan_num].app_fee;
  save_clans();
  GET_CLAN(ch)=clan[clan_num].id;
  save_char(ch, ch->in_room);
  send_to_char("You've applied to the clan!\r\n",ch);
  return;
}

void
do_clan_info (struct char_data *ch, char *arg)
{
  int i=0,j;

  if (num_of_clans == 0) 
  {
    send_to_char("No clans have formed yet.\r\n",ch);
    return; 
  }

  if (!(*arg)) 
  {
    sprintf(buf, "\r");
    sprintf(buf, "\t\t\tooO Clans of Dark Pawns Ooo\r\n");
    for (i=0; i < num_of_clans; i++)
    {
      if (GET_LEVEL(ch) >= LVL_IMMORT)
        sprintf(buf, "%s[%-2d]  %-17s Members: %3d  Power: %3d  Appfee: %d Applvl: %d\r\n",
              buf, clan[i].id, clan[i].name, clan[i].members, clan[i].power,
              clan[i].app_fee, clan[i].app_level);
      else if (!clan[i].private)
        sprintf(buf, "%s%-17s Members: %3d  Power: %3d  Appfee: %d Applvl: %d\r\n",
              buf, clan[i].name, clan[i].members, clan[i].power,
              clan[i].app_fee, clan[i].app_level);
    }
    page_string(ch->desc,buf, 1);
    return; 
  }
  else if ((i = find_clan(arg)) < 0) 
    {
      send_to_char("Unknown clan.\r\n", ch);
      return; 
    }

  sprintf(buf, "Info for the clan %s :\r\n",clan[i].name);
  send_to_char(buf, ch);
  sprintf(buf, "Ranks      : %d\r\nTitles     : ",clan[i].ranks);
  for(j=0;j<clan[i].ranks;j++)
    sprintf(buf, "%s%s ",buf,clan[i].rank_name[j]);
  sprintf(buf, "%s\r\nMembers    : %d\r\nPower      : %d\t\nTreasure   : %ld\r\nSpells     : ",buf, clan[i].members, clan[i].power, clan[i].treasure);
  for(j=0; j<5;j++)
    if(clan[i].spells[j])
      sprintf(buf, "%s%d ",buf,clan[i].spells[j]);
  sprintf(buf, "%s\r\n",buf);
  send_to_char(buf, ch);
  sprintf(buf,"Clan privileges:\r\n");
  for(j=0; j<NUM_CP;j++)
    sprintf(buf, "%s   %-10s: %d\r\n",buf,clan_privileges[j],clan[i].privilege[j]);
  sprintf(buf, "%s\r\n",buf);
  send_to_char(buf, ch);
  sprintf(buf, "Description:\r\n%s\r\n\n", clan[i].plan);
  send_to_char(buf, ch);
  if((clan[i].at_war[0] == 0) && (clan[i].at_war[1] == 0) && (clan[i].at_war[2] == 0) && (clan[i].at_war[3] == 0))
    send_to_char("This clan is at peace with all others.\r\n", ch);
  else
    send_to_char("This clan is at war.\r\n", ch);
  sprintf(buf, "Application fee  : %d gold\r\nMonthly Dues     : %d gold\r\n", clan[i].app_fee, clan[i].dues);
  send_to_char(buf, ch);
  sprintf(buf, "Application level: %d\r\n", clan[i].app_level);
  send_to_char(buf, ch);

  return;
}

int
find_clan_by_id(int idnum)
{
  int i;
  for( i=0; i < num_of_clans; i++)
    if(idnum==clan[i].id)
      return i;
  return -1;
}

int
find_clan(char *name)
{
  int i;
  for( i=0; i < num_of_clans; i++)
    if(!str_cmp(name, clan[i].name))
      return i;
  return -1;
}

void
save_clans()
{
  FILE *fl = NULL;
  int i = 0;

  if (!(fl = fopen(CLAN_FILE, "wb"))) 
  {
    log("SYSERR: Unable to open clan file");
    return; 
  }

  fwrite(&num_of_clans, sizeof(int), 1, fl);
  for (i=0; i<num_of_clans; i++) {
    if (clan[i].plan != NULL)
      strncpy(clan[i].description, clan[i].plan, CLAN_PLAN_LENGTH-1);
    fwrite(&clan[i], sizeof(struct clan_rec), 1, fl);
  }

  fclose(fl);
  return;
}


extern int top_of_p_table;
extern struct player_index_element *player_table;

void
init_clans()
{
  FILE *fl;
  int i,j;
  struct char_file_u chdata;

  CREATE(clan, struct clan_rec, MAX_CLANS);
  num_of_clans=0;
  i=0;

  if (!(fl = fopen(CLAN_FILE, "rb"))) 
  {
    log("   Clan file does not exist. Will create a new one");
    save_clans();
    return; 
  }

  fread(&num_of_clans, sizeof(int), 1, fl);
  for(i=0;i<num_of_clans;i++) {
    fread(&clan[i], sizeof(struct clan_rec), 1, fl);
    if (clan[i].description[0] != '\0')
      clan[i].plan = strdup(clan[i].description);
    else
      clan[i].plan = NULL;
  }
  fclose(fl);

  log("   Calculating powers and members");
  for(i=0;i<num_of_clans;i++) {
    clan[i].power=0;
    clan[i].members=0;
  }
  for (j = 0; j <= top_of_p_table; j++){
    load_char((player_table + j)->name, &chdata);
    if((i=find_clan_by_id(chdata.player_specials_saved.clan))>=0 &&
        chdata.player_specials_saved.clan_rank != 0) {
      clan[i].power+=chdata.level;
      clan[i].members++;
    }
  }

  return;
}
  
void
do_clan_bank(struct char_data *ch, char *arg, int action)
{
  int clan_num = -1;
  int immcom=0;
  long amount=0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) 
  {
    send_clan_format(ch);
    return; 
  }

  if ( GET_LEVEL(ch) < LVL_IMMORT) {
    clan_num = find_clan_by_id(GET_CLAN(ch));
    if ( (clan_num < 0) ||
         GET_CLAN_RANK(ch) == 0 ) 
    {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  } else {
    if (GET_LEVEL(ch) < LVL_CLAN_GOD) 
    {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; 
    }
    immcom = 1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) 
    {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_WITHDRAW] &&
     !immcom                                                 &&
     action==CB_WITHDRAW) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }

  if(!(*arg)) {
    switch(action) {
    case CB_DEPOSIT:
      send_to_char("Deposit how much?\r\n",ch);
      break;
    case CB_WITHDRAW:
      send_to_char("Withdraw how much?\r\n", ch);
      break;
    default:
      send_to_char("Bad clan banking call, please report to a God.\r\n", ch);
    }
    return;
  }
  
  if(!is_number(arg)) {
    switch(action) {
    case CB_DEPOSIT:
      send_to_char("Deposit what?\r\n",ch);
      break;
    case CB_WITHDRAW:
      send_to_char("Withdraw what?\r\n", ch);
      break;
    default:
      send_to_char("Bad clan banking call, please report to a God.\r\n", ch);
    }
    return;
  }

  amount=atoi(arg);

  switch(action) {
  case CB_WITHDRAW:
     if(action==CB_WITHDRAW && clan[clan_num].treasure<amount) {
       send_to_char("The clan is not wealthy enough for your needs!\r\n",ch);
       return;
     }
     GET_GOLD(ch)+=amount;
     clan[clan_num].treasure-=amount;
     send_to_char("You withdraw from the clan's treasure.\r\n",ch);
     sprintf(buf2, "%s withdraws %ld coins from %s clan account.", GET_NAME(ch), amount, HSHR(ch));
     mudlog(buf2, BRF, LVL_IMMORT, TRUE);
     break;
  case CB_DEPOSIT:
     if(!immcom && action==CB_DEPOSIT && GET_GOLD(ch)<amount) {
       send_to_char("You do not have that kind of money!\r\n",ch);
       return;
     }
     if(!immcom) GET_GOLD(ch)-=amount;
     clan[clan_num].treasure+=amount;
     send_to_char("You add to the clan's treasure.\r\n",ch);
     sprintf(buf2, "%s adds %ld coins to %s clan account.", GET_NAME(ch), amount, HSHR(ch));
     mudlog(buf2, BRF, LVL_IMMORT, TRUE);
     break;
  default:
    send_to_char("Problem in command, please report.\r\n",ch);
    break;
  }
  save_char(ch, ch->in_room);
  save_clans();
  return;
}

void
do_clan_money(struct char_data *ch, char *arg, int action)
{
  int clan_num,immcom=0;
  long amount=0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_FEES] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }

  if(!(*arg)) {
    send_to_char("Set it to how much?\r\n",ch);
    return;
  }
  
  if(!is_number(arg)) {
    send_to_char("Set it to what?\r\n",ch);
    return;
  }

  amount=atoi(arg);

  if ((amount < 0) || (amount > 10000))  
  {
    stc("Please pick a number between 0 and 10,000 coins.\r\n", ch);
    return;
  }

  switch(action) {
  case CM_APPFEE:
    clan[clan_num].app_fee=amount;
    send_to_char("You change the application fee.\r\n",ch);
    break;
  case CM_DUES:
    clan[clan_num].dues=amount;
    send_to_char("You change the monthly dues.\r\n",ch);
    break;
  default:
    send_to_char("Problem in command, please report.\r\n",ch);
    break;
  }

  save_clans();
  return;
}

void
do_clan_ranks(struct char_data *ch, char *arg)
{
  int i,j;
  int clan_num,immcom=0;
  int new_ranks;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  extern int top_of_p_table;
  extern struct player_index_element *player_table;
  struct char_file_u chdata;
  struct char_data *victim=NULL;

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }

  if(!(*arg)) {
    send_to_char("Set how many ranks?\r\n",ch);
    return;
  }
  
  if(!is_number(arg)) {
    send_to_char("Set the ranks to what?\r\n",ch);
    return;
  }

  new_ranks=atoi(arg);

  if(new_ranks==clan[clan_num].ranks) {
    send_to_char("The clan already has this number of ranks.\r\n",ch);
    return;
  }

  if(new_ranks<2 || new_ranks>20) {
    send_to_char("Clans must have from 2 to 20 ranks.\r\n",ch);
    return;
  }

  if(GET_GOLD(ch)<5000 && !immcom) {
    send_to_char("Changing the clan hierarchy requires 5,000 coins!\r\n",ch);
    return;
  }

  if(!immcom)
    GET_GOLD(ch)-=5000;

  for (j = 0; j <= top_of_p_table; j++) {
    if((victim=is_playing((player_table +j)->name))) {
      if(GET_CLAN(victim)==clan[clan_num].id) {
	if(GET_CLAN_RANK(victim)<clan[clan_num].ranks && GET_CLAN_RANK(victim)>0)
	  GET_CLAN_RANK(victim)=1;
	if(GET_CLAN_RANK(victim)==clan[clan_num].ranks) 
	  GET_CLAN_RANK(victim)=new_ranks;
	save_char(victim, victim->in_room);
      }
    }
    else {
      load_char((player_table + j)->name, &chdata);
      if(chdata.player_specials_saved.clan==clan[clan_num].id) {
	if(chdata.player_specials_saved.clan_rank<clan[clan_num].ranks && chdata.player_specials_saved.clan_rank>0)
	  chdata.player_specials_saved.clan_rank=1;
	if(chdata.player_specials_saved.clan_rank==clan[clan_num].ranks)
	  chdata.player_specials_saved.clan_rank=new_ranks;
	save_char_file_u(chdata);
      }
    }
  }

  clan[clan_num].ranks=new_ranks;
  for(i=0;i<clan[clan_num].ranks-1;i++)
    strcpy(clan[clan_num].rank_name[i],"Member");
  strcpy(clan[clan_num].rank_name[clan[clan_num].ranks -1],"Leader");
  for(i=0;i<NUM_CP;i++)
    clan[clan_num].privilege[i]=new_ranks;

  save_clans();
  return;
}

void
do_clan_titles( struct char_data *ch, char *arg)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int clan_num=0,rank;

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
    if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks) {
      send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg2);
    if(!is_number(arg1)) {
      send_to_char("You need to specify a clan number.\r\n",ch);
      return;
    }
    if((clan_num=atoi(arg1))<0 || clan_num>=num_of_clans) {
      send_to_char("There is no clan with that number.\r\n",ch);
      return;
    }
  }

  half_chop(arg,arg1,arg2);

  if(!is_number(arg1)) {
    send_to_char("You need to specify a rank number.\r\n",ch);
    return; }

  rank=atoi(arg1);

  if(rank<1 || rank>clan[clan_num].ranks) {
    send_to_char("This clan has no such rank number.\r\n",ch);
    return; }

  if(strlen(arg2)<1 || strlen(arg2)>19) {
    send_to_char("You need a clan title of under 20 characters.\r\n",ch);
    return; }

  strcpy(clan[clan_num].rank_name[rank-1],arg2);
  save_clans();
  send_to_char("Done.\r\n",ch);
  return;
}

void
do_clan_private(struct char_data *ch, char *arg)
{
  int clan_num=0;

  if (GET_LEVEL(ch)<LVL_IMMORT) 
  {
    if ((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) 
    {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
    if (GET_CLAN_RANK(ch)!=clan[clan_num].ranks) 
    {
      send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
      return;
    }
  }
  else 
  {
    if (GET_LEVEL(ch)<LVL_CLAN_GOD) 
    {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; 
    }
    if ((clan_num = find_clan(arg)) < 0) 
    {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if (clan[clan_num].private == CLAN_PUBLIC)
  {
    clan[clan_num].private = CLAN_PRIVATE;
    stc("Your clan is now private.\r\n", ch);
    save_clans();
    return;
  }
  
  if (clan[clan_num].private == CLAN_PRIVATE)
  {
    clan[clan_num].private = CLAN_PUBLIC;
    stc("Your clan is now public.\r\n", ch);
    save_clans();
    return;
  }
}


void
do_clan_application( struct char_data *ch, char *arg)
{
  int clan_num,immcom=0;
  int applevel;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg2)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_APPLEV] && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }

  if(!(*arg)) {
    send_to_char("Set to which level?\r\n",ch);
    return;
  }
  
  if(!is_number(arg)) {
    send_to_char("Set the application level to what?\r\n",ch);
    return;
  }

  applevel=atoi(arg);

  if(applevel<1 || applevel>30) {
    send_to_char("The application level can go from 1 to 30.\r\n",ch);
    return;
  }

  clan[clan_num].app_level=applevel;
  save_clans();

  return;
}

void
do_clan_sp(struct char_data *ch, char *arg, int priv)
{
  int clan_num,immcom=0;
  int rank;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if (!(*arg)) {
    send_clan_format(ch);
    return; }


  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return; }
    immcom=1;
    half_chop(arg,arg1,arg2);
    strcpy(arg,arg1);
    if ((clan_num = find_clan(arg1)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks && !immcom) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }

  if(!(*arg)) {
    send_to_char("Set the privilege to which rank?\r\n",ch);
    return;
  }
  
  if(!is_number(arg)) {
    send_to_char("Set the privilege to what?\r\n",ch);
    return;
  }

  rank=atoi(arg);

  if(rank<1 || rank>clan[clan_num].ranks) {
    send_to_char("There is no such rank in the clan.\r\n",ch);
    return;
  }

  clan[clan_num].privilege[priv]=rank;
  save_clans();

  return;
}

void
do_clan_plan(struct char_data *ch, char *arg)
{
  int clan_num = 0;

  if(GET_LEVEL(ch)<LVL_IMMORT) {
    if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
      send_to_char("You don't belong to any clan!\r\n",ch);
      return;
     }
    if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_PLAN]) {
      send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
      return;
    }
  }
  else {
    if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
      send_to_char("You do not have clan privileges.\r\n", ch);
      return;
    }
    if (!(*arg)) {
      send_clan_format(ch);
      return;
    }
    if ((clan_num = find_clan(arg)) < 0) {
      send_to_char("Unknown clan.\r\n", ch);
      return;
    }
  }

  if(!clan[clan_num].plan) {
    sprintf(buf, "Enter the description, or plan for clan <<%s>>.\r\n",
            clan[clan_num].name);
    send_to_char(buf, ch);
  }
  else {
    sprintf(buf, "Old plan for clan <<%s>>:\r\n%s\r\n",
            clan[clan_num].name, clan[clan_num].plan);
    send_to_char(buf, ch);
    send_to_char("Enter new plan:\r\n", ch);
  }
  send_to_char("End with @ on a line by itself.\r\n", ch);
  FREE(clan[clan_num].plan);
  clan[clan_num].plan = NULL;
  string_write(ch->desc, &clan[clan_num].plan, 
               CLAN_PLAN_LENGTH, 0, NULL);
  save_clans();
  return;
}

void
do_clan_privilege( struct char_data *ch, char *arg)
{
  char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
  int i;

  half_chop(arg,arg1,arg2);

  if (is_abbrev(arg1,"setplan"  ))
    {
      do_clan_sp(ch,arg2,CP_SET_PLAN);
      return ;
    }
  if (is_abbrev(arg1,"enroll"   ))
    {
      do_clan_sp(ch,arg2,CP_ENROLL);
      return ;
    }
  if (is_abbrev(arg1,"expel"    ))
    {
      do_clan_sp(ch,arg2,CP_EXPEL);
      return ;
    }
  if (is_abbrev(arg1,"promote"  ))
    {
      do_clan_sp(ch,arg2,CP_PROMOTE);
      return ;
    }
  if (is_abbrev(arg1,"demote"   ))
    {
      do_clan_sp(ch,arg2,CP_DEMOTE);
      return ;
    }
  if (is_abbrev(arg1,"withdraw" ))
    {
      do_clan_sp(ch,arg2,CP_WITHDRAW);
      return ;
    }
  if (is_abbrev(arg1,"setfees"  ))
    {
      do_clan_sp(ch,arg2,CP_SET_FEES);
      return ;
    }
  if (is_abbrev(arg1,"setapplev"))
    {
      do_clan_sp(ch,arg2,CP_SET_APPLEV);
      return ;
    }
  send_to_char("\r\nClan privileges:\r\n", ch);
  for(i=0;i<NUM_CP;i++)
    {
      sprintf(arg1,"\t%s\r\n",clan_privileges[i]);
      send_to_char(arg1,ch);
    }
}

void
do_clan_set(struct char_data *ch, char *arg)
{  
  char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];

  half_chop(arg,arg1,arg2);

  if (is_abbrev(arg1, "plan"      ))
    {
      do_clan_plan(ch,arg2);            return ;
    }
  if (is_abbrev(arg1, "ranks"     ))
    {
      do_clan_ranks(ch,arg2);
      return ;
    }
  if (is_abbrev(arg1, "title"     ))
    {
      do_clan_titles(ch,arg2);
      return ;
    }
  if (is_abbrev(arg1, "privilege" ))
    {
      do_clan_privilege(ch,arg2);
      return ;
    }
  if (is_abbrev(arg1, "dues"      ))
    {
      do_clan_money(ch,arg2,CM_DUES);
      return ;
    }
  if (is_abbrev(arg1, "appfee"    ))
    {
      do_clan_money(ch,arg2,CM_APPFEE);
      return ;
    }
  if (is_abbrev(arg1, "applev"    ))
    {
      do_clan_application(ch,arg2);
      return ;
    }
  send_clan_format(ch);
}

ACMD(do_clan)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  
  half_chop(argument, arg1, arg2);
  
  if (is_abbrev(arg1, "rename"  )) { do_clan_rename(ch,arg2);   return ;}
  if (is_abbrev(arg1, "create"  )) { do_clan_create(ch,arg2);   return ;}
  if (is_abbrev(arg1, "destroy" )) { do_clan_destroy(ch,arg2);  return ;}
  if (is_abbrev(arg1, "enroll"  )) { do_clan_enroll(ch,arg2);   return ;}
  if (is_abbrev(arg1, "expel"   )) { do_clan_expel(ch,arg2);    return ;}
  if (is_abbrev(arg1, "who"     )) { do_clan_who(ch);           return ;}
  if (is_abbrev(arg1, "status"  )) { do_clan_status(ch);        return ;}
  if (is_abbrev(arg1, "info"    )) { do_clan_info(ch,arg2);     return ;}
  if (is_abbrev(arg1, "apply"   )) { do_clan_apply(ch,arg2);    return ;}
  if (is_abbrev(arg1, "demote"  )) { do_clan_demote(ch,arg2);   return ;}
  if (is_abbrev(arg1, "promote" )) { do_clan_promote(ch,arg2);  return ;}
  if (is_abbrev(arg1, "set"     )) { do_clan_set(ch,arg2);      return ;}
  if (is_abbrev(arg1, "members" )) { do_clan_members(ch);       return ;}
  if (is_abbrev(arg1, "private" )) { do_clan_private(ch, arg2); return ;}
  if (is_abbrev(arg1, "quit"    )) { do_clan_quit(ch);          return ;}
  if (is_abbrev(arg1, "withdraw"))
    {
      do_clan_bank(ch,arg2,CB_WITHDRAW);
      return ;
    }
  if (is_abbrev(arg1, "deposit" ))
    {
      do_clan_bank(ch,arg2,CB_DEPOSIT);
      return ;
    }
  send_clan_format(ch);
}

