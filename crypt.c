
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>

#include "crypto.h"

void help(void)
{
  printf("command list: \n");
  printf("    -e : encrypt input file\n");
  printf("    -d : decrypt input file\n");
  printf("    -i : input file name\n");
  printf("    -o : output file name\n");
}

int main(int argc, char *argv[])
{
  char key[9], inputFile[256], outputFile[256], command[8];
  char *cmd[] = {"-e", "-d", "-i", "-o", "-h", NULL};
  
  int crypt = 2, i, ncom;
      struct termio tty, oldtty;
  
  for(i=1; i<argc; i++)
  {
    strcpy(command, argv[i]);
    //printf("argv = %s\n", argv[i]);
    for(ncom=0; cmd[ncom]!=NULL; ncom++ )
    if (!strcmp(command, cmd[ncom])) break;
  
    if (cmd[ncom]==NULL) 
    printf("Error! Unknown command.\n");
    switch (ncom)
    {
      case 0 : {crypt = 0; break;}
      case 1 : {crypt = 1; break;}
      case 2 : {i++; strcpy(inputFile, argv[i]); break;}
      case 3 : {i++; strcpy(outputFile, argv[i]); break;}
      case 4 : {help(); return 0;}
      default : {printf("Unknown command!\n"); return -1;}
    }
  }
  
  //Save the old tty settings and get rid of echo for new tty
  
  ioctl(0, TCGETA, &oldtty);
  tty = oldtty;
  tty.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 0;
  ioctl(0, TCSETA, &tty);
  
  //printf("key: %s\n", key);
  printf("Write key: \n");
  
  i = 0;
  while (i < 9  && (key[i] = getchar()) != '\n')
  {
    i++;
    printf("*");
  }
  if (key[i] == '\n') 
  {
    printf("\n");
  }
  else
  {
    printf("ERROR: your key is too long\n");
    ioctl(0, TCSETA, &oldtty); 
    return -1;
  }
  
  key[i] = 0;
  
  //reset the old settings
  
  ioctl(0, TCSETA, &oldtty); 

  if(crypt == 0) encrypt(key, inputFile, outputFile);
    else if(crypt == 1) decrypt(key, inputFile, outputFile);
  
  return 0;
}

