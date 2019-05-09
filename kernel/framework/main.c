#include <kernel.h>
#include <klib.h>
#include <common.h>

void echo(void* arg) {
  char *name = (char *) arg;
  device_t *tty = dev_lookup(name);
  char text[128], line[128];
  while (1) {
    sprintf(text, "(%s) $ ", name);
    tty->ops->write(tty, 0, text, strlen(text));

    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.", line);
    tty->ops->write(tty, 0, text, strlen(text));
  }
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();
  
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-1", echo, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-2", echo, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-3", echo, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-4", echo, "tty4");

  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
