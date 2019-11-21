#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
  // preps to run tests by deleting the .beargit directory if it exists
  fs_force_rm_beargit_dir();
  unlink("TEST_STDOUT");
  unlink("TEST_STDERR");
  return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite
 */
int clean_suite(void)
{
  return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
  // This is a very basic test. Your tests should likely do more than this.
  // We suggest checking the outputs of printfs/fprintfs to both stdout
  // and stderr. To make this convenient for you, the tester replaces
  // printf and fprintf with copies that write data to a file for you
  // to access. To access all output written to stdout, you can read
  // from the "TEST_STDOUT" file. To access all output written to stderr,
  // you can read from the "TEST_STDERR" file.
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);
  retval = beargit_add("asdf.txt");
  CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit **commit_list, const char *msg) {
  int retval = beargit_commit(msg);
  CU_ASSERT(0==retval);

  struct commit *new_commit = (struct commit*)malloc(sizeof(struct commit));
  new_commit->next = *commit_list;
  strcpy(new_commit->msg, msg);
  *commit_list = new_commit;
}

void simple_log_test(void)
{
  struct commit* commit_list = NULL;
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);
  FILE *asdf = fopen("asdf.txt", "w");
  fclose(asdf);
  retval = beargit_add("asdf.txt");
  CU_ASSERT(0==retval);
  run_commit(&commit_list, "GO BEARS!1");
  run_commit(&commit_list, "GO BEARS!2");
  run_commit(&commit_list, "GO BEARS!3");

  retval = beargit_log();
  CU_ASSERT(0==retval);

  struct commit *cur_commit = commit_list;

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  FILE *fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  while (cur_commit != NULL) {
    char refline[LINE_SIZE];

    // First line is empty
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    // Second line is commit -- don't check the ID.
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

    // Third line is msg
    sprintf(refline, "    %s\n", cur_commit->msg);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT_STRING_EQUAL(line, refline);

    cur_commit = cur_commit->next;
  }

  // Last line is empty
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line,"\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

  // It's the end of output
  CU_ASSERT(feof(fstdout));
  fclose(fstdout);

  free_commit_list(&commit_list);
}

void beargit_status_should_print_print_correct_output_when_no_files_are_tracked()
{
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);

  retval = beargit_status();
  CU_ASSERT(0==retval);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];
  FILE *fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "Tracked files:\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "0 files total\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(feof(fstdout));
  fclose(fstdout);
}

void beargit_status_should_print_tracked_files()
{
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);

  FILE *file = fopen("file-1.txt", "w");
  fclose(file);
  retval = beargit_add("file-1.txt");
  CU_ASSERT(0==retval);

  file = fopen("file-2.txt", "w");
  fclose(file);
  retval = beargit_add("file-2.txt");
  CU_ASSERT(0==retval);

  retval = beargit_status();
  CU_ASSERT(0==retval);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];
  FILE *fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "Tracked files:\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "  file-1.txt\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "  file-2.txt\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line, "2 files total\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(feof(fstdout));
  fclose(fstdout);
}

void beargit_rm_should_remove_files_from_index()
{
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);

  FILE *file = fopen("file-1.txt", "w");
  fclose(file);
  retval = beargit_add("file-1.txt");
  CU_ASSERT(0==retval);

  file = fopen("file-2.txt", "w");
  fclose(file);
  retval = beargit_add("file-2.txt");
  CU_ASSERT(0==retval);

  retval = beargit_rm("file-1.txt");
  CU_ASSERT(0==retval);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];
  FILE *findex = fopen(".beargit/.index", "r");
  CU_ASSERT_PTR_NOT_NULL(findex);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, findex));
  CU_ASSERT(!strcmp(line, "file-2.txt\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, findex));
  CU_ASSERT(feof(findex));
  fclose(findex);
}

void beargit_rm_should_display_error_when_file_not_in_index()
{
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);

  FILE *file = fopen("file-1.txt", "w");
  fclose(file);
  retval = beargit_add("file-1.txt");
  CU_ASSERT(0==retval);

  retval = beargit_rm("file-non-tracked.txt");
  CU_ASSERT(1==retval);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];
  FILE *fstderr = fopen("TEST_STDERR", "r");
  CU_ASSERT_PTR_NOT_NULL(fstderr);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  CU_ASSERT(!strcmp(line, "ERROR: File file-non-tracked.txt not tracked\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstderr));
  CU_ASSERT(feof(fstderr));
  fclose(fstderr);

  FILE *findex = fopen(".beargit/.index", "r");
  CU_ASSERT_PTR_NOT_NULL(findex);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, findex));
  CU_ASSERT(!strcmp(line, "file-1.txt\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, findex));
  CU_ASSERT(feof(findex));
  fclose(findex);
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
  CU_pSuite pSuite = NULL;
  CU_pSuite pSuite2 = NULL;
  CU_pSuite pSuite3 = NULL;
  CU_pSuite pSuite4 = NULL;
  CU_pSuite pSuite5 = NULL;
  CU_pSuite pSuite6 = NULL;

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add a suite to the registry */
  pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #1 */
  if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  pSuite2 = CU_add_suite("Suite_2", init_suite, clean_suite);
  if (NULL == pSuite2) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #2 */
  if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  pSuite3 = CU_add_suite("Suite_3", init_suite, clean_suite);
  if (NULL == pSuite3) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #3 */
  if (NULL == CU_add_test(pSuite3, "Status with no tracked files test", beargit_status_should_print_print_correct_output_when_no_files_are_tracked))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  pSuite4 = CU_add_suite("Suite_4", init_suite, clean_suite);
  if (NULL == pSuite4) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #4 */
  if (NULL == CU_add_test(pSuite4, "Status with added files test", beargit_status_should_print_tracked_files))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  pSuite5 = CU_add_suite("Suite_5", init_suite, clean_suite);
  if (NULL == pSuite5) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #5 */
  if (NULL == CU_add_test(pSuite5, "Rm successful removal test", beargit_rm_should_remove_files_from_index))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  pSuite6 = CU_add_suite("Suite_6", init_suite, clean_suite);
  if (NULL == pSuite6) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add tests to the Suite #6 */
  if (NULL == CU_add_test(pSuite6, "Rm error test", beargit_rm_should_display_error_when_file_not_in_index))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}

