#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "kronut_test.h"
#include "midi_data_test.h"
#include "kstring_test.h"
#include "mock_kronos_test.h"
#include "editor_test.h"

// ================ running tests ================

test_results results;

void test_results_init() {
  results.num_tests = results.num_errors = 0;
}

void test_passed() {
  ++results.num_tests;
  printf(".");
}

void test_failed() {
  ++results.num_tests;
  ++results.num_errors;
  printf("*");
}

int test_num_errors() {
  return results.num_errors;
}

void print_time() {
  rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  printf("\n\nFinished in %ld.%06ld seconds\n", usage.ru_utime.tv_sec,
         (long)usage.ru_utime.tv_usec);
}

void print_results() {
  printf("\nTests run: %d, tests passed: %d, tests failed: %d\n",
         results.num_tests, results.num_tests - results.num_errors,
         results.num_errors);
  printf("done\n");
}

void run_tests() {
  test_midi_data();
  test_kstring();
  test_mock_kronos();
  test_editor();
}

void run_tests_and_print_results() {
  run_tests();
  print_time();
  print_results();
}

// ================ main ================

int main(int argc, const char **argv) {
  run_tests_and_print_results();
  exit(results.num_errors == 0 ? 0 : 1);
  return 0;
}
