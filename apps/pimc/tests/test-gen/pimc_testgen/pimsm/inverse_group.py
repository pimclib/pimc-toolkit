from typing import Optional, Tuple, Union, List, Mapping, Any

from .IPv4Address import IPv4Address
from .utils import addr
from .group_summary import GroupSummary


class InverseGroup:
    def __init__(self, maddr: Union[IPv4Address, str]) -> None:
        self.addr = addr(maddr)
        self.pruned_sources: List[IPv4Address] = list()
        self.pruned_rp: Optional[IPv4Address] = None

    def prune(self, sources: List[IPv4Address]) -> "InverseGroup":
        self.pruned_sources = sorted(self.pruned_sources + sources)
        return self

    def prune_rp(self, rp: Union[IPv4Address, str]) -> "InverseGroup":
        if self.pruned_rp is not None:
            raise RuntimeError(f"InverseGroup {self.addr}: RP already pruned")
        self.pruned_rp = addr(rp)
        return self

    def size(self) -> int:
        if len(self.pruned_sources) == 0 and self.pruned_rp is None:
            raise RuntimeError(
                f"group {self.addr}: no remaining pruned sources and no pruned RPT"
            )

        return (
            # Group header: IPv4 enc group   (8 bytes)
            #             + number of joins  (2 bytes)
            #             + number of prunes (2 bytes)
            12
            + len(self.pruned_sources) * 8
            + (8 if self.pruned_rp is not None else 0)
        )

    def has_pruned_sources(self) -> bool:
        return len(self.pruned_sources) > 0 or self.pruned_rp is not None

    def summary(self) -> GroupSummary:
        pruned_cnt = len(self.pruned_sources)
        if self.pruned_rp is not None:
            pruned_cnt += 1

        return GroupSummary(
            group=self.addr, joins=0, prunes=pruned_cnt, size=self.size()
        )

    def as_update(self) -> Tuple[str, Mapping[str, Any]]:
        d = dict()
        if self.pruned_rp is not None:
            d["Prune(*,G)"] = [str(self.pruned_rp)]
        if len(self.pruned_sources) > 0:
            d["Prune(S,G)"] = [str(a) for a in self.pruned_sources]
        return str(self.addr), d

    def take_prunes(self, count: int) -> Tuple["InverseGroup", "InverseGroup"]:
        if count < 1:
            raise ValueError(
                f"group {self.addr}: "
                f"{count} is invalid number of prunes to take , "
                f"must greater than 0"
            )

        if not self.has_pruned_sources():
            raise RuntimeError(f"group {self.addr}: no remaining pruned sources")

        a = InverseGroup(self.addr)
        b = InverseGroup(self.addr)

        if self.pruned_rp:
            a.prune_rp(self.pruned_rp)
            count -= 1

        if count > len(self.pruned_sources):
            count = len(self.pruned_sources)

        a.prune(self.pruned_sources[:count])
        b.prune(self.pruned_sources[count:])

        return a, b
