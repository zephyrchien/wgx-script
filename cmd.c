#include "wgx.h"

#include <stdio.h>
#include <stdlib.h>

void print_current(struct strings confs, int cur_conf_at) {
  for (int i = 0; i < confs.len; i++) {
    const char *c = confs.ptr[i];
    if (i == cur_conf_at) {
      printf("%d|%s <--(current)\n", i, c);
    } else {
      printf("%d|%s\n", i, c);
    }
  }
}

int read_new_conf(struct strings confs) {
  printf("> pick one of the above\n");
  int n;
  if (!scanf("%d", &n)) return -1;
  return n;
}


int main(int agrc, const char **argv) {
  struct state st;
  if (state_load(&st) < 0) return EXIT_FAILURE;
  
  // interactive
  print_current(st.confs, st.cur_conf_at);
  int next_conf_at = read_new_conf(st.confs);
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
