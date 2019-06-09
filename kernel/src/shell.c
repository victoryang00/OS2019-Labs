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
  { "write", write },
  { "mkdir", mkdir },
  { "rmdir", rmdir },
  { "rm"   , rm    },
};
const int NR_CMD = sizeof(cmd_list) / sizeof(cmd_t);

void shell_task(void *arg) {
  int tty_id = (int)arg;
  char buf[512] = "";
  char pwd[512] = "";
  char cmd[512] = "";
  char ret[512] = "";

  sprintf(buf, "/dev/tty%d", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);
  Assert(stdin >= 0, "failed to open stdin");
  Assert(stdout >= 0, "failed to open stdout");

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
    memset(ret, 0, sizeof(ret));

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
  char buf[512] = "";
  if (arg[0] == '/') {
    sprintf(buf, "%s", arg);
  } else {
    if (pwd[strlen(pwd) - 1] == '/') {
      sprintf(buf, "%s%s", pwd, arg);
    } else {
      sprintf(buf, "%s/%s", pwd, arg);
    }
  }

  size_t pos = 0;
  size_t cur = 0;
  size_t len = strlen(buf);
  while (pos <= len) {
    if (!strncmp(buf + pos, "//", 2)) {
      return false;
    } else if (!strncmp(buf + pos, "/./", 3)) {
      pos += 2;
      continue;
    } else if (!strncmp(buf + pos, "/../", 4)) {
      pos += 3;
      if (cur > 0 && dir[cur] == '/') --cur;
      while (cur > 0 && dir[cur] != '/') --cur;
      continue;
    } else {
      dir[cur] = buf[pos];
      ++pos, ++cur;
    }
  }
  dir[cur] = '\0';
  len = strlen(dir);
  if (len > 1 && dir[len - 1] == '/') dir[len - 1] = '\0';
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
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_RDONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      vfs->readdir(dir, ret);
    }
  }
}

FUNC(cd) {
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_RDONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      inode_t *ip = inode_search(root, dir);
      if (ip->type == TYPE_MNTP || ip->type == TYPE_DIRC) {
        strcpy(pwd, dir);
        sprintf(ret, "Directory changed to %s.\n", dir);
      } else {
        sprintf(ret, "Invalid inode type of %s: %s.\n", dir, inode_types_human[ip->type]);
      }
    }
  }
}

FUNC(cat) {
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_RDONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      int fd = vfs->open(dir, O_RDONLY);
      if (fd == -1) {
        sprintf(ret, "VFS ERROR: open failed.");
      } else {
        vfs->read(fd, ret, 512);
        vfs->close(fd);
      }
    }
  }
}

FUNC(write) {
  char dir[512] = "";
  char arg1[512] = "";
  const char *arg2 = arg;
  for (size_t i = 0; *arg2 != '\0'; ++i, ++arg2) {
    arg1[i] = *arg2;
    if (*arg2 == ' ') {
      arg1[i] = '\0';
      ++arg2;
      break;
    }
  }

  if (!get_dir(arg1, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_WRONLY | O_CREAT)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      int fd = vfs->open(dir, O_WRONLY | O_CREAT);
      if (fd == -1) {
        sprintf(ret, "VFS ERROR: open failed.");
      } else {
        vfs->write(fd, (void *)arg2, strlen(arg2));
        vfs->write(fd, "\n", 1);
        vfs->close(fd);
      }
    }
  }
}

FUNC(mkdir) {
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_WRONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      if (!vfs->mkdir(dir, ret)) {
        sprintf(ret, "Successfully created folder %s.", dir);
      } else {
        sprintf(ret, "VFS ERROR: mkdir failed.");
      }
    }
  }
}

FUNC(rmdir) {
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_WRONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      if (!vfs->mkdir(dir, ret)) {
        sprintf(ret, "Successfully removed folder %s.", dir);
      } else {
        sprintf(ret, "VFS ERROR: rmdir failed.");
      }
    }
  }
}

FUNC(rm) {
  char dir[512] = "";
  if (!get_dir(arg, pwd, dir)) {
    sprintf(ret, "Invalid directory address.\n");
  } else {
    if (vfs->access(dir, O_WRONLY)) {
      sprintf(ret, "Cannot access %s.\n");
    } else {
      if (!vfs->rm(dir, ret)) {
        sprintf(ret, "Successfully removed %s.", dir);
      } else {
        sprintf(ret, "VFS ERROR: rm failed.");
      }
    }
  }
}
