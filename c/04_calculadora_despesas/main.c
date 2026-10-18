#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

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

/* Pos: aceita apenas reais finitos no intervalo fechado [lo, hi]. */
static int real(const char *prompt, double lo, double hi, double *out)
{
    char s[80], *end;
    double value;
    while (texto(prompt, s, sizeof s)) {
        errno = 0;
        value = strtod(s, &end);
        if (errno == 0 && end != s && *end == '\0'
                && isfinite(value) && value >= lo && value <= hi) {
            *out = value;
            return 1;
        }
        printf("Informe um numero de %.2f a %.2f (ponto decimal).\n",
               lo, hi);
    }
    return 0;
}

#define MAX_DESPESAS 1000

/* Pre: v possui n valores finitos em [0, 1e9], n <= 1000. */
static double total(const double *v, size_t n)
{
    size_t i;
    double soma = 0;
    for (i = 0; i < n; ++i) soma += v[i];
    return soma;
}

/* Pre: 1 <= janela <= n. Pos: media das ultimas janela despesas. */
static double media_movel(const double *v, size_t n, size_t janela)
{
    return total(v + n - janela, janela) / (double)janela;
}

int main(void)
{
    double valores[MAX_DESPESAS], valor;
    size_t n = 0, i;
    long op, janela;
    while (inteiro("1 Adicionar | 2 Relatorio | 3 Media movel | "
                   "0 Sair: ", 0, 3, &op) && op != 0) {
        if (op == 1) {
            if (n == MAX_DESPESAS) { puts("Limite atingido."); continue; }
            if (!real("Despesa: ", 0, 1e9, &valor)) break;
            valores[n++] = valor;
        } else if (n == 0) {
            puts("Nenhuma despesa cadastrada.");
        } else if (op == 2) {
            double soma = total(valores, n);
            for (i = 0; i < n; ++i)
                printf("%zu: R$ %.2f\n", i + 1, valores[i]);
            printf("Total: R$ %.2f | Media: R$ %.2f\n",
                   soma, soma / (double)n);
        } else {
            if (!inteiro("Janela: ", 1, (long)n, &janela)) break;
            printf("Media movel: R$ %.2f\n",
                   media_movel(valores, n, (size_t)janela));
        }
    }
    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
}
