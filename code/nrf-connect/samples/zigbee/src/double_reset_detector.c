#include "double_reset_detector.h"

#include <prstlib/macros.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "debug_counters.h"
#include "double_reset_flag.h"

LOG_MODULE_REGISTER(double_reset_detector, CONFIG_LOG_DEFAULT_LEVEL);

static const char *flag_filename = "/lfs/reset_flag";

static struct fs_file_t flag_file;

static int erase_flag() {
  return fs_unlink(flag_filename);
}

void erase_flag_callback(struct k_work *work) {
  LOG_INF("Erasing double reset flag.");
  if (erase_flag() != 0) {
    LOG_ERR("Error deleting flag");
  }
}

K_WORK_DELAYABLE_DEFINE(erase_flag_work, erase_flag_callback);

int prst_detect_double_reset(prst_double_reset_callback_t on_double_reset) {
  fs_file_t_init(&flag_file);

  RET_IF_ERR(fs_open(&flag_file, flag_filename, FS_O_CREATE | FS_O_RDWR));

  char buff[PRST_DOUBLE_RESET_FLAG_SIZE] = {0};
  ssize_t read = fs_read(&flag_file, buff, sizeof(buff));
  if (read < 0) {
    RET_IF_ERR(fs_close(&flag_file));
    return read;
  }

  // Consider it a double reset if the flag is present in the FS.
  if (prst_double_reset_flag_matches(buff, (size_t)read)) {
    RET_IF_ERR(fs_close(&flag_file));
    RET_IF_ERR(erase_flag());
    prst_debug_counters_increment("double_reset");
    return on_double_reset();
  }

  // Rewind file.
  RET_IF_ERR(fs_seek(&flag_file, 0, SEEK_SET));

  // Write the flag and erase it after some time.
  ssize_t written = fs_write(&flag_file, PRST_DOUBLE_RESET_FLAG, PRST_DOUBLE_RESET_FLAG_SIZE);
  if (written != PRST_DOUBLE_RESET_FLAG_SIZE) {
    LOG_ERR("fs_write returned %zd, expected %zu", written, PRST_DOUBLE_RESET_FLAG_SIZE);
    RET_IF_ERR(fs_close(&flag_file));
    return -1;
  }

  RET_IF_ERR(fs_close(&flag_file));

  // Schedule the erasure of the flag after some time.
  RET_CHECK(k_work_schedule(&erase_flag_work, K_SECONDS(5)) == 1,
            "Work not scheduled");

  return 0;
}
