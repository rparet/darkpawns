#include "../conf.h"
#include "../sysdep.h"
#include "../structs.h"
#include "../house.h"
#include "../conf.h"
#include "../utils.h"

struct new_house_control_rec {
   sh_int vnum;                 /* vnum of this house           */
   sh_int atrium;               /* vnum of atrium               */
   sh_int exit_num;             /* direction of house's exit    */
   time_t built_on;             /* date this house was built    */
   int mode;                    /* mode of ownership            */
   long owner;                  /* idnum of house's owner       */
   int num_of_guests;           /* how many guests for house    */
   long guests[MAX_GUESTS];     /* idnums of house's guests     */
   time_t last_payment;         /* date of last house payment   */
   sh_int key;                  /* vnum of this house's key     */
   long spare1;
   long spare2;
   long spare3;
   long spare4;
   long spare5;
   long spare6;
   long spare7;
};



int num_of_houses = 0;

struct new_house_control_rec house_control[MAX_HOUSES];

void house_read_in(void)
{

  struct house_control_rec old_house;
  struct new_house_control_rec new_house;
  FILE *fl;
  int j;

  memset((char *)house_control,0,sizeof(struct new_house_control_rec)*MAX_HOUSES);

  if (!(fl = fopen("../lib/etc/hcontrol", "rb"))) {
    return;
  }

  while (!feof(fl) && num_of_houses < MAX_HOUSES)
  {
    fread(&old_house, sizeof(struct house_control_rec), 1, fl);

    if (feof(fl))
      break;

    new_house.vnum = old_house.vnum;
    new_house.atrium = old_house.atrium;
    new_house.exit_num = old_house.exit_num;
    new_house.built_on = old_house.built_on;
    new_house.mode = old_house.mode;
    new_house.owner = old_house.owner;
    new_house.num_of_guests = old_house.num_of_guests;

    for (j = 0; j < old_house.num_of_guests; j++)
       new_house.guests[j] = old_house.guests[j];

    new_house.last_payment = old_house.last_payment;
    new_house.key = -1;


    house_control[num_of_houses++] = new_house;
  }

  fclose(fl);
}

void house_write_out(void)
{

  FILE *fl;

  if (!(fl = fopen("../lib/etc/hcontrol.new", "wb")))
    return;

  fwrite(house_control, sizeof(struct new_house_control_rec), num_of_houses, fl);

  fclose(fl);
}


int main(void)
{

  house_read_in();
  house_write_out();

  return 1;
}

