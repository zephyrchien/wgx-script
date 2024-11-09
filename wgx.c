#include "wgx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ifaddrs.h>

void* clone_data(const void *src, int len) {
  void *dst = (void*)malloc(len);
  memcpy(dst, src, len);
  return dst;
}

int str_has_suffix(const char *str, const char *suffix) {
  char c = suffix[0];
  const char *s = str;
  // find starting char
  while (*s != 0) { if (*s++ == c) break; }
  // compare the remaining chars
  return strcmp(s, ++suffix) == 0;
}

const char* str_trim_suffix(const char *src, const char *suffix) {
  // deduce new length
  int src_len = strlen(src);
  int suffix_len = strlen(suffix);
  int dst_len = src_len - suffix_len;
  if (dst_len <= 0) return NULL;

  // create new string
  char *dst = (char *)clone_data(src, dst_len+1);
  *(dst + dst_len) = 0;

  return dst;
}

void strings_free(struct strings strs) {
  for (int i = 0; i < strs.len; i++) {
    free(strs.ptr[i]);
  }
  free(strs.ptr);
}

int _cmp(const void *a, const void* b) {
   const char *as = *((const char**)a);
   const char *bs = *((const char**)b);
   return strcmp(as, bs);
}
void strings_sort(struct strings strs) {
  qsort(strs.ptr, strs.len, sizeof(char*), _cmp);
}

void strings_unique_sorted(struct strings *strs) {
  if (strs->len < 2) return;

  int x = 0;
  for (int i = 1; i < strs->len; i++) {
    char *this_str = strs->ptr[x];
    char *next_str = strs->ptr[i];
    if (strcmp(this_str, next_str) == 0) {
      free(next_str);
    } else {
      strs->ptr[++x] = next_str;
    }
  }
  strs->len = x+1;
}

const char* strings_match_at(struct strings a, struct strings b, int *at) {
  for (int i = 0; i < a.len; i++) {
    const char *as = a.ptr[i];
    // binary
    int beg = 0, end = b.len;
    while (beg < end) {
      int mid = (beg + end) / 2;
      const char *bs = b.ptr[mid];
      int cmp_gap = strcmp(as, bs);
      if (cmp_gap == 0) {
        *at = mid;
        return as;
      }
      if (cmp_gap < 0) {
        end = mid;
      } else {
        beg = mid+1;
      }
    }
  }
  *at = -1;
  return NULL;
}

int list_all_ifs(struct strings *out) {
  int total = 0;
  const char *all_names[1024] = {0};
  
  // enter
  struct ifaddrs *ifs;
  int ret = getifaddrs(&ifs);
  if (ret < 0) {
    perror("getifaddrs");
    return -1;
  }
  // traverse
  struct ifaddrs *it;
  for (it = ifs; it != NULL; it = it->ifa_next) {
    if (it->ifa_addr == NULL) continue;
    sa_family_t family = it->ifa_addr->sa_family;
    if (family != AF_INET && family != AF_INET6) continue;

    int len = strlen(it->ifa_name);
    all_names[total++] = clone_data(it->ifa_name, len+1);
  }
  // cleanup
  freeifaddrs(ifs);

  void *data = clone_data(all_names, sizeof(char*)*total);
  out->ptr = data; out->len = total;
  return 0;
}

int list_all_confs(struct strings *out) {
  int total = 0;
  const char *all_confs[1024] = {0};
  
  // enter dir
  DIR *dir = opendir(WG_CONF_PATH);
  if (dir == NULL) {
    perror("opendir");
    return -1;
  }
  // traverse
  struct dirent *it;
  while ((it = readdir(dir)) != NULL) {
    if(!str_has_suffix(it->d_name, WG_CONF_SUFFIX)) continue;

    const char *name = str_trim_suffix(it->d_name, WG_CONF_SUFFIX);
    all_confs[total++] = name;
  }
  // leave dir
  closedir(dir);

  void *data = clone_data(all_confs, sizeof(char*)*total);
  out->ptr = data; out->len = total;
  return 0;
}

int shell_exec(const char *cmd) {
  FILE *fp = popen(cmd, "r");
  if (fp == NULL) {
    return -1;
  }

  char buf[1024] = {0};
  while (fgets(buf, 1024, fp) != NULL) {
    printf("%s", buf);
  }

  pclose(fp);
  return 0;
}

int shell_stop_cur(const char *cur_use) {
  char cmd[256];
  snprintf(cmd, 256, "wg-quick down %s", cur_use);
  if (shell_exec(cmd) < 0) {
    printf("failed to run: %s\n", cmd);
    return -1;
  }
  return 0;
}

int shell_start_next(const char *next_use) {
  char cmd[256];
  snprintf(cmd, 256, "wg-quick up %s", next_use);
  if (shell_exec(cmd) < 0) {
    printf("failed to run: %s\n", cmd);
    return -1;
  }
  return 0;
}

int state_load(struct state *st) {
  struct strings confs;
  if (list_all_confs(&confs) < 0) return -1;
  strings_sort(confs);

  struct strings ifs;
  if (list_all_ifs(&ifs) < 0) return -1;
  strings_sort(ifs); strings_unique_sorted(&ifs);

  int cur_conf_at;
  const char *cur_use_ref = strings_match_at(ifs, confs, &cur_conf_at);

  st->ifs = ifs;
  st->confs = confs;
  st->cur_conf_at = cur_conf_at;
  st->cur_use_ref = cur_use_ref;
  return 0;
}

void state_free(struct state st) {
  strings_free(st.ifs);
  strings_free(st.confs);
}
