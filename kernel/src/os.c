#include <common.h>
#include <klib.h>
#include <debug.h>

static void os_init() {
  pmm->init();
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("012345678"[_cpu()]); _putc('\n');
}

static int A[100] = {1, 2, 3, 4, 5, 1, 1, 4, 5, 1, 4, 2, 3, 3, 3, 3, 3, 3, 9, 1, 0, 2};
static bool B[100] = {true, false, true, false, true, false};
static char C[100] = "AKJDjsfajfdioaoxhKJDHSJxckjzbKAJDHSFJHJKEWHJLHDSJKHFKJHSKJDHJEHWUHYROEHD";
static double D[2000] = {0.121343287482374, 28343294, 329748238, 032103212, 820.121, 219382914.4324};

static void yls_test(){
  void *space[100];
  int i;
  for(i=0;i<100;++i){
    space[i]=pmm->alloc(rand()%((1<<10)-1));
  }
  CLog(BG_GREEN, "yls-OK(0) on cpu %d", _cpu());
  for(i=0;i<1000;++i){
    int temp=rand()%10;
    pmm->free(space[temp]);
    space[temp]=pmm->alloc(rand()&((1<<10)-1));
  }
  CLog(BG_GREEN, "yls-OK(1) on cpu %d", _cpu());
  for(i=0;i<100;++i){
    pmm->free(space[i]);
  }
  CLog(BG_GREEN, "yls-OK(2) on cpu %d", _cpu());
}

static void os_run() {
  hello();
  yls_test();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
