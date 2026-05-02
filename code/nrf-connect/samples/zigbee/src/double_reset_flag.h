#ifndef _PRST_DOUBLE_RESET_FLAG_H_
#define _PRST_DOUBLE_RESET_FLAG_H_

#include <stdbool.h>
#include <stddef.h>

#define PRST_DOUBLE_RESET_FLAG "prst-rst-count"
#define PRST_DOUBLE_RESET_FLAG_SIZE (sizeof(PRST_DOUBLE_RESET_FLAG))

bool prst_double_reset_flag_matches(const char* buffer, size_t read_len);

#endif  // _PRST_DOUBLE_RESET_FLAG_H_
