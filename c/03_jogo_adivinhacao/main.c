#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <time.h>

/* Pre: cap >= 2. Pos: string terminada; 0 indica EOF/erro.
   Consome a linha inteira; rejeita NUL, excesso e texto vazio. */
static int texto(const char *prompt, char *out, size_t cap)
{
    for (;;) {
        char one[2];
        size_t n = 0, start = 0;
        int bad = 0, seen = 0;
        fputs(prompt, stdout);
        fflush(stdout);
        while (fgets(one, sizeof one, stdin) != NULL) {
            seen = 1;
            if (one[0] == '\n') break;
            if (one[0] == '\0' || n >= cap - 1) bad = 1;
            else out[n++] = one[0];
        }
        if (ferror(stdin) || (!seen && feof(stdin))) return 0;
        out[n] = '\0';
        while (n > 0 && isspace((unsigned char)out[n - 1]))
            out[--n] = '\0';
        while (isspace((unsigned char)out[start])) ++start;
        if (!bad && start < n) {
            memmove(out, out + start, n - start + 1);
            return 1;
        }
        puts("Entrada vazia, longa demais ou invalida.");
    }
}

/* Pre: lo <= hi. Pos: somente grava valor valido; 0 em EOF. */
static int inteiro(const char *prompt, long lo, long hi, long *out)
{
    char s[80], *end;
    long value;
    while (texto(prompt, s, sizeof s)) {
        errno = 0;
        value = strtol(s, &end, 10);
        if (errno == 0 && end != s && *end == '\0'
                && value >= lo && value <= hi) {
            *out = value;
            return 1;
        }
        printf("Informe um inteiro de %ld a %ld.\n", lo, hi);
    }
    return 0;
}

/* Pos: inteiro uniforme em [1,100], supondo rand() uniforme. */
static int sortear(void)
{
    int r, limite = RAND_MAX - RAND_MAX % 100;
    do { r = rand(); } while (r >= limite);
    return r % 100 + 1;
}

int main(void)
{
    long palpite, novamente;
    srand((unsigned int)time(NULL));
    do {
        int segredo = sortear(), baixo = 1, alto = 100;
        unsigned int tentativas = 0;
        for (;;) {
            printf("Intervalo atual: [%d, %d]\n", baixo, alto);
            if (!inteiro("Palpite: ", baixo, alto, &palpite))
                return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
            ++tentativas;
            if (palpite == segredo) {
                printf("Acertou em %u tentativa(s)!\n", tentativas);
                break;
            }
            if (palpite < segredo) {
                baixo = (int)palpite + 1; puts("O numero e maior.");
            } else {
                alto = (int)palpite - 1; puts("O numero e menor.");
            }
        }
        if (!inteiro("Jogar novamente? 1 Sim | 0 Nao: ",
                     0, 1, &novamente)) break;
    } while (novamente == 1);
    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
}
