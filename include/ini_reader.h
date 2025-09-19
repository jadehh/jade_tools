/**
# @File     : ini_reader.h
# @Author   : jade
# @Date     : 2025/9/15 15:02
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : ini_reader.h
*/
// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/
#ifndef __INI_H__
#define __INI_H__
/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Typedef for prototype of handler function. */
typedef int (*ini_handler)(void *user, const char *section, const char *name,
                           const char *value);

/* Typedef for prototype of fgets-style reader function. */
typedef char *(*ini_reader)(char *str, int num, void *stream);

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's configparser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).
*/
int ini_parse(const char *filename, ini_handler handler, void *user);

/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that. */
int ini_parse_file(FILE *file, ini_handler handler, void *user);

/* Same as ini_parse(), but takes an ini_reader function pointer instead of
   filename. Used for implementing custom or string-based I/O. */
int ini_parse_stream(ini_reader reader, void *stream, ini_handler handler,
                     void *user);

/* Nonzero to allow multi-line value parsing, in the style of Python's
   configparser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to allow inline comments (with valid inline comment characters
   specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
   Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cctype>
#include <cstring>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 100

/* Strip whitespace chars off end of given string, in place. Return s. */
inline static char *rstrip(char *s) {
  char *p = s + strlen(s);
  while (p > s && isspace(static_cast<unsigned char>(*--p)))
    *p = '\0';
  return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char *lskip(const char *s) {
  while (*s && isspace(static_cast<unsigned char>(*s)))
    s++;
  return const_cast<char*>(s);
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
inline static char *find_chars_or_comment(const char *s, const char *chars) {
#if INI_ALLOW_INLINE_COMMENTS
  int was_space = 0;
  while (*s && (!chars || !strchr(chars, *s)) &&
         !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
    was_space = isspace(static_cast<unsigned char>(*s));
    s++;
  }
#else
  while (*s && (!chars || !strchr(chars, *s))) {
    s++;
  }
#endif
  return const_cast<char*>(s);
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
inline static char *strncpy0(char *dest, const char *src, size_t size) {
  // 优先使用安全版本
#ifdef _WIN32
  strncpy_s(dest, size, src, _TRUNCATE);
#else
  // Linux/macOS 回退方案
  strncpy(dest, src, size - 1);
  dest[size - 1] = '\0';
#endif
  return dest;
}

/* See documentation in header file. */
inline int ini_parse_stream(ini_reader reader, void *stream,
                            ini_handler handler, void *user) {
  /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
  char line[INI_MAX_LINE];
#else
  char *line;
#endif
  char section[MAX_SECTION] = "";
  char prev_name[MAX_NAME] = "";

  char *start;
  char *end;
  char *name;
  char *value;
  int lineno = 0;
  int error = 0;

#if !INI_USE_STACK
  line = (char *)malloc(INI_MAX_LINE);
  if (!line) {
    return -2;
  }
#endif

  /* Scan through stream line by line */
  while (reader(line, INI_MAX_LINE, stream) != nullptr) {
    lineno++;

    start = line;
#if INI_ALLOW_BOM
    if (lineno == 1 && static_cast<unsigned char>(start[0]) == 0xEF &&
        static_cast<unsigned char>(start[1]) == 0xBB && static_cast<unsigned char>(start[2]) == 0xBF) {
      start += 3;
    }
#endif
    start = lskip(rstrip(start));

    if (*start == ';' || *start == '#') {
      /* Per Python configparser, allow both ; and # comments at the
         start of a line */
    }
#if INI_ALLOW_MULTILINE
    else if (*prev_name && *start && start > line) {

#if INI_ALLOW_INLINE_COMMENTS
      end = find_chars_or_comment(start, nullptr);
      if (*end)
        *end = '\0';
      rstrip(start);
#endif

      /* Non-blank line with leading whitespace, treat as continuation
         of previous name's value (as per Python configparser). */
      if (!handler(user, section, prev_name, start) && !error)
        error = lineno;
    }
#endif
    else if (*start == '[') {
      /* A "[section]" line */
      end = find_chars_or_comment(start + 1, "]");
      if (*end == ']') {
        *end = '\0';
        strncpy0(section, start + 1, sizeof(section));
        *prev_name = '\0';
      } else if (!error) {
        /* No ']' found on section line */
        error = lineno;
      }
    } else if (*start) {
      /* Not a comment, must be a name[=:]value pair */
      end = find_chars_or_comment(start, "=:");
      if (*end == '=' || *end == ':') {
        *end = '\0';
        name = rstrip(start);
        value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
        end = find_chars_or_comment(value, nullptr);
        if (*end)
          *end = '\0';
#endif
        rstrip(value);

        /* Valid name[=:]value pair found, call handler */
        strncpy0(prev_name, name, sizeof(prev_name));
        if (!handler(user, section, name, value) && !error)
          error = lineno;
      } else if (!error) {
        /* No '=' or ':' found on name[=:]value line */
        error = lineno;
      }
    }

#if INI_STOP_ON_FIRST_ERROR
    if (error)
      break;
#endif
  }

#if !INI_USE_STACK
  free(line);
#endif

  return error;
}

/* See documentation in header file. */
inline int ini_parse_file(FILE *file, const ini_handler handler, void *user) {
  return ini_parse_stream(reinterpret_cast<ini_reader>(fgets), file, handler, user);
}

/* See documentation in header file. */
inline int ini_parse(const char *filename, const ini_handler handler, void *user) {
  FILE *file;
#ifdef _WIN32
  if (const errno_t err = fopen_s(&file, filename, "r"); err != 0 || file == nullptr) {
    return -1;
  }
#else
  file = fopen(filename, "r");
#endif
  if (!file)
  {
    return -1;
  }
  const int error = ini_parse_file(file, handler, user);
  fclose(file);
  return error;
}

#endif /* __INI_H__ */
