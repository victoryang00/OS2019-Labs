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
static char C[100] = "FUCKFUCKFUCKFUCKFUCKFUCKFUCKAJDHSFJHJKEWHJLHDSJKHFKJHSKJDHJEHWUHYROEHD";
static double D[2000] = {0.121343287482374, 28343294, 329748238, 032103212, 820.121, 219382914.4324};

static void kmem_test() {
  int *a = pmm->alloc(sizeof(int) * 100);
  for (int i = 0; i < 100; ++i) {
    a[i] = A[i];
  }
  Assert(memcmp(a, A, sizeof(a)) == 0, "A1 CHECK FAILED");
  bool *b = pmm->alloc(sizeof(bool) * 100);
  for (int i = 0; i < 100; ++i) {
    b[i] = B[i];
  }
  Assert(memcmp(a, A, sizeof(a)) == 0, "A2 CHECK FAILED");
  char *c = pmm->alloc(sizeof(char) * 100);
  for (int i = 0; i < 100; ++i) {
    c[i] = C[i];
  }
  Assert(memcmp(c, C, sizeof(c)) == 0, "C1 CHECK FAILED");
  pmm->free(c);
  double *d = pmm->alloc(sizeof(double) * 2000);
  for (int i = 0; i < 2000; ++i) {
    d[i] = D[i];
  }
  Assert(memcmp(d, D, sizeof(d)) == 0, "D1 CHECK FAILED");
  Assert(memcmp(b, B, sizeof(b)) == 0, "B1 CHECK FAILED");
  pmm->free(b);
  int **e = pmm->alloc(sizeof(int *) * 10);
  for (int i = 0; i < 10; ++i) {
    e[i] = pmm->alloc(sizeof(int) * 100);
  }
  Assert(memcmp(d, D, sizeof(d)) == 0, "D2 CHECK FAILED");
  pmm->free(d);
  for (int i = 0; i < 10; ++i) pmm->free(e[i]);
  Assert(memcmp(a, A, sizeof(a)) == 0, "A3 CHECK FAILED");
  pmm->free(a);
  pmm->free(e);
}

static void os_run() {
  hello();
  kmem_test();
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
