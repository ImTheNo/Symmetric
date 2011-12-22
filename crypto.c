#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

char state[8];
char key[8];

u_int get_round_key(int i)
{
  u_int round_key;
  u_int *p_round_key = &round_key;
  switch(i) {
    case 0:
      *(short *)p_round_key = *(short *)key ^ *((short *)key + 1);
      *((short *)p_round_key + 1) = *((short *)key + 2);
      break;
    case 1:
      *(short *)p_round_key = *(short *)key ^ *((short *)key + 2);
      *((short *)p_round_key + 1) = *((short *)key + 3);
      break;
    case 2:
      *(short *)p_round_key = *(short *)key ^ *((short *)key + 3);
      *((short *)p_round_key + 1) = *((short *)key + 1);
      break;
    case 3:
      *(short *)p_round_key = *((short *)key + 1) ^ *((short *)key + 2);
      *((short *)p_round_key + 1) = *(short *)key;
      break;
    case 4:
      *(short *)p_round_key = *((short *)key + 2) ^ *((short *)key + 3);
      *((short *)p_round_key + 1) = *((short *)key + 1);
      break;
    case 5:
      *(short *)p_round_key = *((short *)key + 1) ^ *((short *)key + 3);
      *((short *)p_round_key + 1) = *((short *)key + 2);
      break;
    case 6:
      *((short *)p_round_key + 1) = *(short *)key ^ *((short *)key + 1);
      *(short *)p_round_key = *((short *)key + 1);
      break;
    case 7:
      *((short *)p_round_key + 1) = *((short *)key + 1) ^ *((short *)key + 2);
      *(short *)p_round_key = *(short *)key;
      break;
  }
  round_key = (*(u_short *)p_round_key) * (u_int)*((u_short *)p_round_key + 1);
  *(u_short *)p_round_key = *(u_char *)p_round_key * (u_short)*((u_char *)p_round_key + 1);
  *((u_short *)p_round_key + 1) = *((u_char *)p_round_key + 2) * (u_short)*((u_char *)p_round_key + 3);

  return round_key;
}

u_int function(u_int round_key, u_int block)
{
  int i = 0;

  u_int lmask = 1 << 31;
  u_int rmask = 1;

  for (i = 0; i < 16; i++) 
  {
    if ((lmask & round_key) ^ ((rmask & round_key) << ((15 - i) * 2 + 1))) 
    {
      block = (block & ~lmask & ~rmask) | (((lmask & block) >> ((15 - i) * 2 + 1))) | (((rmask & block) << ((15 - i) * 2 + 1)));
    }
    lmask >>= 1;
    rmask <<= 1;
  }

  return block;
}

void round_func(u_int round_key, u_int *new_left)
{
  *new_left = function(round_key, *(u_int *)state) ^ *((u_int *)state + 1);
  return;
}

void encrypt(char* in_key, char* inf, char* of)
{
  int i;
  u_int round_key;
  u_int new_left;

  int f_in, f_out;

  if ((f_in = open(inf, O_RDONLY)) <= 0 || (f_out = open(of, O_WRONLY | O_CREAT, 0664)) <= 0) 
  {
    printf("can't open file \n");
    return;
  }

  memcpy(key, in_key, 8);
  memset(state, 0, 8);
  while (read(f_in, state, 8) > 0) 
  {
    for (i = 0; i < 8; i++) 
    {
      round_key = get_round_key(i);
      round_func(round_key, &new_left);
      *((u_int *)state + 1) = *(u_int *)state;
      *(u_int *)state = new_left;
    }
    *(u_int *)state = *((u_int *)state + 1);
    *((u_int *)state + 1) = new_left;
    write(f_out, state, 8);
    memset(state, 0, 8);
  }
  close(f_in);
  close(f_out);
  return;
}

void decrypt(char * in_key, char * inf, char * of)
{
  int i;
  u_int round_key;
  u_int new_left;
  int f_in, f_out;

  if ((f_in = open(inf, O_RDONLY)) <= 0 || (f_out = open(of, O_WRONLY | O_CREAT, 0664)) <= 0) 
  {
      printf("can't open file \n");
      return;
  }
  memcpy(key, in_key, 8);
  memset(state, 0, 8);
  while (read(f_in, state, 8) > 0) 
  {
    for (i = 7; i >= 0; i--) 
    {
      round_key = get_round_key(i);
      round_func(round_key, &new_left);
      *((u_int *)state + 1) = *(u_int *)state;
      *(u_int *)state = new_left;
    }
    *(u_int *)state = *((u_int *)state + 1);
    *((u_int *)state + 1) = new_left;
    write(f_out, state, 8);
    memset(state, 0, 8);
  }
  close(f_in);
  close(f_out);
  return;
}
