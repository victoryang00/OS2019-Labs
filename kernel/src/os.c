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

void *space[4][1000];
static void yls_test(){
  int i;
  for(i=0;i<1000;++i){
    printf("%d - yls0 %d addr=%d\n, heap=[%p, %p)\n", _cpu(), i, space + i, _heap.start, _heap.end);
    size_t size = rand() % ((1 << 10) - 1) + 1;
    //Assert(size != 0, "size is 0 on round %d", i);
    space[_cpu()][i]=pmm->alloc(size);
  }
  for(i=0;i<1000;++i){
    printf("%d - yls2 %d addr=%p, heap=[%p, %p)\n", _cpu(), i, space[i], _heap.start, _heap.end);
    pmm->free(space[_cpu()][i]);
  }
  CLog(BG_GREEN, "OKOK on cpu %d", _cpu());
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
