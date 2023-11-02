#include "param.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

char buffer[HEIGHT * WIDTH + 1];

int main() {
  int fd = open("/dev/badapple", O_RDONLY);
  if (fd == -1) {
    perror("open(/dev/badapple)");
    return 1;
  }
  while (1) {
    size_t sz = read(fd, buffer, sizeof(buffer) - 1);
    if (sz == 0)
      break;
    buffer[sizeof(buffer) - 1] = 0;
    printf("%s\ec", buffer);
    usleep(31330);
    fflush(stdout);
  }
  close(fd);
}
