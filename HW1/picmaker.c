#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
  umask(0);
  int fd = open("image.ppm", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  char header[] = "P3 500 500 255\n";
  write(fd, header, 15);
  char line[20];
  int m, n;
  int r, g, b;
  
  for (m = 0; m < 500; m++){
    for (n = 0; n < 500; n++){
      r = (m * n) % 256;
      g = n % 128 + 64;
      b = (m / (n + 1)) % 256;
      memset(line, 0, 20);
      sprintf(line, "%d %d %d\n", r, g, b);
      write(fd, line, strlen(line));
      //printf("printed one pixel \n");
    }
  }
  
  close(fd);

  return 0;
}
