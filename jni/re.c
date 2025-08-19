#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "re.h"

#define MAX_BRANCHES 100
#define MAX_BRACKETS 100
#define FAIL_IF(condition, error_code) if (condition) return (error_code)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif

struct bracket_pair {
  const char *ptr;
  int len;
  int branches;
  int num_branches;
};

struct branch {
  int bracket_index;
  const char *schlong;
};

struct regex_info {
  struct bracket_pair brackets[MAX_BRACKETS];
  int num_brackets;

  struct branch branches[MAX_BRANCHES];
  int num_branches;

  struct re_cap *caps;
  int num_caps;

  int flags;
};

static int is_metacharacter(const unsigned char *s) {
  static const char *metacharacters = "^$().[]*+?|\\Ssdbfnrtv";
  return strchr(metacharacters, *s) != NULL;
}

static int op_len(const char *re) {
  return re[0] == '\\' && re[1] == 'x' ? 4 : re[0] == '\\' ? 2 : 1;
}

static int set_len(const char *re, int re_len) {
  int len = 0;

  while (len < re_len && re[len] != ']') {
    len += op_len(re + len);
  }

  return len <= re_len ? len + 1 : -1;
}

static int get_op_len(const char *re, int re_len) {
  return re[0] == '[' ? set_len(re + 1, re_len - 1) + 1 : op_len(re);
}

static int is_quantifier(const char *re) {
  return re[0] == '*' || re[0] == '+' || re[0] == '?';
}

static int toi(int x) {
  return isdigit(x) ? x - '0' : x - 'W';
}

static int hextoi(const unsigned char *s) {
  return (toi(tolower(s[0])) << 4) | toi(tolower(s[1]));
}

static int match_op(const unsigned char *re, const unsigned char *s,
                    struct regex_info *info) {
  int result = 0;
  switch (*re) {
    case '\\':
      switch (re[1]) {
        case 'S': FAIL_IF(isspace(*s), RE_NO_MATCH); result++; break;
        case 's': FAIL_IF(!isspace(*s), RE_NO_MATCH); result++; break;
        case 'd': FAIL_IF(!isdigit(*s), RE_NO_MATCH); result++; break;
        case 'b': FAIL_IF(*s != '\b', RE_NO_MATCH); result++; break;
        case 'f': FAIL_IF(*s != '\f', RE_NO_MATCH); result++; break;
        case 'n': FAIL_IF(*s != '\n', RE_NO_MATCH); result++; break;
        case 'r': FAIL_IF(*s != '\r', RE_NO_MATCH); result++; break;
        case 't': FAIL_IF(*s != '\t', RE_NO_MATCH); result++; break;
        case 'v': FAIL_IF(*s != '\v', RE_NO_MATCH); result++; break;

        case 'x':
          FAIL_IF(hextoi(re + 2) != *s, RE_NO_MATCH);
          result++;
          break;

        default:
          FAIL_IF(re[1] != s[0], RE_NO_MATCH);
          result++;
          break;
      }
      break;

    case '|': FAIL_IF(1, RE_INTERNAL_ERROR); break;
    case '$': FAIL_IF(1, RE_NO_MATCH); break;
    case '.': result++; break;

    default:
      if (info->flags & RE_IGNORE_CASE) {
        FAIL_IF(tolower(*re) != tolower(*s), RE_NO_MATCH);
      } else {
        FAIL_IF(*re != *s, RE_NO_MATCH);
      }
      result++;
      break;
  }

  return result;
}

static int match_set(const char *re, int re_len, const char *s,
                     struct regex_info *info) {
  int len = 0, result = -1, invert = re[0] == '^';

  if (invert) re++, re_len--;

  while (len <= re_len && re[len] != ']' && result <= 0) {
    if (re[len] != '-' && re[len + 1] == '-' && re[len + 2] != ']' &&
        re[len + 2] != '\0') {
      result = info->flags &  RE_IGNORE_CASE ?
        tolower(*s) >= tolower(re[len]) && tolower(*s) <= tolower(re[len + 2]) :
        *s >= re[len] && *s <= re[len + 2];
      len += 3;
    } else {
      result = match_op((const unsigned char *) re + len, (const unsigned char *) s, info);
      len += op_len(re + len);
    }
  }
  return (!invert && result > 0) || (invert && result <= 0) ? 1 : -1;
}

static int doh(const char *s, int s_len, struct regex_info *info, int bi);

static int bar(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info, int bi) {
  int i, j, n, step;

  for (i = j = 0; i < re_len && j <= s_len; i += step) {

    step = re[i] == '(' ? info->brackets[bi + 1].len + 2 :
      get_op_len(re + i, re_len - i);

    FAIL_IF(is_quantifier(&re[i]), RE_UNEXPECTED_QUANTIFIER);
    FAIL_IF(step <= 0, RE_INVALID_CHARACTER_SET);

    if (i + step < re_len && is_quantifier(re + i + step)) {
      if (re[i + step] == '?') {
        int result = bar(re + i, step, s + j, s_len - j, info, bi);
        j += result > 0 ? result : 0;
        i++;
      } else if (re[i + step] == '+' || re[i + step] == '*') {
        int j2 = j, nj = j, n1, n2 = -1, ni, non_greedy = 0;

        ni = i + step + 1;
        if (ni < re_len && re[ni] == '?') {
          non_greedy = 1;
          ni++;
        }

        do {
          if ((n1 = bar(re + i, step, s + j2, s_len - j2, info, bi)) > 0) {
            j2 += n1;
          }
          if (re[i + step] == '+' && n1 < 0) break;

          if (ni >= re_len) {
            nj = j2;
          } else if ((n2 = bar(re + ni, re_len - ni, s + j2,
                               s_len - j2, info, bi)) >= 0) {
            nj = j2 + n2;
          }
          if (nj > j && non_greedy) break;
        } while (n1 > 0);

        if (n1 < 0 && n2 < 0 && re[i + step] == '*' &&
            (n2 = bar(re + ni, re_len - ni, s + j, s_len - j, info, bi)) > 0) {
          nj = j + n2;
        }

        FAIL_IF(re[i + step] == '+' && nj == j, RE_NO_MATCH);

        FAIL_IF(nj == j && ni < re_len && n2 < 0, RE_NO_MATCH);

        return nj;
      }
      continue;
    }

    if (re[i] == '[') {
      n = match_set(re + i + 1, re_len - (i + 2), s + j, info);
      
      FAIL_IF(n <= 0, RE_NO_MATCH);
      j += n;
    } else if (re[i] == '(') {
      n = RE_NO_MATCH;
      bi++;
      FAIL_IF(bi >= info->num_brackets, RE_INTERNAL_ERROR);
      
      if (re_len - (i + step) <= 0) {
        n = doh(s + j, s_len - j, info, bi);
      } else {
        int j2;
        for (j2 = 0; j2 <= s_len - j; j2++) {
          if ((n = doh(s + j, s_len - (j + j2), info, bi)) >= 0 &&
              bar(re + i + step, re_len - (i + step),
                  s + j + n, s_len - (j + n), info, bi) >= 0) break;
        }
      }

      FAIL_IF(n < 0, n);
      if (info->caps != NULL && n > 0) {
        info->caps[bi - 1].ptr = s + j;
        info->caps[bi - 1].len = n;
      }
      j += n;
    } else if (re[i] == '^') {
      FAIL_IF(j != 0, RE_NO_MATCH);
    } else if (re[i] == '$') {
      FAIL_IF(j != s_len, RE_NO_MATCH);
    } else {
      FAIL_IF(j >= s_len, RE_NO_MATCH);
      n = match_op((const unsigned char *) (re + i), (const unsigned char *) (s + j), info);
      FAIL_IF(n <= 0, n);
      j += n;
    }
  }

  return j;
}

static int doh(const char *s, int s_len, struct regex_info *info, int bi) {
  const struct bracket_pair *b = &info->brackets[bi];
  int i = 0, len, result;
  const char *p;

  do {
    p = i == 0 ? b->ptr : info->branches[b->branches + i - 1].schlong + 1;
    len = b->num_branches == 0 ? b->len :
      i == b->num_branches ? (int) (b->ptr + b->len - p) :
      (int) (info->branches[b->branches + i].schlong - p);
    result = bar(p, len, s, s_len, info, bi);
  } while (result <= 0 && i++ < b->num_branches);

  return result;
}

static int baz(const char *s, int s_len, struct regex_info *info) {
  int i, result = -1, is_anchored = info->brackets[0].ptr[0] == '^';

  for (i = 0; i <= s_len; i++) {
    result = doh(s + i, s_len - i, info, 0);
    if (result >= 0) {
      result += i;
      break;
    }
    if (is_anchored) break;
  }

  return result;
}

static void setup_branch_points(struct regex_info *info) {
  int i, j;
  struct branch tmp;

  for (i = 0; i < info->num_branches; i++) {
    for (j = i + 1; j < info->num_branches; j++) {
      if (info->branches[i].bracket_index > info->branches[j].bracket_index) {
        tmp = info->branches[i];
        info->branches[i] = info->branches[j];
        info->branches[j] = tmp;
      }
    }
  }

  for (i = j = 0; i < info->num_brackets; i++) {
    info->brackets[i].num_branches = 0;
    info->brackets[i].branches = j;
    while (j < info->num_branches && info->branches[j].bracket_index == i) {
      info->brackets[i].num_branches++;
      j++;
    }
  }
}

static int foo(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info) {
  int i, step, depth = 0;

  info->brackets[0].ptr = re;
  info->brackets[0].len = re_len;
  info->num_brackets = 1;

  for (i = 0; i < re_len; i += step) {
    step = get_op_len(re + i, re_len - i);

    if (re[i] == '|') {
      FAIL_IF(info->num_branches >= (int) ARRAY_SIZE(info->branches),
              RE_TOO_MANY_BRANCHES);
      info->branches[info->num_branches].bracket_index =
        info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->branches[info->num_branches].schlong = &re[i];
      info->num_branches++;
    } else if (re[i] == '\\') {
      FAIL_IF(i >= re_len - 1, RE_INVALID_METACHARACTER);
      if (re[i + 1] == 'x') {
        FAIL_IF(re[i + 1] == 'x' && i >= re_len - 3,
                RE_INVALID_METACHARACTER);
        FAIL_IF(re[i + 1] ==  'x' && !(isxdigit(re[i + 2]) &&
                isxdigit(re[i + 3])), RE_INVALID_METACHARACTER);
      } else {
        FAIL_IF(!is_metacharacter((const unsigned char *) re + i + 1),
                RE_INVALID_METACHARACTER);
      }
    } else if (re[i] == '(') {
      FAIL_IF(info->num_brackets >= (int) ARRAY_SIZE(info->brackets),
              RE_TOO_MANY_BRACKETS);
      depth++;
      info->brackets[info->num_brackets].ptr = re + i + 1;
      info->brackets[info->num_brackets].len = -1;
      info->num_brackets++;
      FAIL_IF(info->num_caps > 0 && info->num_brackets - 1 > info->num_caps,
              RE_CAPS_ARRAY_TOO_SMALL);
    } else if (re[i] == ')') {
      int ind = info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->brackets[ind].len = (int) (&re[i] - info->brackets[ind].ptr);
      depth--;
      FAIL_IF(depth < 0, RE_UNBALANCED_BRACKETS);
      FAIL_IF(i > 0 && re[i - 1] == '(', RE_NO_MATCH);
    }
  }

  FAIL_IF(depth != 0, RE_UNBALANCED_BRACKETS);
  setup_branch_points(info);

  return baz(s, s_len, info);
}

int re_match(const char *regexp, const char *s, int s_len,
               struct re_cap *caps, int num_caps, int flags) {
  struct regex_info info;

  info.flags = flags;
  info.num_brackets = info.num_branches = 0;
  info.num_caps = num_caps;
  info.caps = caps;

  return foo(regexp, (int) strlen(regexp), s, s_len, &info);
}