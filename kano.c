/*** includes ***/
#include <stdio.h>
#include <ctype.h>  // iscntrl()
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>


/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f) // get CTRL+k, for example, q is 113, crtl+q is 17, 113 & 0x1f = 17


/*** data ***/
struct termios orig_termios;


/*** terminal ***/
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[1;1H", 3);

  perror(s); // from stdlib.h, prints a descriptive error message.
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode() {
  // read current terminal to orig_termios
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");

  // atexit is from stdlib.h, execute disableRawMode[restore terminal] when exit
  atexit(disableRawMode);

  // copy terminal state
  struct termios raw = orig_termios;

  // ECHO : bitflag, defined as 00000000000000000000000000001000
  // ICANON : canonical input
  // ISIG : enable signal
  // IEXTEN : enable extended input character processing
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  // IXON : XOFF pause transmission; XON resume transmission
  // ICRNL : map CR(carriage return) to NL(new line)
  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  // OPOST : disable terminal translates "\n" into "\r\n"
  raw.c_oflag &= ~(OPOST);

  raw.c_cflag |= (CS8);

  raw.c_cc[VMIN] = 0; // set minimum number of bytes of input.
  raw.c_cc[VTIME] = 1; // 100ms, it is in tenths of a second.

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}


/*** output ***/
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[1;1H", 3);
}


/*** input ***/
void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[1;1H", 3);
    exit(0);
    break;
  }
}


/*** init ***/
int main () {
  enableRawMode();

  while (1) {
    editorProcessKeypress();
  }
  
  return 0;
}
