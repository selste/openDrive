#include <stdio.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>
#include <errno.h>
/* We use 
   UART1 READ : FD 4
   UART1 WRITE: FD 5
   UART2 READ : FD 0 (stdin)
   UART2 WRITE: FD 1 (stdout)
*/
FILE * UART1_IN  = NULL;
FILE * UART1_OUT = NULL;
FILE * UART2_IN  = NULL;
FILE * UART2_OUT = NULL;

static inline uint32_t xUART1Port_Init(void) {
  struct stat buf;
  if (fstat(4, &buf) == -1 && errno == EBADF) { /* fd is not open */
    dup2(0,4);
  }
  UART1_IN = fdopen(4, "r");
  if (fstat(5, &buf) == -1 && errno == EBADF) { /* fd is not open */
    dup2(1,5);
  }
  UART1_OUT = fdopen(5, "w");
}

static inline uint32_t xUART2Port_Init(void) {
  UART2_IN = fdopen(0, "r");
  UART2_OUT = fdopen(1, "w");
}

static inline void vUART1Port_SendChar(unsigned char SendChar) {
  fputc(SendChar,UART1_OUT);
  fflush(UART1_OUT);
}

static inline  unsigned char vUART1Port_ReceiveChar(void) {
  return fgetc(UART1_IN);
}


static inline void vUART2Port_SendChar(unsigned char SendChar) {
  fputc(SendChar,UART2_OUT);
  fflush(UART1_OUT);
}

static inline  unsigned char vUART2Port_ReceiveChar(void) {
  return fgetc(UART2_IN);
}

