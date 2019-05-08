#include <kernel.h>
#include <klib.h>
#include <devices.h>

sem_t sem_p, sem_c, mutex;
void producer(void *arg) {
  device_t *tty = dev_lookup("tty1");
  while (1) {
    kmt->sem_wait(&sem_p);
    kmt->sem_wait(&mutex);
    tty->ops->write(tty, 0, "(", 1);
    kmt->sem_signal(&mutex);
    kmt->sem_signal(&sem_c);
  }
}
void customer(void *arg) {
  device_t *tty = dev_lookup("tty1");
  while (1) {
    kmt->sem_wait(&sem_c);
    kmt->sem_wait(&mutex);
    tty->ops->write(tty, 0, ")", 1);
    kmt->sem_signal(&mutex);
    kmt->sem_signal(&sem_p);
  }
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();

  kmt->sem_init(&sem_p, "producer-sem", 1);
  kmt->sem_init(&sem_c, "customer-sem", 0);
  kmt->sem_init(&mutex, "mutex", 1);
  for (int i = 0; i < 8; ++i) {
    kmt->create(pmm->alloc(sizeof(task_t)), "p-task", producer, NULL);
    kmt->create(pmm->alloc(sizeof(task_t)), "c-task", customer, NULL);
  }

  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
