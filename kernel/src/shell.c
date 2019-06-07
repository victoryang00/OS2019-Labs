#include <common.h>
#include <shell.h>

const cmd_t cmd_list[] = {
  { "help",  help  },
  { "ping",  ping  },
  { "fuck",  fuck  },
  { "echo",  echo  },
  { "ls",    ls    },
  { "cd",    cd    },
  { "cat",   cat   },
  { "mkdir", mkdir },
  { "rmdir", rmdir },
  { "rm"   , rm    },
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
    while (*cmd == ' ') ++cmd;

    bool succ = false;
    for (int i = 0; i < NR_CMD; ++i) {
      if (!strncmp(cmd, cmd_list[i].name, strlen(cmd_list[i].name))) {
        succ = true;
        char *arg = cmd + strlen(cmd_list[i].name);
        while (*arg == ' ') ++arg;
        cmd_list[i].func(arg, ret);
      }
    }
    if (!succ) sprintf(ret, "invalid command.");
    vfs->write(stdout, ret, strlen(ret));
  }
  Panic("shell cannot exit.");
}

FUNC(help) {
  sprintf(ret, "Available commands: [");
  for (int i = 0; i < NR_CMD; ++i) {
    if (i) strcat(ret, ", ");
    strcat(ret, cmd_list[i].name);
  }
  strcat(ret, "]\n");
}

FUNC(ping) {
  sprintf(ret, "pong\n");
}

FUNC(fuck) {
  sprintf(ret, "nm$l\n");
}

FUNC(echo) {
  sprintf(ret, "%s", arg);
}

FUNC(ls) {
  Panic("not implemented!");
}

FUNC(cd) {
  Panic("not implemented!");
}

FUNC(cat) {
  Panic("not implemented!");
}

FUNC(mkdir) {
  Panic("not implemented!");
}

FUNC(rmdir) {
  Panic("not implemented!");
}

FUNC(rm) {
  Panic("not implemented!");
}
