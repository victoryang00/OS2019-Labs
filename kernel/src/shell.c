#include <common.h>

void shell_task(void *arg) {
  int tty_id = (int)arg;
  char buf[128] = "";

  sprintf(buf, "/dev/tty%d", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);
  while (true) {
    ssize_t nread = vfs->read(stdin, buf, sizeof(buf));
    vfs->write(stdout, buf, nread);
  }
  Panic("shell cannot exit.");
}
