"""Conversoes de comprimento, massa e temperatura sem dependencias."""

import math


FACTORS: dict[str, dict[str, float]] = {
    "comprimento": {"mm": 0.001, "cm": 0.01, "m": 1.0, "km": 1000.0},
    "massa": {"mg": 0.000001, "g": 0.001, "kg": 1.0, "t": 1000.0},
}
TEMPERATURE: dict[str, tuple[float, float]] = {
    "c": (1.0, 273.15),
    "f": (5.0 / 9.0, 459.67),
    "k": (1.0, 0.0),
}


def convert(value: float, category: str, source: str, target: str) -> float:
    """Converte um valor sem modificar estado externo.

    Args:
        value: Valor finito; massa/comprimento devem ser nao negativos.
        category: comprimento, massa ou temperatura.
        source: Unidade de origem em minusculas.
        target: Unidade de destino em minusculas.
    Returns:
        Valor convertido na unidade de destino.
    Raises:
        ValueError: Categoria/unidade invalida, valor fora do dominio ou
            resultado nao representavel por float finito.
    """
    if not math.isfinite(value):
        raise ValueError("Informe um valor finito.")
    if category == "temperatura":
        if source not in TEMPERATURE or target not in TEMPERATURE:
            raise ValueError("Unidade de temperatura invalida.")
        scale, offset = TEMPERATURE[source]
        if value < -offset:
            raise ValueError("Temperatura abaixo do zero absoluto.")
        kelvin = (value + offset) * scale
        scale_out, offset_out = TEMPERATURE[target]
        result = kelvin / scale_out - offset_out
    else:
        units = FACTORS.get(category)
        if units is None or source not in units or target not in units:
            raise ValueError("Categoria ou unidade invalida.")
        if value < 0:
            raise ValueError("Massa e comprimento devem ser nao negativos.")
        result = value * (units[source] / units[target])
    if not math.isfinite(result):
        raise ValueError("Resultado fora do limite numerico.")
    return result


def main() -> int:
    """Repete leituras invalidas e encerra em EOF ou interrupcao."""
    try:
        while True:
            category = input(
                "comprimento | massa | temperatura | sair: "
            ).strip().casefold()
            if category == "sair":
                return 0
            if category == "temperatura":
                units = list(TEMPERATURE)
            elif category in FACTORS:
                units = list(FACTORS[category])
            else:
                print("Categoria invalida.")
                continue
            print("Unidades:", ", ".join(units))
            source = input("Origem: ").strip().casefold()
            target = input("Destino: ").strip().casefold()
            if source not in units or target not in units:
                print("Unidade invalida.")
                continue
            while True:
                raw = input("Valor (ponto decimal): ").strip()
                try:
                    result = convert(float(raw), category, source, target)
                except ValueError as error:
                    print(f"Valor invalido: {error}")
                else:
                    print(f"Resultado: {result:.12g} {target}")
                    break
    except (EOFError, KeyboardInterrupt):
        print("\nEncerrado.")
    except (OSError, UnicodeError) as error:
        print(f"Falha de entrada/saida: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
