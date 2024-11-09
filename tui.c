#include "wgx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

void tui_init() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
}

int tui_loop(struct strings list, int init) {
  int pos = init + 1;
  int running = 1;
  const int len = list.len + 1;

  const char *tpl;
  if (len <= 9) tpl = "%d|%s";
  else if (len <= 99) tpl = "%02d|%s";
  else if (len <= 999) tpl = "%03d|%s";
  else tpl = "%d|%s";
  
  while (running) {
    clear();

    // print list
    for (int i = -1; i < list.len; i++) {
      const int ix = i + 1;
      if (ix == pos) {
        attron(A_REVERSE);
      }
      const char *item;
      if (ix == 0) {
        item = "off";
      } else {
        item = list.ptr[i];
      }
      mvprintw(ix, 0, tpl, ix, item);
      attroff(A_REVERSE);
    }

    // user input
    int ch = getch();
    switch (ch) {
    case KEY_UP:
      pos = (pos-1 + len) % len;
      break;
    case KEY_DOWN:
      pos = (pos+1) % len;
      break;
    case '\n':
      running = 0;
      break;
    case 'q': case 'Q':
      endwin();
      exit(EXIT_SUCCESS);
    default: if (ch >= '0' && ch <= '9') pos = ch - '0';
    }
  }
  endwin();
  return pos - 1;
}

int main() {
  struct state st;  
  if (state_load(&st) < 0) return EXIT_FAILURE;

  tui_init();
  int next_conf_at = tui_loop(st.confs, st.cur_conf_at);
  
  if (next_conf_at == st.cur_conf_at) {
    printf("nothing to do.\n");
    return EXIT_SUCCESS;
  }

  if (st.cur_conf_at >= 0) {
    printf("----- shutdown ------\n");
    shell_stop_cur(st.cur_use_ref);
    printf("---------------------\n");
  }

  if (next_conf_at >= 0) {
    const char *next_use_ref = st.confs.ptr[next_conf_at];
    printf("----- configure -----\n");
    shell_start_next(next_use_ref);
    printf("---------------------\n");
  }

  state_free(st);
  return EXIT_SUCCESS;
}
