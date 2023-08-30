from pathlib import Path
from typing import List

from .group import Group
from .update import Update
from .inverse_update import InverseUpdate
from .info import dump_config


def write_config(
        yaml_file: str,
        name: str,
        jp_cfg: List[Group],
        updates: List[Update],
        inverse_updates: List[InverseUpdate],
):
    p = Path(yaml_file)

    if p.exists():
        raise RuntimeError(f"file '{yaml_file}' exists")

    with open(yaml_file, "w") as yaml_out:
        yaml_out.write("---\n\n")
        yaml_out.write(dump_config(name, jp_cfg, updates, inverse_updates))
