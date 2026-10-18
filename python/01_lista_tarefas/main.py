"""CRUD de tarefas; uma instancia por arquivo e gravacao atomica."""

import json
import os
import sys
import tempfile
from pathlib import Path
from typing import TypedDict, cast


class Task(TypedDict):
    """Registro persistente de uma tarefa."""

    id: int
    title: str
    done: bool


DATA = Path(__file__).resolve().with_name("tasks.json")
MAX_TASKS = 10000


def load_tasks(path: Path) -> list[Task]:
    """Carrega e valida o arquivo, sem substituir dados corrompidos.

    Args:
        path: Arquivo JSON com uma lista de tarefas.
    Returns:
        Lista validada; lista vazia quando o arquivo nao existe.
    Raises:
        OSError: Falha de leitura.
        ValueError: JSON, tamanho ou esquema invalido.
    """
    try:
        with path.open("rb") as stream:
            raw = stream.read(16_000_001)
    except FileNotFoundError:
        return []
    if len(raw) > 16_000_000:
        raise ValueError("Arquivo excede 16 MB.")
    data: object = json.loads(raw.decode("utf-8"))
    if not isinstance(data, list) or len(data) > MAX_TASKS:
        raise ValueError("Esperada uma lista de ate 10000 tarefas.")
    ids: set[int] = set()
    for task in data:
        if not isinstance(task, dict) or set(task) != {"id", "title", "done"}:
            raise ValueError("Registro de tarefa invalido.")
        ident, title, done = task["id"], task["title"], task["done"]
        if type(ident) is not int or not 1 <= ident <= 10**12:
            raise ValueError("ID invalido.")
        if ident in ids or type(done) is not bool:
            raise ValueError("ID duplicado ou estado invalido.")
        if not isinstance(title, str) or not 1 <= len(title) <= 200:
            raise ValueError("Titulo invalido.")
        if title != title.strip() or not title.isprintable():
            raise ValueError("Titulo possui caracteres invalidos.")
        ids.add(ident)
    return cast(list[Task], data)


def save_tasks(path: Path, tasks: list[Task]) -> None:
    """Persiste em temporario no mesmo diretorio e substitui o destino.

    Args:
        path: Destino da gravacao.
        tasks: Registros previamente validados.
    Raises:
        OSError: Gravacao ou substituicao falhou; estado antigo preservado
            se a substituicao ainda nao ocorreu.
    """
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", dir=path.parent,
            prefix=".tasks-", suffix=".tmp", delete=False,
        ) as stream:
            temporary = Path(stream.name)
            json.dump(tasks, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def ask_title() -> str:
    """Solicita titulo imprimivel de 1 a 200 caracteres.

    Returns:
        Titulo validado sem espacos externos.
    Raises:
        EOFError: Entrada encerrada.
        KeyboardInterrupt: Usuario interrompeu a leitura.
    """
    while True:
        title = input("Titulo: ").strip()
        if 1 <= len(title) <= 200 and title.isprintable():
            return title
        print("Use de 1 a 200 caracteres imprimiveis.")


def session(path: Path) -> None:
    """Executa CRUD com lock ja adquirido.

    Args:
        path: Arquivo JSON de persistencia.
    Raises:
        OSError: Falha ao carregar os dados.
        ValueError: Conteudo persistido invalido.
        EOFError: Entrada encerrada.
        KeyboardInterrupt: Interrupcao solicitada pelo usuario.
    """
    tasks = load_tasks(path)
    while True:
        option = input(
            "1 Criar | 2 Listar | 3 Editar | 4 Alternar conclusao | "
            "5 Excluir | 0 Sair: "
        ).strip()
        if option == "0":
            return
        if option == "2":
            for task in tasks:
                mark = "x" if task["done"] else " "
                print(f"{task['id']} [{mark}] {task['title']}")
            if not tasks:
                print("Nenhuma tarefa.")
            continue
        if option not in {"1", "3", "4", "5"}:
            print("Opcao invalida.")
            continue
        updated: list[Task] = [task.copy() for task in tasks]
        if option == "1":
            ident = max((task["id"] for task in tasks), default=0) + 1
            if len(tasks) >= MAX_TASKS or ident > 10**12:
                print("Limite de tarefas ou IDs atingido.")
                continue
            updated.append({"id": ident, "title": ask_title(), "done": False})
        else:
            try:
                ident = int(input("ID: "))
            except ValueError:
                print("ID deve ser inteiro.")
                continue
            index = next(
                (i for i, task in enumerate(updated) if task["id"] == ident),
                None,
            )
            if index is None:
                print("ID inexistente.")
                continue
            if option == "3":
                updated[index]["title"] = ask_title()
            elif option == "4":
                updated[index]["done"] = not updated[index]["done"]
            else:
                del updated[index]
        try:
            save_tasks(path, updated)
        except OSError as error:
            print(f"Nao foi possivel salvar: {error}")
        else:
            tasks = updated
            print("Alteracao salva.")


def main() -> int:
    """Adquire lock exclusivo e retorna codigo de saida do aplicativo."""
    lock = DATA.with_suffix(".json.lock")
    acquired = False
    try:
        with lock.open("x", encoding="ascii") as stream:
            acquired = True
            stream.write(str(os.getpid()))
        session(DATA)
    except FileExistsError:
        print("Arquivo em uso. Verifique outras instancias antes")
        print("de remover um eventual lock residual.")
        return 1
    except (EOFError, KeyboardInterrupt):
        print("\nEncerrado.")
    except (OSError, ValueError, RecursionError) as error:
        print(f"Falha: {error}", file=sys.stderr)
        return 1
    finally:
        if acquired:
            try:
                lock.unlink(missing_ok=True)
            except OSError as error:
                print(f"Falha ao remover lock: {error}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
