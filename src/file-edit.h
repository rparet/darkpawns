#include <dirent.h>

/* Public functions in file-edit.c */
void tedit_string_cleanup(struct descriptor_data *d, int terminator);
int list_directory(struct char_data *ch, const char *dir, int(*filter)(const struct dirent *));
int view_file(struct char_data *ch, const char *dir, const char *file);
int edit_file(struct char_data *ch, const char *dir, const char *file, bool kill_on_empty);
void general_file_edit(struct char_data *ch, char *filename, char **buffer, int size);
