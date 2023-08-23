from pathlib import Path
from typing import List

from .Group import Group
from .Update import Update
from .info import dump_config


def write_config(yaml_file: str, name: str, jp_cfg: List[Group], updates: List[Update]):
    p = Path(yaml_file)

    if p.exists():
        raise RuntimeError(f"file '{yaml_file}' exists")

    with open(yaml_file, "w") as yaml_out:
        yaml_out.write("---\n\n")
        yaml_out.write(dump_config(name, jp_cfg, updates))
