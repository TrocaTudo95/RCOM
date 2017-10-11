/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

int llopen(int fd) {
    bool STOP = false;
    unsigned char SET[5];
    unsigned char c;
    int state=0;

   printf("llopen: sending SET\n");

    SET[0]=FLAG;
    SET[1]=A;
    SET[2]=C_SET;
    SET[3]=SET[1]^SET[2];
    SET[4]=FLAG;


    if(write(fd,SET,5) != 5){
        printf("llopen: write error\n");
        return -1;
    }

    printf("llopen: receiving UA\n");

    while ((read(fd,&c,1) != -1) && STOP==false) {

      printf("char = %04x\n", c);
      printf("state = %d\n", state);


      switch (state) {
          case 0:
            if(c == FLAG)
              state =1;
          break;

          case 1:
            if(c!= FLAG)
              state =0;

            if(c == A)
              state = 2;
            break;

          case 2:
            if(c!= FLAG)
              state =0;
            else
              state =1;

            if(c == C_UA)
              state =3;
            break;

          case 3:
            if(c!= FLAG)
              state =0;

            if( c== FLAG)
              state =1;

            if( c == (C_UA^A))
              state = 4;
            break;

          case 4:
            if(c != FLAG)
              state =0;

            if(c== FLAG)
              STOP = true;
            break;

          }
        }

    printf("llopen: success\n");
    return 0;
}


int main(int argc, char** argv){
    int fd;
    struct termios oldtio,newtio;


    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

    printf("New termios structure set\n");

    if( llopen(fd) !=0)
      printf("llopen: failed\n");


    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
