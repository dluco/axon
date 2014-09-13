#ifndef UTILS_H
#define UTILS_H

void version(void);
void die(const char *errstr, ...);
void print_err(const char *errstr, ...);
void colortable(void);
unsigned int strv_length(char **);
void strv_sort(char **);
int str_has_suffix(const char *, const char *);
void str_remove_suffix(char *, char *);

#endif /* UTILS_H */
