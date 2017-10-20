#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "data_link.h"

#define BAUDRATE B38400

int main(int argc, char** argv){
   int fd;
   struct termios oldtio,newtio;


   if ( (argc < 3) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0)) ||
        ((strcmp("TRANSMITTER", argv[2]) !=0) &&  (strcmp("RECEIVER", argv[2]) !=0))) {

     printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial /dev/ttyS1 TRANSMITTER\n");
     exit(1);
   }

  init_link_layer(3,3,BAUDRATE);


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

   newtio.c_cc[VTIME]    = 5;   /* inter-character timer unused */
   newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

 /*
   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
   leitura do(s) pr�ximo(s) caracter(es)
 */

   tcflush(fd, TCIOFLUSH);

   if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
     perror("tcsetattr");
     exit(-1);
   }

   printf("New termios structure set\n");

   //if( llopen(fd,argv[2]) !=0)
  //    printf("llopen: failed\n");

   if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
     perror("tcsetattr");
     exit(-1);
   }

   close(fd);
   return 0;
}