#pragma once

#define WG_CONF_PATH "/etc/wireguard"
#define WG_CONF_SUFFIX ".conf"

void *clone_data(const void *src, int len);

// str
int str_has_suffix(const char *str, const char *suffix);
const char *str_trim_suffix(const char *src, const char *suffix);

// str[]
struct strings {
  int len;
  char **ptr;
};
void strings_free(struct strings strs);
void strings_sort(struct strings strs);
void strings_unique_sorted(struct strings *strs);
// find matched string at post=N in second array
const char *strings_match_at(struct strings a, struct strings b, int *at);

// wg
int list_all_ifs(struct strings *out);
int list_all_confs(struct strings *out);

// shell
int shell_exec(const char *cmd);
int shell_stop_cur(const char *cur_use);
int shell_start_next(const char *next_use);

// state
struct state {
  struct strings ifs;
  struct strings confs;
  int cur_conf_at;
  const char *cur_use_ref;
};

int state_load(struct state *);
void state_free(struct state);
