#include "utils/uart.h"

#define PRINTK_BUF_LEN 1024

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)

size_t strlen(const char *s) {
  const char *p = s;
  while (*p)
    ++p;
  return (size_t)(p - s);
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2) return *s1 - *s2;
    ++s1;
    ++s2;
  }
  if (!*s1 && !*s2) {
    return 0;
  } else if (!*s1) {
    return -1;
  } else { // !*s2
    return 1;
  }
}

static int vsprintf(char *str, const char *fmt, va_list ap) {
  char *outptr = str;
  char *s;
  long long d;
  unsigned long long u;
  char tmp_num[20];

  // flag for printing negative number with no zero_padded
  bool d_negative_no_zero_padded_flag = false;

  bool zero_padded = false;
  bool in_format = false;
  bool long_prefix = false;
  unsigned width_now = 0;
  size_t i;
  while (*fmt != '\0') {
    if (in_format) {
      switch (*fmt) {
      case 'l': long_prefix = true; break;
      case 'c':
        d = va_arg(ap, int);
        *outptr++ = d;
        in_format = false;
        break;
      case 's': // string
        s = va_arg(ap, char *);
        int count_s = width_now - strlen(s);
        while (count_s-- > 0) {
          if (zero_padded) *outptr++ = '0';
          else
            *outptr++ = ' ';
        }
        while (*s != '\0')
          *outptr++ = *s++;
        in_format = false;
        break;
      case 'd': // integer
        if (long_prefix) d = va_arg(ap, long long);
        else
          d = va_arg(ap, int);
        if (d < 0) {
          d = -d;
          if (zero_padded) {
            *outptr++ = '-';
            --width_now;
          } else
            d_negative_no_zero_padded_flag = true;
        } else if (d == 0) {
          if (width_now == 0) width_now = 1;
          while (width_now-- > 0)
            *outptr++ = '0';
        }
        for (i = 1; d; i++, d /= 10)
          tmp_num[i] = (d % 10) + '0';
        if (d_negative_no_zero_padded_flag) tmp_num[i++] = '-';
        int count_d = width_now - (i - 1);
        while (count_d-- > 0) {
          if (zero_padded) *outptr++ = '0';
          else
            *outptr++ = ' ';
        }
        for (i--; i; i--)
          *outptr++ = tmp_num[i];
        in_format = false;
        long_prefix = false;
        break;
      case 'u': // unsigned
        if (long_prefix) u = va_arg(ap, unsigned long long);
        else
          u = va_arg(ap, unsigned);
        if (u == 0) {
          if (width_now == 0) width_now = 1;
          while (width_now-- > 0)
            *outptr++ = '0';
        }
        for (i = 1; u; i++, u /= 10)
          tmp_num[i] = (u % 10) + '0';
        int count_u = width_now - (i - 1);
        while (count_u-- > 0) {
          if (zero_padded) *outptr++ = '0';
          else
            *outptr++ = ' ';
        }
        for (i--; i; i--)
          *outptr++ = tmp_num[i];
        in_format = false;
        long_prefix = false;
        break;
      case 'x': // hex
      case 'p': // pointer
        if (*fmt == 'p') {
          *outptr++ = '0';
          *outptr++ = 'x';
          long_prefix = true;
        }
        if (long_prefix) u = va_arg(ap, unsigned long long);
        else
          u = va_arg(ap, unsigned);
        if (u == 0) {
          if (width_now == 0) width_now = 1;
          while (width_now-- > 0)
            *outptr++ = '0';
        }
        for (i = 1; u; i++, u /= 16) {
          if (u % 16 <= 9) {
            tmp_num[i] = (u % 16) + '0';
          } else {
            tmp_num[i] = (u % 16) + 'a' - 10;
          }
        }
        int count_x = width_now - (i - 1);
        while (count_x-- > 0) {
          if (zero_padded) *outptr++ = '0';
          else
            *outptr++ = ' ';
        }
        for (i--; i; i--)
          *outptr++ = tmp_num[i];
        in_format = false;
        long_prefix = false;
        break;
      case '0':
        if (*(fmt - 1) == '%') zero_padded = true;
        else
          width_now *= 10;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': width_now = 10 * width_now + *fmt - '0'; break;
      default:
        for (char *s = "Error in printk!\n", *p = s; *p; ++p)
          write_serial(*p);
      }
      ++fmt;
    } else if (*fmt == '%') {
      in_format = true;
      width_now = 0;
      zero_padded = false;
      ++fmt;
    } else {
      *outptr = *fmt;
      ++fmt;
      ++outptr;
    }
  }
  *outptr = '\0';
  return outptr - str;
}

int printk(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char buf[PRINTK_BUF_LEN];
  int ret;

  ret = vsprintf(buf, fmt, ap);
  for (char *c = buf; *c; c++)
    write_serial(*c);

  va_end(ap);
  return ret;
}