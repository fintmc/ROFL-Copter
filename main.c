#ifndef __linux__
# error Unsupported OS
#endif

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FPS 15
#define DELTA_TIME (1.f / FPS)

#define ESC "\033" // \e
#define CR  "\015" // \r

#define ESC_ ESC"["

#define cursor_home() printf(ESC_"H")
#define cursor_move(x, y) printf(ESC_"%d;%dH", y, x)
#define cursor_save() printf(ESC_"s")
#define cursor_restore() printf(ESC_"u")
#define cursor_show() printf(ESC_"?25h")
#define cursor_hide() printf(ESC_"?25l")

#define screen_push() printf(ESC_"?47h")
#define screen_pop()  printf(ESC_"?47l")
#define screen_clear() printf(ESC_"J")

char* frames[] = {
  [0] =
    "ROFL:ROFL:LOL:         "   "\n"
    "      ,____Y____       "   "\n"
    " LOL===       []\\      "  "\n"
    "       \\         \\     " "\n"
    "        \\________ ]    "  "\n"
    "         __I___I_____/ "   "\n",
  [1] =
    "         :LOL:ROFL:ROFL"   "\n"
    "  L   ,____Y____       "   "\n"
    "  O ===       []\\      "  "\n"
    "  L    \\         \\     " "\n"
    "        \\________ ]    "  "\n"
    "         __I___I_____/ "   "\n"
};

#define FRAME_WIDTH  24
#define FRAME_HEIGHT 6
#define FRAME_SIZE   (FRAME_WIDTH*FRAME_HEIGHT)

void prep() {
  setvbuf(stdout, NULL, _IONBF, 0); // make stdout non-buffered
  if(!isatty(STDOUT_FILENO)) {
    fprintf(stderr, "Expected a tty at STDOUT (fd=%d)\n", STDOUT_FILENO);
    exit(1);
  }
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK); // make stdin non-blocking
  cursor_save();
  cursor_hide();
  cursor_home();
  screen_push();
  screen_clear();
}

void quit(int code) {
  screen_pop();
  cursor_restore();
  cursor_show();
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK); // make stdin blocking
  exit(code);
}

void on_sigint() {
  quit(0);
}

void handle_input(unsigned* running) {
  char buf[16] = {0};
  ssize_t read_ret = read(STDIN_FILENO, buf, sizeof(buf));
  if(read_ret < 0) {
    // we set stdin to non-blocking earlier
    if(errno == EAGAIN || errno == EWOULDBLOCK) return;
    fprintf(stderr, "Error reading: %s\n", strerror(errno));
    quit(1);
  }
  if(read_ret == 0 /* EOF */ || *buf == 'q' || *buf == 'Q') *running = 0;
}

int main() {
  unsigned running = 1;
  unsigned i = 0;

  prep();
  signal(SIGINT, &on_sigint);
  while(running) {
    handle_input(&running);
    write(STDIN_FILENO, frames[i], FRAME_SIZE);
    cursor_home();
    ++i;
    if(i >= sizeof(frames)/sizeof(frames[0])) i = 0;
    usleep(DELTA_TIME * 1e6);
  }
  quit(0);
  __builtin_unreachable();
  return 1;
}
