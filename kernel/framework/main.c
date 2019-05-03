#include <kernel.h>
#include <klib.h>

sem_t sem_p;
sem_t sem_c;
void customer(void *arg) {
  while (1) {
    kmt->sem_wait(&sem_c);
    printf(")");
    kmt->sem_signal(&sem_p);
  }
}
void producer(void *arg) {
  while (1) {
    kmt->sem_wait(&sem_p);
    printf("(");
    kmt->sem_signal(&sem_c);
  }
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();

  // FOR TEST PURPOSE
  // REMOVE WHEN TEST IS PASSED
  kmt->sem_init(&sem_p, "Producer SEM", 1);
  kmt->sem_init(&sem_c, "Customer SEM", 0);
  kmt->create(pmm->alloc(sizeof(task_t)), "Producer Task", producer, NULL);
  kmt->create(pmm->alloc(sizeof(task_t)), "Customer Task", customer, NULL);



  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
