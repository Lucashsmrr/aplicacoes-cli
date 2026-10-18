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

typedef struct {
    const char *enunciado;
    const char *respostas[3];
} Pergunta;

/* Pre: s e uma string terminada. Pos: normaliza letras ASCII. */
static void normalizar(char *s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; ++i)
        s[i] = (char)tolower((unsigned char)s[i]);
}

int main(void)
{
    const Pergunta perguntas[] = {
        {"Qual funcao libera memoria em C?", {"free", "free()", NULL}},
        {"Quanto e 7 * 8?", {"56", "cinquenta e seis", NULL}},
        {"Qual palavra inicia uma estrutura em C?", {"struct", NULL, NULL}}
    };
    const size_t n = sizeof perguntas / sizeof perguntas[0];
    size_t i, j, pontos = 0;
    char resposta[100];
    for (i = 0; i < n; ++i) {
        printf("%zu/%zu: %s\n", i + 1, n, perguntas[i].enunciado);
        if (!texto("Resposta: ", resposta, sizeof resposta)) {
            puts("Quiz interrompido.");
            return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        normalizar(resposta);
        for (j = 0; j < 3 && perguntas[i].respostas[j] != NULL; ++j)
            if (strcmp(resposta, perguntas[i].respostas[j]) == 0) break;
        if (j < 3 && perguntas[i].respostas[j] != NULL) {
            ++pontos; puts("Correto!");
        } else printf("Resposta: %s\n", perguntas[i].respostas[0]);
    }
    printf("Resultado: %zu/%zu (%.1f%%)\n", pontos, n,
           100.0 * (double)pontos / (double)n);
    return EXIT_SUCCESS;
}
