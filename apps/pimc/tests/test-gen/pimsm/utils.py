import io
from typing import List, Union

from .IPv4Address import IPv4Address


def addr(a: Union[IPv4Address, str]) -> IPv4Address:
    if isinstance(a, IPv4Address):
        return a
    elif isinstance(a, str):
        return IPv4Address.parse(a)
    else:
        raise TypeError(f"expecting IPv4Address or str, not {type(a).__name__}")


def addr_range(
    start: Union[IPv4Address, str],
    end: Union[IPv4Address, str],
) -> List[IPv4Address]:
    a = addr(start)
    b = addr(end)
    if a > b:
        raise ValueError(f"start address {a} > end address {b}")
    if a == b:
        return [a]

    return [IPv4Address(v) for v in range(a.value, b.value + 1)]


def dump_struct(s) -> str:
    buf = io.StringIO()

    def writei(x, indent, first_indent):
        if isinstance(x, dict):
            for k, v in x.items():
                if first_indent:
                    buf.write(" " * indent)
                else:
                    first_indent = True
                buf.write(k)
                buf.write(":\n")
                writei(v, indent + 2, True)
        elif isinstance(x, list):
            for e in x:
                if first_indent:
                    buf.write(" " * indent)
                else:
                    first_indent = True
                buf.write("- ")
                writei(e, indent + 2, False)
        else:
            if first_indent:
                buf.write(" " * indent)
            buf.write(str(x))
            buf.write("\n")

    writei(s, 0, True)
    return buf.getvalue()
