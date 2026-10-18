"""Quiz com opcoes aninhadas, embaralhamento opcional e relatorio."""

import argparse
import random
from typing import TypedDict


class Question(TypedDict):
    """Enunciado, alternativas e chave correta de uma questao."""

    text: str
    options: dict[str, str]
    answer: str


QUESTIONS: list[Question] = [
    {"text": "Qual tipo e imutavel?",
     "options": {"a": "list", "b": "tuple", "c": "dict"}, "answer": "b"},
    {"text": "Quanto vale 2 ** 3?",
     "options": {"a": "6", "b": "9", "c": "8"}, "answer": "c"},
    {"text": "Qual estrutura armazena chaves e valores?",
     "options": {"a": "dict", "b": "set", "c": "list"}, "answer": "a"},
]


def ask(question: Question) -> bool:
    """Solicita alternativa valida e informa se houve acerto.

    Args:
        question: Questao cujo gabarito pertence as alternativas.
    Returns:
        Verdadeiro se a alternativa escolhida e correta.
    Raises:
        EOFError: Entrada encerrada.
        KeyboardInterrupt: Usuario interrompeu o quiz.
    """
    print(question["text"])
    for key, value in question["options"].items():
        print(f"  {key}) {value}")
    while True:
        answer = input("Resposta: ").strip().casefold()
        if answer in question["options"]:
            return answer == question["answer"]
        print("Escolha uma das letras exibidas.")


def main() -> int:
    """Executa o quiz e exibe desempenho, incluindo partidas interrompidas."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--embaralhar", action="store_true")
    args = parser.parse_args()
    questions = QUESTIONS.copy()
    if args.embaralhar:
        random.shuffle(questions)
    results: list[tuple[str, bool]] = []
    interrupted = False
    try:
        for question in questions:
            correct = ask(question)
            results.append((question["text"], correct))
            if correct:
                print("Correto!")
            else:
                answer = question["options"][question["answer"]]
                print(f"Resposta correta: {answer}")
    except (EOFError, KeyboardInterrupt):
        interrupted = True
        print("\nQuiz interrompido.")
    except (OSError, UnicodeError) as error:
        print(f"Falha de entrada/saida: {error}")
        return 1
    score = sum(correct for _, correct in results)
    answered = len(results)
    percentage = 100 * score / answered if answered else 0.0
    for text, correct in results:
        print(f"{'Acerto' if correct else 'Erro'}: {text}")
    print(f"Respondidas: {answered}/{len(questions)}")
    print(f"Acertos: {score}")
    print(f"Aproveitamento das respondidas: {percentage:.1f}%")
    return 130 if interrupted else 0


if __name__ == "__main__":
    raise SystemExit(main())
