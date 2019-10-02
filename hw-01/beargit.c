#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

#define BEARGIT_DIR_NAME_SIZE  9
#define COMMIT_DIR_PATH_SIZE (BEARGIT_DIR_NAME_SIZE + COMMIT_ID_SIZE)
#define COMMIT_FILE_PATH_SIZE (COMMIT_DIR_PATH_SIZE + FILENAME_SIZE)

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE *findex = fopen(".beargit/.index", "w");
  fclose(findex);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char *filename) {
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 *
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char *filename) {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  unsigned short is_file_tracked = 0;
  char line[FILENAME_SIZE];

  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      is_file_tracked = 1;
    } else {
      fprintf(fnewindex, "%s\n", line);
    }
  }

  if (!is_file_tracked) {
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".beargit/.newindex");
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    return 1;
   }

  fclose(findex);
  fclose(fnewindex);
  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char *go_bears = "GO BEARS!";

int is_commit_msg_ok(const char *msg) {
  /* COMPLETE THE REST */
  unsigned short number_of_same_chars = 0;
  char character;

  while ((character = *msg) != '\0') {
    if (character == go_bears[number_of_same_chars]) {
      number_of_same_chars++;

      if (go_bears[number_of_same_chars] == '\0') {
        return 1;
      }
    } else {
      number_of_same_chars = 0;
    }

    msg++;
  }

  return 0;
}

const char FIRST_COMMIT_ID[COMMIT_ID_SIZE] = "1111111111111111111111111111111111111111";

void next_commit_id(char *commit_id) {
  /* COMPLETE THE REST */
  if (commit_id[0] == '0') {
    strcpy(commit_id, FIRST_COMMIT_ID);
    return;
  }

  for (int i = 0; i < COMMIT_ID_SIZE; i++) {
    switch (commit_id[i]) {
      case '1':
        commit_id[i] = '6';
        return;
      case '6':
        commit_id[i] = 'c';
        return;
    }
  }

  // When the maximum commit ID is reached, switch to the minimum one.
  strcpy(commit_id, FIRST_COMMIT_ID);
}

int beargit_commit(const char *msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  /* COMPLETE THE REST */
  char commit_dir_path[COMMIT_DIR_PATH_SIZE];
  sprintf(commit_dir_path, ".beargit/%s", commit_id);
  fs_mkdir(commit_dir_path);

  char commit_file_path[COMMIT_FILE_PATH_SIZE];
  sprintf(commit_file_path, "%s/.index", commit_dir_path);
  fs_cp(".beargit/.index", commit_file_path);
  sprintf(commit_file_path, "%s/.prev", commit_dir_path);
  fs_cp(".beargit/.prev", commit_file_path);

  FILE *findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    sprintf(commit_file_path, "%s/%s", commit_dir_path, line);
    fs_cp(line, commit_file_path);
  }

  // Erase the contents of the index file.
  freopen(".beargit/.index", "w", findex);
  fclose(findex);

  sprintf(commit_file_path, "%s/.msg", commit_dir_path);
  write_string_to_file(commit_file_path, msg);

  write_string_to_file(".beargit/.prev", commit_id);

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  int file_count = 0;
  char line[FILENAME_SIZE];

  fprintf(stdout, "Tracked files:\n\n");
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fprintf(stdout, "  %s\n", line);
    file_count++;
  }

  fprintf(stdout, "\n%d files total\n", file_count);

  fclose(findex);

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int beargit_log() {
  /* COMPLETE THE REST */
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  if (commit_id[0] == '0') {
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  }

  char commit_dir_path[COMMIT_DIR_PATH_SIZE];
  char commit_file_path[COMMIT_FILE_PATH_SIZE];
  char commit_msg[MSG_SIZE];
  while (commit_id[0] != '0') {
    fprintf(stdout, "\ncommit %s\n", commit_id);

    sprintf(commit_dir_path, ".beargit/%s", commit_id);

    sprintf(commit_file_path, "%s/.msg", commit_dir_path);
    read_string_from_file(commit_file_path, commit_msg, MSG_SIZE);
    fprintf(stdout, "    %s\n", commit_msg);

    sprintf(commit_file_path, "%s/.prev", commit_dir_path);
    read_string_from_file(commit_file_path, commit_id, COMMIT_ID_SIZE);
  }

  fprintf(stdout, "\n");

  return 0;
}
