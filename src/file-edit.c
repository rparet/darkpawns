/*
 * This file provides functions to edit, view, and create files.
 */

#include <dirent.h>

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "olc.h"
#include "improved-edit.h"

/* $Id: /local/dp2.2/src/db.c 1285 2008-04-23T02:27:13.368876Z jsravn  $ */

static int kill_file(char *pathname)
{
  /* If file doesn't exist there is no need to remove it */
  if (access(pathname, R_OK))
    return 0;

  return remove(pathname);
}

void tedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  FILE *fl;
  char *storage = OLC_STORAGE(d);

  if (!storage)
    terminator = STRINGADD_ABORT;

  switch (terminator) {
  case STRINGADD_SAVE:
    if (OLC(d)->kill_on_empty && (!(*d->str) || **d->str == '\0')) {
      if (kill_file(storage)) {
        sprintf(buf, "SYSERR: Can't delete file '%s'.", storage);
        mudlog(buf, CMP, LVL_IMPL, TRUE);
      } else {
        sprintf(buf, "OLC: %s deletes '%s'.", GET_NAME(d->character), storage);
        mudlog(buf, CMP, LVL_GOD, TRUE);
        write_to_output(d, "Deleted.\r\n");
      }
    } else if (!(fl = fopen(storage, "w"))) {
      sprintf(buf, "SYSERR: Can't write file '%s'.", storage);
      mudlog(buf, CMP, LVL_IMPL, TRUE);
    } else {
      if (*d->str) {
        strip_string(*d->str);
        fputs(*d->str, fl);
      }
      fclose(fl);
      sprintf(buf, "OLC: %s saves '%s'.", GET_NAME(d->character), storage);
      mudlog(buf, CMP, LVL_GOD, TRUE);
      write_to_output(d, "Saved.\r\n");
    }
    break;
  case STRINGADD_ABORT:
    write_to_output(d, "Edit aborted.\r\n");
    act("$n stops editing some scrolls.", TRUE, d->character, 0, 0, TO_ROOM);
    break;
  default:
    log("SYSERR: tedit_string_cleanup: Unknown terminator status.");
    break;
  }

  /* Common cleanup code. */
  cleanup_olc(d, CLEANUP_ALL);
  STATE(d) = CON_PLAYING;
}

int file_to_string_alloc(char *name, char **buf);

/* Returns non-zero result if directory is safe */
static int valid_directory(const char *file)
{
  return matches(file,"^[/a-zA-Z0-9_-]+$");
}

int list_directory(struct char_data *ch, const char *dir, int(*filter)(const struct dirent *))
{
  struct dirent **nl;
  int i, n;

  if (!valid_directory(dir))
    return -1;

  n = scandir(dir, &nl, filter, alphasort);

  if (n < 0)
    return -1;
  else {
    for (i = 0; i < n; i++) {
      if (i && !(i % 3))
        send_to_char("\r\n", ch);

      sprintf(buf, "%-26.26s", nl[i]->d_name);
      send_to_char(buf, ch);

      free(nl[i]);
    }

    if (!i)
      send_to_char("None.\r\n", ch);
    else
      send_to_char("\r\n", ch);

    free(nl);
  }

  return 0;
}

/* Returns non-zero result if filename is safe */
static int valid_filename(const char *file)
{
  return matches(file, "^[a-zA-Z0-9_-]+.?[a-zA-Z0-9_-]*$");
}

static void path(char *pathname, const char *dir, const char *file)
{
  sprintf(pathname, "%s/%s", dir, file);
}

int view_file(struct char_data *ch, const char *dir, const char *file)
{
  char fname[MAX_INPUT_LENGTH];
  char *filebuf = NULL;

  if (!valid_filename(file))
    return -1;

  path(fname, dir, file);

  if (!access(fname, R_OK))
    file_to_string_alloc(fname, &filebuf);
  else
    return -1;

  send_to_char(filebuf, ch);

  FREE(filebuf);

  return 0;
}

void general_file_edit(struct char_data *ch, char *filename, char **buffer, int size)
{
  char *backstr = NULL;

  /* set up editor stats */
  send_editor_help(ch->desc);
  send_to_char("Edit file below:\r\n\r\n", ch);

  if (!ch->desc->olc) {
    CREATE(ch->desc->olc, struct olc_data, 1);
  }

  if (buffer && *buffer) {
    sprintf(buf, "%s", *buffer);
    send_to_char(buf, ch);
    backstr = strdup(*buffer);
  }

  OLC_STORAGE(ch->desc) = strdup(filename);
  string_write(ch->desc, buffer, size, 0, backstr);

  act("$n begins editing a scroll.", TRUE, ch, 0, 0, TO_ROOM);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);
  STATE(ch->desc) = CON_TEDIT;
}

int edit_file(struct char_data *ch, const char *dir, const char *file, bool kill_on_empty)
{
  char fname[MAX_INPUT_LENGTH];

  if (!valid_filename(file))
    return -1;

  if (!ch->desc->olc) {
    CREATE(ch->desc->olc, struct olc_data, 1);
  }

  path(fname, dir, file);

  if (!access(fname, R_OK | W_OK))
    file_to_string_alloc(fname, &OLC_BUFFER(ch->desc));
  else
    CREATE(OLC_BUFFER(ch->desc), char, MAX_STRING_LENGTH);

  OLC(ch->desc)->kill_on_empty = kill_on_empty;

  general_file_edit(ch, fname, &OLC_BUFFER(ch->desc), MAX_STRING_LENGTH);

  return 0;
}
