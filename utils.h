#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define MAX_NAME 64
#define MAX_CAT  32
#define MAX_LINE 256

/* Saisie sécurisée (fgets) */
void  read_line(const char *prompt, char *buf, size_t size);
int   read_int(const char *prompt, int min, int max);
float read_float(const char *prompt, float min);
int   confirm(const char *prompt);

/* Texte */
void sanitize_text(char *s);                 /* retire '|' et retours à la ligne, coupe les espaces */
void print_padded(const char *s, int width); /* alignement correct avec les accents (UTF-8) */
int  split_fields(char *line, char sep, char **fields, int max_fields);
void strip_newline(char *s);

/* Fichiers */
int ensure_dir(const char *path);

#endif
