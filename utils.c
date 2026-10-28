#define _POSIX_C_SOURCE 200809L
#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#endif

/* Lit une ligne complète. Si l'entrée est fermée (Ctrl+D), on quitte proprement. */
void read_line(const char *prompt, char *buf, size_t size)
{
    if (prompt)
        printf("%s", prompt);
    fflush(stdout);

    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\nFin de l'entrée, au revoir.\n");
        exit(EXIT_SUCCESS);
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int c;                                   /* ligne trop longue : on vide le reste */
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

int read_int(const char *prompt, int min, int max)
{
    char buf[MAX_LINE];
    for (;;) {
        read_line(prompt, buf, sizeof buf);
        char *end;
        errno = 0;
        long v = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && errno == 0 && v >= min && v <= max)
            return (int)v;
        printf("Entrée invalide : entrez un nombre entre %d et %d.\n", min, max);
    }
}

float read_float(const char *prompt, float min)
{
    char buf[MAX_LINE];
    for (;;) {
        read_line(prompt, buf, sizeof buf);
        for (char *p = buf; *p; p++)             /* accepte la virgule : 12,5 */
            if (*p == ',')
                *p = '.';
        char *end;
        errno = 0;
        double v = strtod(buf, &end);
        if (end != buf && *end == '\0' && errno == 0 && v >= min && v < 100000.0)
            return (float)v;
        printf("Entrée invalide : entrez un nombre supérieur ou égal à %.2f.\n", min);
    }
}

int confirm(const char *prompt)
{
    char buf[MAX_LINE];
    read_line(prompt, buf, sizeof buf);
    return buf[0] == 'o' || buf[0] == 'O' || buf[0] == 'y' || buf[0] == 'Y';
}

void strip_newline(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

/* Remplace '|' (séparateur des fichiers) et les caractères de contrôle, supprime les espaces autour. */
void sanitize_text(char *s)
{
    for (char *p = s; *p; p++) {
        if (*p == '|')
            *p = '/';
        else if ((unsigned char)*p < 32)
            *p = ' ';
    }
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    size_t start = 0;
    while (s[start] && isspace((unsigned char)s[start]))
        start++;
    if (start > 0)
        memmove(s, s + start, strlen(s + start) + 1);
}

/* Affiche s sur "width" caractères (un caractère UTF-8 accentué = 2 octets mais 1 colonne). */
void print_padded(const char *s, int width)
{
    int chars = 0;
    for (const char *p = s; *p; p++)
        if (((unsigned char)*p & 0xC0) != 0x80)
            chars++;
    fputs(s, stdout);
    for (int i = chars; i < width; i++)
        putchar(' ');
}

int split_fields(char *line, char sep, char **fields, int max_fields)
{
    int n = 0;
    char *p = line;
    while (n < max_fields) {
        fields[n++] = p;
        char *q = strchr(p, sep);
        if (!q)
            break;
        *q = '\0';
        p = q + 1;
    }
    return n;
}

int ensure_dir(const char *path)
{
#ifdef _WIN32
    int r = _mkdir(path);
#else
    int r = mkdir(path, 0755);
#endif
    return (r == 0 || errno == EEXIST) ? 1 : 0;
}
