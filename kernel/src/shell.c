#include <common.h>

const cmd_t cmd_list[] = {
  { "ping ",  ping  },
  { "fuck ",  fuck  },
  { "echo ",  echo  },
  { "ls ",    ls    },
  { "cd ",    cd    },
  { "cat ",   cat   },
  { "mkdir ", mkdir },
  { "rmdir ", rmdir },
  { "rm "   , rm    },
};
const int NR_CMD = sizeof(cmd_list) / sizeof(cmd_t);

void shell_task(void *arg) {
  int tty_id = (int)arg;
  char buf[128] = "";
  char cmd[256] = "";
  char ret[256] = "";

  sprintf(buf, "/dev/tty%d", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);

  sprintf(buf, "(tty%d) $ ", tty_id);
  while (true) {
    vfs->write(stdout, buf, strlen(buf));
    vfs->read(stdin, cmd, sizeof(cmd));

    bool succ = false;
    for (int i = 0; i < NR_CMD; ++i) {
      if (!strcmp(buf, cmd_list[i].name)) {
        succ = true;
        char *arg = cmd + strlen(cmd_list[i].name);
        cmd_list[i].func(arg, ret);
      }
    }
    if (!succ) sprinf(ret, "invalid command.");
    vfs->write(stdout, ret, strlen(ret));
    vfs->write(stdout, "\n", 1);
  }
  Panic("shell cannot exit.");
}


