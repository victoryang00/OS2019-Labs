#include <common.h>
#include <shell.h>
#include <file.h>
#include <vfs.h>

const cmd_t cmd_list[] = {
  { "help",  help  },
  { "ping",  ping  },
  { "fuck",  fuck  },
  { "echo",  echo  },
  { "ls",    ls    },
  { "pwd",   pwd   },
  { "cd",    cd    },
  { "cat",   cat   },
  { "mkdir", mkdir },
  { "rmdir", rmdir },
  { "rm"   , rm    },
};
const int NR_CMD = sizeof(cmd_list) / sizeof(cmd_t);

void shell_task(void *arg) {
  int tty_id = (int)arg;
  char buf[256] = "";
  char pwd[256] = "";
  char cmd[256] = "";
  char ret[256] = "";

  sprintf(buf, "/dev/tty%d", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);

  sprintf(buf, "Welcome to sHELL.\nType [help] for help.\n\n");
  vfs->write(stdout, buf, strlen(buf));

  sprintf(pwd, "/");
  while (true) {
    sprintf(buf, "(tty%d) %s\n -> ", tty_id, pwd);
    vfs->write(stdout, buf, strlen(buf));

    ssize_t nread = vfs->read(stdin, cmd, sizeof(cmd));
    cmd[nread - 1] = '\0';
    char *arg = cmd;
    while (*arg == ' ') ++arg;

    bool succ = false;
    for (int i = 0; i < NR_CMD; ++i) {
      if (!strncmp(arg, cmd_list[i].name, strlen(cmd_list[i].name))) {
        succ = true;
        arg += strlen(cmd_list[i].name);
        while (*arg == ' ') ++arg;
        cmd_list[i].func(arg, pwd, ret);
      }
    }
    if (!succ) sprintf(ret, "Invalid command.\n");
    vfs->write(stdout, ret, strlen(ret));
    vfs->write(stdout, "\n", 1);
  }
  Panic("shell cannot exit.");
}

bool get_dir(const char *arg, const char *pwd, char *dir) {
  if (arg[0] == '/') {
    sprintf(dir, "%s", arg);
  } else {
    sprintf(dir, "%s%s", pwd, arg);
  }

  size_t cur = 0;
  size_t len = strlen(dir);
  for (size_t i = 0; i <= len; ++i, ++cur) {
    if (!strncmp(dir + i, "//", 2)) return false;
    if (!strncmp(dir + i, "./", 2)) i += 2;
    if (!strncmp(dir + i, "../", 3)) {
      i += 3;
      while (cur > 0 && dir[cur] != '/') --cur;
    }
    if (i > len) break;
    dir[cur] = dir[i];
  }
  return true;
}

FUNC(help) {
  sprintf(ret, "Available commands: \n");
  for (int i = 0; i < NR_CMD; ++i) {
    strcat(ret, " - ");
    strcat(ret, cmd_list[i].name);
    strcat(ret, "\n");
  }
  strcat(ret, "\n");
}

FUNC(ping) {
  sprintf(ret, "pong\n");
}

FUNC(fuck) {
  sprintf(ret, "nm$l\n");
}

FUNC(echo) {
  sprintf(ret, "%s\n", arg);
}

FUNC(pwd) {
  sprintf(ret, "%s\n", pwd);
}

FUNC(ls) {
  inode_t *cur = inode_search(&root, pwd);
  cur->ops->readdir(cur, ret);
}

FUNC(cd) {
  char dir[256] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.");
  }

  if (vfs->access(dir, 0)) {
    sprintf(ret, "Cannot access %s.");
  } else {
    strcpy(pwd, dir);
    sprintf(ret, "Current dir = %s", dir);
  }
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
