#ifndef UTILS_H
#define UTILS_H

void version(void);
void die(const char *errstr, ...);
void print_err(const char *errstr, ...);
void sort_string_array(char **);
int string_cmp(const void *, const void *);
void remove_suffix(char *, char *);

#endif /* UTILS_H */
