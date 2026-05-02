#include "double_reset_flag.h"

#include <string.h>

bool prst_double_reset_flag_matches(const char* buffer, size_t read_len) {
  return read_len == PRST_DOUBLE_RESET_FLAG_SIZE &&
         memcmp(buffer, PRST_DOUBLE_RESET_FLAG, PRST_DOUBLE_RESET_FLAG_SIZE) == 0;
}
