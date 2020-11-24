#include <stdio.h>
#include <sys/io.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PORT_BASE 0x2030
#define PORT_STATUS PORT_BASE + 1
#define END_X_BIT 64 
#define START_X_BIT 32
#define END_Y_BIT 8
#define START_Y_BIT 16

// int x = 0, y = 0;

struct ZScanTable
{
  int x;
  int y;
  int start_x;
  int end_x;
  int start_y;
  int end_y;
  int max_x;
  int max_y;
};

int zscan_table_check_position(struct ZScanTable *table)
{
  int answ = inb(PORT_STATUS);
  table->start_x = answ & START_X_BIT;
  table->end_x   = answ & END_X_BIT;
  table->start_y = answ & START_Y_BIT;
  table->end_y   = answ & END_Y_BIT;
  return answ;
}

void zscan_table_put_data(struct ZScanTable *table)
{
  outb((((0xcc >> (table->x & 3)) & 15) << 4) | ((0xcc >> (table->y & 3)) & 15), PORT_BASE);
}

int zscan_table_move_x(struct ZScanTable *table, int d_x, int delay)
{
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
    outb(0x00, PORT_BASE);
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
    outb(0x00, PORT_BASE);
    return 0;
  }

  outb(0x00, PORT_BASE);
  return 0;
  
}

int zscan_table_move_y(struct ZScanTable *table, int d_y, int delay)
{
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
    outb(0x00, PORT_BASE);
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
    outb(0x00, PORT_BASE);
    return 0;
  }

  outb(0x00, PORT_BASE);
  return 0;
}

//не тестировал
int zscan_table_calibrate(struct ZScanTable *table, int delay)
{
  int max_x = 0;
  zscan_table_check_position(table);
  while (!table->start_x)
  {
    table->x--;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  outb(0x00, PORT_BASE);
  zscan_table_check_position(table);
  while (!table->end_x)
  {
    table->x++;
    max_x++;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  outb(0x00, PORT_BASE);
  table->max_x = max_x;

  int max_y = 0;
  zscan_table_check_position(table);
  while (!table->start_y)
  {
    table->y--;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  outb(0x00, PORT_BASE);
  zscan_table_check_position(table);
  while (!table->end_y)
  {
    table->y++;
    max_y++;
    zscan_table_put_data(table);
    zscan_table_check_position(table);
    usleep(delay);
  }
  outb(0x00, PORT_BASE);
  table->max_y = max_y;

  zscan_table_move_x(table, - max_x / 2, 2000);
  zscan_table_move_y(table, - max_y / 2, 2000);
  table->x = 0;
  table->y = 0;

  return 0;
}

// int finish_move(int start_x, int start_y, int d_x, int d_y, int delay)
// {
//   int dif_x, dif_y;
//   dif_x = x - start_x;
//   dif_y = y - start_y;
//   if (dif_x != d_x) return move_x(d_x - dif_x, delay);
//   else if (dif_y != d_y) return move_y(d_y - dif_y, delay);
// }

// int move_x_y(int d_x, int d_y, int delay)
// {
//   int temp_x = x, temp_y = y;
//   int answ = check_position();
//   if ((d_x >= 0) && (d_y >= 0))
//   {
//     if ((answ & END_X_BIT) || (answ & END_Y_BIT))  return -1;
//     for(int i = 0, j = 0; (i < d_x) && (j < d_y); i++, j++)
//     { 
//       x++;
//       y++;
//       put_data();
//       int temp_answ = check_position();
//       if ((temp_answ & END_X_BIT) || (temp_answ & END_Y_BIT)) return -1;
//       usleep(delay);
//     }
//     outb(0x00, PORT_BASE);
//     return finish_move(temp_x, temp_y, d_x, d_y, delay);
//   }
//   if ((d_x >= 0) && (d_y < 0))
//   {
//     if ((answ & END_X_BIT) || (answ & START_Y_BIT))  return -1;
//     for(int i = 0, j = 0; (i < d_x) && (j > d_y); i++, j--)
//     { 
//       x++;
//       y--;
//       put_data();
//       int temp_answ = check_position();
//       if ((temp_answ & END_X_BIT) || (temp_answ & START_Y_BIT)) return -1;
//       usleep(delay);
//     }
//     outb(0x00, PORT_BASE);
//     return finish_move(temp_x, temp_y, d_x, d_y, delay);
//   }
//   if ((d_x < 0) && (d_y >= 0))
//   {
//     if ((answ & START_X_BIT) || (answ & END_Y_BIT))  return -1;
//     for(int i = 0, j = 0; (i > d_x) && (j < d_y); i--, j++)
//     { 
//       x--;
//       y++;
//       put_data();
//       int temp_answ = check_position();
//       if ((temp_answ & START_X_BIT) || (temp_answ & END_Y_BIT)) return -1;
//       usleep(delay);
//     }
//     outb(0x00, PORT_BASE);
//     return finish_move(temp_x, temp_y, d_x, d_y, delay);
//   }
//   if ((d_x < 0) && (d_y < 0))
//   {
//     if ((answ & START_X_BIT) || (answ & START_Y_BIT))  return -1;
//     for(int i = 0, j = 0; (i > d_x) && (j > d_y); i--, j--)
//     { 
//       x--;
//       y--;
//       put_data();
//       int temp_answ = check_position();
//       if ((temp_answ & START_X_BIT) || (temp_answ & START_Y_BIT)) return -1;
//       usleep(delay);
//     }
//     outb(0x00, PORT_BASE);
//     return finish_move(temp_x, temp_y, d_x, d_y, delay);
//   }
// }

int zscan_table_move_to_start(struct ZScanTable *table, int delay)
{
  zscan_table_move_x(table, -table->x, delay);
  zscan_table_move_y(table, -table->y, delay);
  return 0;
}

int zscan_table_setup(struct ZScanTable *table)
{
  outb(0x00, PORT_BASE);
  outb(0x02, PORT_BASE + 2);
  table->x = 0;
  table->y = 0;
}

int zscan_table_exit(struct ZScanTable *table)
{
  outb(0x00, PORT_BASE);
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

  struct ZScanTable table;

  zscan_table_setup(&table);

  zscan_table_move_y(&table, 500, 2000);
  zscan_table_move_to_start(&table, 2000);

  zscan_table_exit(&table);
  
  return 0;
}
