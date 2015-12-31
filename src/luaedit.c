/*
 * In game editor for modifying LUA scripts.
 */

/* $Id: /local/dp2.2/src/db.c 1285 2008-04-23T02:27:13.368876Z jsravn  $ */

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "olc.h"
#include "improved-edit.h"
#include "file-edit.h"

static int luafilter(const struct dirent *a)
{
  return matches(a->d_name, "^([0-9]+|archive|mob|obj|room|.+[.]lua)$");
}

static bool is_scripts_root(const char *pathname)
{
  return !matches(pathname, "^(mob|obj|room)");
}

ACMD(do_luaedit)
{
  int r;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char err[] = "Invalid lua edit option.\r\n";

  two_arguments(argument, arg1, arg2);

  if (*arg1 == '\0')
    strcpy(buf, SCRIPT_DIR);
  else if (is_scripts_root(arg1)) {
    strcpy(buf, SCRIPT_DIR);
    strcpy(arg2, arg1);
  } else
    sprintf(buf, SCRIPT_DIR "/%s", arg1);

  if (*arg2 == '\0') {
    if (list_directory(ch, buf, luafilter))
      send_to_char(err, ch);
  } else {
    if (!strstr(arg2, ".lua")) {
      strncat(arg2, ".lua", MAX_INPUT_LENGTH - 1);
    }
    if (GET_LEVEL(ch) < subcmd)
      r = view_file(ch, buf, arg2);
    else
      r = edit_file(ch, buf, arg2, TRUE);
    if (r)
      send_to_char(err, ch);
  }
}
