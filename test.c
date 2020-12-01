#include <stdio.h>
#include <fcntl.h>		    /* open() */
#include <sys/types.h>		/* open() */
#include <sys/stat.h>		  /* open() */
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <unistd.h>

#define DEVICE "/dev/parport1"
#define END_X_BIT 64
#define START_X_BIT 32
#define END_Y_BIT 8
#define START_Y_BIT 16

struct ZScanTable
{
  int x;        // текущая координата x
  int y;        // текущая координата y
  int start_x;  // не 0, когда стол в начале x
  int end_x;    // не 0, когда стол в конце  x
  int start_y;  // не 0, когда стол в начале y
  int end_y;    // не 0, когда стол в конце  y
  int length_x; // длина оси x
  int length_y; // длина оси y
  int fd;       // дескриптор файла устройства
};

int zscan_table_check_position(struct ZScanTable *table)
{
  int answ;
  ioctl(table->fd, PPRSTATUS, &answ);
  table->start_x = answ & START_X_BIT;
  table->end_x   = answ & END_X_BIT;
  table->start_y = answ & START_Y_BIT;
  table->end_y   = answ & END_Y_BIT;
  return answ;
}

void zscan_table_put_data(struct ZScanTable *table)
{
  int data = (((0xcc >> (table->x & 3)) & 15) << 4) | ((0xcc >> (table->y & 3)) & 15);
  ioctl(table->fd, PPWDATA, &data);	
}

void zscan_table_put_0_data(struct ZScanTable *table)
{
  int data = 0;
  ioctl(table->fd, PPWDATA, &data);	
}

int zscan_table_move_x(struct ZScanTable *table, int d_x, int delay)
{
  int next_x = table->x + d_x;
  int max_x = table->length_x / 2;
  if ( (next_x > max_x) || (next_x < - max_x) )
  {
    fprintf(stderr, "**Error**: Not enough space to move on the x-axis. %i < x < %i  \n", -max_x, max_x);
    return -1;
  }
  zscan_table_check_position(table);
  if (d_x > 0)
  {
    //проверка не упремся ли при начале движения
    if (table->end_x) return -1;
    for(int i = 0; i < d_x; i++)
    {
      table->x++;
      zscan_table_put_data(table);
      zscan_table_check_position(table);
      if (table->end_x) return -1;
      usleep(delay);
    }
    zscan_table_put_0_data(table);
    return 0;
  }

  if (d_x < 0)
  {
    //проверка не упремся ли при начале движения
    if (table->start_x) return -1;
    for(int i = 0; i > d_x; i--)
    {
      table->x--;
      zscan_table_put_data(table);
      zscan_table_check_position(table);
      if (table->start_x) return -1;
      usleep(delay);
    }
    zscan_table_put_0_data(table);
    return 0;
  }

  zscan_table_put_0_data(table);
  return 0;
}

int zscan_table_move_y(struct ZScanTable *table, int d_y, int delay)
{
  int next_y = table->y + d_y;
  int max_y = table->length_y / 2;
  if ( (next_y > max_y) || (next_y < - max_y) )
  {
    fprintf(stderr, "**Error**: Not enough space to move on the y-axis. %i < y < %i  \n", -max_y, max_y);
    return -1;
  }
  zscan_table_check_position(table);
  if (d_y > 0)
  {
    //проверка не упремся ли при начале движения
    if (table->end_y) return -1;
    for(int i = 0; i < d_y; i++)
    {
      table->y++;
      zscan_table_put_data(table);
      zscan_table_check_position(table);
      if (table->end_y) return -1;
      usleep(delay);
    }
    zscan_table_put_0_data(table);
    return 0;
  }

  if (d_y < 0)
  {
    //проверка не упремся ли при начале движения
    if (table->start_y) return -1;
    for(int i = 0; i > d_y; i--)
    {
      table->y--;
      zscan_table_put_data  (table);
      zscan_table_check_position(table);
      if (table->start_y) return -1;
      usleep(delay);
    }
    zscan_table_put_0_data(table);
    return 0;
  }

  zscan_table_put_0_data(table);
  return 0;
}

//не тестировал
int zscan_table_calibrate(struct ZScanTable *table, int delay)
{
  /*
  int max_x = 0;
  zscan_table_check_position(table);
  while (!table->start_x)
  {
    table->x--;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  zscan_table_put_0_data(table);
  zscan_table_check_position(table);
  while (!table->end_x)
  {
    table->x++;
    max_x++;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  zscan_table_put_0_data(table);
  table->length_x = max_x;
  */

  int max_y = 0;
  zscan_table_check_position(table);
  while (!table->start_y)
  {
    table->y--;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  zscan_table_put_0_data(table);
  zscan_table_check_position(table);
  while (!table->end_y)
  {
    table->y++;
    max_y++;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  zscan_table_put_0_data(table);
  table->length_y = max_y;

  //zscan_table_move_x(table, - max_x / 2, 2000);
  zscan_table_move_y(table, - max_y / 2, 2000);
  table->x = 0;
  table->y = 0;

  return 0;
}

int zscan_table_move_to_start(struct ZScanTable *table, int delay)
{
  zscan_table_move_x(table, -table->x, delay);
  zscan_table_move_y(table, -table->y, delay);
  return 0;
}

int zscan_table_setup(struct ZScanTable *table)
{
  int contr = 0x02;
  zscan_table_put_0_data(table);
  ioctl(table->fd, PPWCONTROL, &contr);	
  table->x = 0;
  table->y = 0;
}

int zscan_table_exit(struct ZScanTable *table)
{
  zscan_table_put_0_data(table);
  ioctl(table->fd, PPRELEASE);
	close(table->fd);
}

int main(void)
{
  struct ZScanTable table;
                                                                            
	if ((table.fd = open(DEVICE, O_RDWR)) < 0) {
		fprintf(stderr, "can not open %s\n", DEVICE);
		return 5;
	}
  if (ioctl(table.fd, PPCLAIM)) {
		perror("PPCLAIM");
		close(table.fd);
		return 5;
	}

  zscan_table_setup(&table);
  zscan_table_calibrate(&table, 2000);
  //printf("len_x = %i, len_y = %i\n", table.length_x, table.length_y);

  //table.length_x = 1000;
  //zscan_table_move_x(&table, 500, 2000);
  
  //zscan_table_move_y(&table, 500, 2000);
  
  zscan_table_move_to_start(&table, 2000);
  zscan_table_exit(&table);

  return 0;
}
