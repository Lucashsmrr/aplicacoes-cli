"""Organiza arquivos por extensao; simula por padrao e ignora symlinks."""

import argparse
import os
import shutil
import sys
from pathlib import Path


CATEGORIES: dict[str, frozenset[str]] = {
    "imagens": frozenset({".png", ".jpg", ".jpeg", ".gif", ".webp", ".svg"}),
    "documentos": frozenset({
        ".pdf", ".txt", ".docx", ".odt", ".csv", ".xlsx",
    }),
    "audio": frozenset({".mp3", ".wav", ".flac", ".ogg"}),
    "video": frozenset({".mp4", ".mkv", ".avi", ".webm"}),
    "compactados": frozenset({".zip", ".gz", ".rar", ".7z", ".tar"}),
}
BY_EXTENSION = {
    extension: category
    for category, extensions in CATEGORIES.items()
    for extension in extensions
}


def collect(root: Path, recursive: bool) -> list[Path]:
    """Lista arquivos regulares, excluindo saida e links simbolicos.

    Args:
        root: Diretorio de origem absoluto.
        recursive: Se verdadeiro, visita subdiretorios.
    Returns:
        Snapshot ordenado dos caminhos elegiveis.
    Raises:
        OSError: Diretorio nao pode ser inspecionado.
    """
    pending = [root]
    files: list[Path] = []
    output = root / "_organizados"
    while pending:
        directory = pending.pop()
        for entry in directory.iterdir():
            if entry == output or entry.is_symlink():
                continue
            if entry.is_file():
                files.append(entry)
            elif recursive and entry.is_dir():
                pending.append(entry)
    return sorted(files)


def move_exclusive(source: Path, directory: Path) -> Path:
    """Copia com criacao exclusiva e remove origem somente apos sucesso.

    Args:
        source: Arquivo regular, nao modificado durante a operacao.
        directory: Diretorio de destino confiavel.
    Returns:
        Destino criado; duplicatas recebem um sufixo numerico.
    Raises:
        OSError: Falha de IO; se a exclusao da origem falhar, ambas as
            copias permanecem. Nao ha atomicidade do lote inteiro.
    """
    if source.is_symlink() or not source.is_file():
        raise OSError(f"Origem deixou de ser arquivo regular: {source}")
    if directory.parent.is_symlink() or directory.is_symlink():
        raise OSError("Destino nao pode ser link simbolico.")
    directory.mkdir(parents=True, exist_ok=True)
    for number in range(10000):
        name = source.name if number == 0 else (
            f"{source.stem}_{number}{source.suffix}"
        )
        target = directory / name
        try:
            output = target.open("xb")
        except FileExistsError:
            continue
        try:
            with output, source.open("rb") as input_file:
                shutil.copyfileobj(input_file, output)
                output.flush()
                os.fsync(output.fileno())
        except (OSError, KeyboardInterrupt):
            target.unlink(missing_ok=True)
            raise
        source.unlink()
        return target
    raise OSError("Limite de nomes duplicados atingido.")


def main() -> int:
    """Interpreta argumentos e executa simulacao ou movimentacao."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("diretorio", type=Path)
    parser.add_argument("--recursivo", action="store_true")
    parser.add_argument("--aplicar", action="store_true")
    args = parser.parse_args()
    try:
        root = args.diretorio.expanduser().resolve(strict=True)
        if not root.is_dir():
            parser.error("A origem deve ser um diretorio.")
        output = root / "_organizados"
        if output.is_symlink():
            raise OSError("A pasta _organizados nao pode ser um symlink.")
        files = collect(root, args.recursivo)
        failures = 0
        for source in files:
            category = BY_EXTENSION.get(source.suffix.lower(), "outros")
            directory = output / category
            if not args.aplicar:
                print(f"[SIMULACAO] {source} -> {directory}/")
                continue
            try:
                target = move_exclusive(source, directory)
                print(f"{source} -> {target}")
            except OSError as error:
                failures += 1
                print(f"Falha em {source}: {error}", file=sys.stderr)
        print(f"Arquivos encontrados: {len(files)}; falhas: {failures}.")
        return int(failures > 0)
    except (OSError, RuntimeError, ValueError) as error:
        print(f"Falha: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nInterrompido; arquivos ja movidos permanecem no destino.")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
