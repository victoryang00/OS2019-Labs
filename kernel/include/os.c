#ifndef __OS_C__
#define __OS_C__

struct os_handler {
  int seq;
  int event;
  handler_t handler;
  struct os_handler *next;
};

#endif
