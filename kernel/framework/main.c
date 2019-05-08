#include <kernel.h>
#include <klib.h>

void echo_task(void *arg) {
  char *name = (char *) arg;
  char text[128] = "", line[128] = "";
  device_t *tty = device_lookup(name);
  while (1) {
    sprintf(text, "(%s) $ ", name);
    tty->ops->write(tty, 0, text, strlen(text));

    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line);
    tty->ops->write(tty, 0, text, strlen(text));
  }
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();

  kmt->create(pmm->alloc(sizeof(task_t)), "echo-1", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-2", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-3", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "echo-4", echo_task, "tty4");

  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
