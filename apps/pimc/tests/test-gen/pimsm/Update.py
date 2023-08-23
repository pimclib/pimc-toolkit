from typing import List, Mapping, Any

from .Group import Group


class Update:
    def __init__(self, max_size: int = 1466) -> None:
        self.max_size = max_size
        self.groups: List[Group] = list()

    def size(self) -> int:
        return sum(g.size() for g in self.groups)

    def remaining(self) -> int:
        return self.max_size - self.size()

    def add(self, g: Group) -> None:
        r = self.remaining()
        gsz = g.size()
        if gsz > r:
            raise RuntimeError(
                f"size of group {g.addr} is {gsz}, which is greater "
                f"than the remaining space in this update ({r})"
            )
        self.groups.append(g)

    def as_dict(self) -> Mapping[str, Any]:
        return {ga: v for ga, v in (g.as_update() for g in self.groups)}

    def empty(self) -> bool:
        return len(self.groups) == 0

    def full(self) -> bool:
        return self.remaining() < 20
