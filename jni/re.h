#ifndef RE_H_
#define RE_H_

struct re_cap {
  const char *ptr;
  int len;
};

int re_match(const char *regexp, const char *buf, int buf_len,
               struct re_cap *caps, int num_caps, int flags);

enum { RE_IGNORE_CASE = 1 };

#define RE_NO_MATCH               -1
#define RE_UNEXPECTED_QUANTIFIER  -2
#define RE_UNBALANCED_BRACKETS    -3
#define RE_INTERNAL_ERROR         -4
#define RE_INVALID_CHARACTER_SET  -5
#define RE_INVALID_METACHARACTER  -6
#define RE_CAPS_ARRAY_TOO_SMALL   -7
#define RE_TOO_MANY_BRANCHES      -8
#define RE_TOO_MANY_BRACKETS      -9

#endif /* RE_H_ */