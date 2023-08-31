from typing import Iterable, List

from .group import Group
from .inverse_group import InverseGroup
from .inverse_update import InverseUpdate


def _max_sources(sz: int) -> int:
    return int((sz - 12) / 8)


class _InversePackBuilder:
    def __init__(self):
        self.updates = [InverseUpdate()]

    def uq(self):
        for u in self.updates:
            yield u

        while True:
            self.updates.append(InverseUpdate())
            yield self.updates[-1]

    def place(self, g: InverseGroup) -> None:
        if g.has_pruned_sources():
            for u in self.uq():
                if u.full():
                    continue

                nsrc = _max_sources(u.remaining())

                a, b = g.take_prunes(nsrc)
                u.add(a)

                g = b
                if not g.has_pruned_sources():
                    break

    def result(self) -> List[InverseUpdate]:
        if self.updates[-1].empty():
            self.updates.pop(-1)

        return self.updates


def _inverse(g: Group) -> InverseGroup:
    ig = InverseGroup(g.addr)
    ig.pruned_rp = g.rp
    ig.pruned_sources.extend(g.joined_sources)
    return ig


def inverse_pack(groups: Iterable[Group]) -> List[InverseUpdate]:
    pb = _InversePackBuilder()
    for g in groups:
        pb.place(_inverse(g))

    return pb.result()
