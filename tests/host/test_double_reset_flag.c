#include "double_reset_flag.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expr); \
      failures++;                                                              \
    }                                                                          \
  } while (0)

int main(void) {
  const char exact[] = PRST_DOUBLE_RESET_FLAG;
  CHECK(sizeof(exact) == PRST_DOUBLE_RESET_FLAG_SIZE);
  CHECK(prst_double_reset_flag_matches(exact, sizeof(exact)));
  CHECK(!prst_double_reset_flag_matches(exact, sizeof(exact) - 1));

  char wrong[PRST_DOUBLE_RESET_FLAG_SIZE] = PRST_DOUBLE_RESET_FLAG;
  wrong[0] = 'x';
  CHECK(!prst_double_reset_flag_matches(wrong, sizeof(wrong)));

  char longer[PRST_DOUBLE_RESET_FLAG_SIZE + 1] = {0};
  memcpy(longer, exact, sizeof(exact));
  CHECK(!prst_double_reset_flag_matches(longer, sizeof(longer)));

  return failures == 0 ? 0 : 1;
}
