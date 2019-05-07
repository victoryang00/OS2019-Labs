#ifndef __OS_H__
#define __OS_H__

struct os_handler {
  int seq;
  int event;
  handler_t handler;
  struct os_handler *next;
};

#endif
