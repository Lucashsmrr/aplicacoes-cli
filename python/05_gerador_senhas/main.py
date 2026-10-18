"""Gera senhas usando exclusivamente aleatoriedade criptografica."""

import secrets
import string


GROUPS: tuple[str, ...] = (
    string.ascii_lowercase,
    string.ascii_uppercase,
    string.digits,
    string.punctuation,
)
ALPHABET = "".join(GROUPS)


def generate_password(length: int) -> str:
    """Gera senha com minuscula, maiuscula, digito e simbolo obrigatorios.

    Args:
        length: Numero inteiro de caracteres, entre 12 e 128.
    Returns:
        Senha amostrada uniformemente entre as que satisfazem a politica.
    Raises:
        ValueError: Comprimento fora da politica ou tipo incorreto.
        OSError: Fonte de aleatoriedade do sistema indisponivel.
    """
    if type(length) is not int or not 12 <= length <= 128:
        raise ValueError("Comprimento deve ser um inteiro entre 12 e 128.")
    while True:
        password = "".join(secrets.choice(ALPHABET) for _ in range(length))
        if all(any(char in group for char in password) for group in GROUPS):
            return password


def main() -> int:
    """Valida comprimento e quantidade; gera ate 100 senhas por execucao."""
    try:
        while True:
            try:
                length = int(input("Comprimento [12..128]: "))
                if not 12 <= length <= 128:
                    raise ValueError("Use de 12 a 128 caracteres.")
                break
            except ValueError as error:
                print(f"Entrada invalida: {error}")
        while True:
            try:
                count = int(input("Quantidade [1..100]: "))
                if not 1 <= count <= 100:
                    raise ValueError("Use de 1 a 100 senhas.")
                break
            except ValueError as error:
                print(f"Entrada invalida: {error}")
        for _ in range(count):
            print(generate_password(length))
    except (EOFError, KeyboardInterrupt):
        print("\nEncerrado.")
    except (OSError, UnicodeError) as error:
        print(f"Falha de IO ou aleatoriedade: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
