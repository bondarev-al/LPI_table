#include <stdio.h>
#include <sys/io.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PORT_BASE 0x2030
#define PORT_STATUS PORT_BASE + 1

int x = 0, y = 0;

int check_position()
{
  int answ = inb(PORT_STATUS);
  // if (answ != 0x10) return -1;
  return answ;
}

void put_data()
{
  outb((((0xcc >> (x & 3)) & 15) << 4) | ((0xcc >> (y & 3)) & 15), PORT_BASE);
}

int move_x(int d_x, int delay)
{
  int answ = check_position();
  if (d_x > 0)
  {
    //проверка не упремся ли при начале движения
    if (answ & 64) return -1;
    for(int i = 0; i < d_x; i++)
    { 
      x++;
      put_data();
      if (check_position() & 64) return -1;
      usleep(delay);
    }
    outb(0x00, PORT_BASE);
    return 0;
  }

  if (d_x < 0)
  {
    //проверка не упремся ли при начале движения
    if (answ & 32) return -1;
    for(int i = 0; i > d_x; i--)
    { 
      x--;
      put_data();
      if (check_position() & 32) return -1;
      usleep(delay);
    }
    outb(0x00, PORT_BASE);
    return 0;
  }

  outb(0x00, PORT_BASE);
  return 0;
  
}

int move_y(int d_y, int delay)
{
  int answ = check_position();
  if (d_y > 0)
  {
    //проверка не упремся ли при начале движения
    if (answ & 8) return -1;
    for(int i = 0; i < d_y; i++)
    { 
      y++;
      put_data();
      if (check_position() & 8) return -1;
      usleep(delay);
    }
    outb(0x00, PORT_BASE);
    return 0;
  }

  if (d_y < 0)
  {
    // этот выключатель не работает
    //проверка не упремся ли при начале движения
    if (answ & 4) return -1;
    for(int i = 0; i > d_y; i--)
    { 
      y--;
      put_data();
      if (check_position() & 4) return -1;
      usleep(delay);
    }
    outb(0x00, PORT_BASE);
    return 0; 
  }

  outb(0x00, PORT_BASE);
  return 0;
}

int move_to_start(int delay)
{
  move_x(-x, delay);
  move_y(-y, delay);
  return 0;
}

int setup()
{
  outb(0x00, PORT_BASE);
  outb(0x02, PORT_BASE + 2);
}

int main(void)
{
  int result;

  result = ioperm(PORT_BASE, 8, 1);

  if (result != 0)
  {
    fprintf(stderr, "**Error**: ioperm returned error %d, \"%s\"\n", errno, strerror(errno));
    return errno;
  }

  setup();

  move_x(-1000, 2000);
  // move_y(1000, 2000);
  // move_to_start(2000);

  // int answ = check_position();
  // printf("answ = %x\n", answ);
  // printf("st = %i, end = %i\n", (answ & 4), (answ & 8));

  // int answ = 0;
  // while( 1 )
  // {
  //   usleep(1000000);
  //   answ = inb(PORT_STATUS);
  //   printf("answ = %x\n", answ);
  // }

  outb(0x00, PORT_BASE);
  
  return 0;
}
