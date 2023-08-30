from typing import Optional, Iterable, List

from .group import Group
from .update import Update


def _max_sources(sz: int) -> int:
    return int((sz - 12) / 8)


class _PackBuilder:
    def __init__(self):
        self.updates = [Update()]

    def uq(self):
        for u in self.updates:
            yield u

        while True:
            self.updates.append(Update())
            yield self.updates[-1]

    def find_update_for_rpt(self, g: Group) -> Optional[Update]:
        if not g.has_rpt():
            return None

        rpt_sz = g.rpt_size()

        # Sanity check
        if rpt_sz > self.updates[0].max_size:
            raise ValueError(
                f"group {g.addr}: has {len(g.rpt_pruned_sources)} Prune(S,G,rpt) entries, "
                f"which results in encoded size {rpt_sz}, which is greater than the "
                f"maximum available size {self.updates[0].max_size} in any update"
            )

        for u in self.uq():
            if u.remaining() >= rpt_sz:
                return u

    def place(self, g: Group) -> None:
        rpt_update = self.find_update_for_rpt(g)

        if g.has_joined_sources():
            for u in self.uq():
                if u.full():
                    continue

                if u != rpt_update:
                    nsrc = _max_sources(u.remaining())

                    a, b = g.take_sg_joins(nsrc)
                    u.add(a)

                    g = b
                    if not g.has_joined_sources():
                        break
                else:
                    rpt_sz = g.rp_and_prunes_size()
                    nsrc = _max_sources(u.remaining() - rpt_sz)

                    a, b = g.take_sg_joins_and_rpt(nsrc)
                    u.add(a)
                    rpt_update = None

                    g = b
                    if not g.has_joined_sources():
                        break

        if rpt_update is not None:
            rpt_update.add(g)

    def result(self) -> List[Update]:
        if self.updates[-1].empty():
            self.updates.pop(-1)

        return self.updates


def pack(groups: Iterable[Group]) -> List[Update]:
    pb = _PackBuilder()
    for g in groups:
        pb.place(g)

    return pb.result()
