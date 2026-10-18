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

#define MAX_PRODUTOS 100
#define MAX_QUANTIDADE 1000000L

typedef struct { long quantidade; double preco; } Saldo;
typedef struct { long id; char nome[81]; Saldo saldo; } Produto;

/* Pre: n corresponde a elementos validos. Pos: n significa ausente. */
static size_t buscar(const Produto *p, size_t n, long id)
{
    size_t i;
    for (i = 0; i < n; ++i) if (p[i].id == id) return i;
    return n;
}

/* Pos: falha nao altera saldo; sucesso aplica delta integralmente. */
static int movimentar(Saldo *s, long delta)
{
    if (delta < -s->quantidade
            || delta > MAX_QUANTIDADE - s->quantidade) return 0;
    s->quantidade += delta;
    return 1;
}

static void mostrar(const Produto *p)
{
    printf("%ld | %s | %ld unidades | R$ %.2f\n",
           p->id, p->nome, p->saldo.quantidade, p->saldo.preco);
}

int main(void)
{
    Produto produtos[MAX_PRODUTOS];
    size_t n = 0, i;
    long op, id, delta;
    while (inteiro("1 Criar | 2 Listar | 3 Buscar ID | 4 Buscar nome | "
                   "5 Movimentar | 6 Excluir | 7 Editar | 0 Sair: ",
                   0, 7, &op) && op != 0) {
        if (op == 1) {
            Produto novo;
            if (n == MAX_PRODUTOS) { puts("Estoque cheio."); continue; }
            if (!inteiro("ID: ", 1, 1000000000L, &novo.id)) break;
            if (buscar(produtos, n, novo.id) != n) {
                puts("ID existente."); continue;
            }
            if (!texto("Nome: ", novo.nome, sizeof novo.nome)
                    || !inteiro("Quantidade: ", 0, MAX_QUANTIDADE,
                                &novo.saldo.quantidade)
                    || !real("Preco: ", 0, 1000000, &novo.saldo.preco))
                break;
            produtos[n++] = novo;
        } else if (op == 2) {
            if (n == 0) puts("Estoque vazio.");
            for (i = 0; i < n; ++i) mostrar(&produtos[i]);
        } else if (op == 4) {
            char nome[81];
            size_t achados = 0;
            if (!texto("Trecho do nome (diferencia maiusculas): ",
                       nome, sizeof nome)) break;
            for (i = 0; i < n; ++i) {
                if (strstr(produtos[i].nome, nome) != NULL) {
                    mostrar(&produtos[i]); ++achados;
                }
            }
            if (!achados) puts("Nenhum produto encontrado.");
        } else {
            if (!inteiro("ID: ", 1, 1000000000L, &id)) break;
            i = buscar(produtos, n, id);
            if (i == n) { puts("ID inexistente."); continue; }
            if (op == 3) mostrar(&produtos[i]);
            else if (op == 5) {
                if (!inteiro("Delta (+entrada/-saida): ", -MAX_QUANTIDADE,
                             MAX_QUANTIDADE, &delta)) break;
                puts(movimentar(&produtos[i].saldo, delta)
                     ? "Saldo atualizado." : "Operacao fora dos limites.");
            } else if (op == 6) {
                for (; i + 1 < n; ++i) produtos[i] = produtos[i + 1];
                --n;
                puts("Produto excluido.");
            } else {
                Produto novo = produtos[i];
                if (!texto("Novo nome: ", novo.nome, sizeof novo.nome)
                        || !real("Novo preco: ", 0, 1000000,
                                 &novo.saldo.preco)) break;
                produtos[i] = novo;
            }
        }
    }
    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
}
