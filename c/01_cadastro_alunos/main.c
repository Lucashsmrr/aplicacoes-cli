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

#define MAX_ALUNOS 100
#define NOTAS 3

typedef struct {
    char nome[81];
    double notas[NOTAS];
} Aluno;

/* Pre: a aponta para um aluno com tres notas de 0 a 10. */
static double media(const Aluno *a)
{
    size_t i;
    double soma = 0.0;
    for (i = 0; i < NOTAS; ++i) soma += a->notas[i];
    return soma / NOTAS;
}

int main(void)
{
    Aluno alunos[MAX_ALUNOS];
    size_t n = 0, i, j;
    long op;
    do {
        if (!inteiro("1 Cadastrar | 2 Todos | 3 Aprovados | "
                     "4 Reprovados | 0 Sair: ", 0, 4, &op)) break;
        if (op == 1) {
            Aluno novo;
            if (n == MAX_ALUNOS) {
                puts("Cadastro cheio.");
                continue;
            }
            if (!texto("Nome: ", novo.nome, sizeof novo.nome)) break;
            for (j = 0; j < NOTAS; ++j) {
                printf("Nota %zu\n", j + 1);
                if (!real("Valor: ", 0, 10, &novo.notas[j]))
                    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
            }
            alunos[n++] = novo;
            puts("Aluno cadastrado.");
        } else if (op >= 2) {
            size_t exibidos = 0;
            for (i = 0; i < n; ++i) {
                double m = media(&alunos[i]);
                if (op == 2 || (op == 3 && m >= 6)
                            || (op == 4 && m < 6)) {
                    printf("%s | %.2f | %s\n", alunos[i].nome, m,
                           m >= 6 ? "Aprovado" : "Reprovado");
                    ++exibidos;
                }
            }
            if (exibidos == 0) puts("Nenhum aluno neste filtro.");
        }
    } while (op != 0);
    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
}
