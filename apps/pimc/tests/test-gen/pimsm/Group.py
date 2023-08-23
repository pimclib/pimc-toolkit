import dataclasses
from typing import Optional, Tuple, Union, List, Mapping, Any

from .IPv4Address import IPv4Address
from .utils import addr, addr_range


@dataclasses.dataclass
class GroupSummary:
    group: IPv4Address
    joins: int
    prunes: int
    size: int


class Group:
    def __init__(self, maddr: Union[IPv4Address, str]) -> None:
        self.addr = addr(maddr)
        self.joined_sources: List[IPv4Address] = list()
        self.rp: Optional[IPv4Address] = None
        self.rpt_pruned_sources: List[IPv4Address] = list()

    def join(self, sources: List[IPv4Address]) -> "Group":
        self.joined_sources = sorted(self.joined_sources + sources)
        return self

    def join_range(
        self, start: Union[IPv4Address, str], end: Union[IPv4Address, str]
    ) -> "Group":
        return self.join(addr_range(start, end))

    def join_rp(self, rp: Union[IPv4Address, str]) -> "Group":
        if self.rp is not None:
            raise RuntimeError(f"Group {self.addr}: RP already joined")
        self.rp = addr(rp)
        return self

    def rpt_prune(self, sources: List[IPv4Address]) -> "Group":
        if self.rp is None:
            raise RuntimeError(
                f"Group {self.addr}: RP must be set before pruning sources from rpt"
            )
        self.rpt_pruned_sources = sorted(self.rpt_pruned_sources + sources)
        if len(self.rpt_pruned_sources) > 180:
            raise RuntimeError(
                f"group {self.addr} has {len(self.rpt_pruned_sources)} RPT pruned sources, "
                f"which is greater than 180, the maximum number of RPT pruned sources"
            )
        return self

    def rpt_prune_range(
        self, start: Union[IPv4Address, str], end: Union[IPv4Address, str]
    ) -> "Group":
        return self.rpt_prune(addr_range(start, end))

    def size(self) -> int:
        if len(self.joined_sources) == 0 and self.rp is None:
            raise RuntimeError(f"group {self.addr}: no remaining sources and no RPT")

        return (
            # Group header: IPv4 enc group   (8 bytes)
            #             + number of joins  (2 bytes)
            #             + number of prunes (2 bytes)
            12
            + len(self.joined_sources) * 8
            + (8 if self.rp is not None else 0)
            + len(self.rpt_pruned_sources) * 8
        )

    def has_joined_sources(self) -> bool:
        return len(self.joined_sources) > 0

    def has_rpt(self) -> bool:
        return self.rp is not None

    def rpt_size(self) -> int:
        if self.rp is None:
            return 0
        return 12 + (len(self.rpt_pruned_sources) + 1) * 8

    def rp_and_prunes_size(self) -> int:
        if self.rp is None:
            raise RuntimeError(f"group {self.addr}: no RPT")
        return (len(self.rpt_pruned_sources) + 1) * 8

    def summary(self) -> GroupSummary:
        joined_cnt = len(self.joined_sources)
        if self.rp is not None:
            joined_cnt += 1
        pruned_cnt = len(self.rpt_pruned_sources)

        return GroupSummary(
            group=self.addr, joins=joined_cnt, prunes=pruned_cnt, size=self.size()
        )

    def as_update(self) -> Tuple[str, Mapping[str, Any]]:
        d = dict()
        if len(self.joined_sources) > 0:
            d["Join(S,G)"] = [str(a) for a in self.joined_sources]
        if self.rp is not None:
            d["Join(*,G)"] = [str(self.rp)]
        if len(self.rpt_pruned_sources) > 0:
            d["Prune(S,G,rpt)"] = [str(a) for a in self.rpt_pruned_sources]
        return str(self.addr), d

    def as_jp_group_entry(self) -> Tuple[str, Mapping[str, Any]]:
        d = dict()
        if self.rp is not None:
            rpt = dict()
            rpt["RP"] = str(self.rp)
            if len(self.rpt_pruned_sources) > 0:
                rpt["Prune"] = [str(a) for a in self.rpt_pruned_sources]
            d["Join*"] = rpt
        if len(self.joined_sources) > 0:
            d["Join"] = [str(a) for a in self.joined_sources]
        return str(self.addr), d

    def take_sg_joins(self, count: int) -> Tuple["Group", "Group"]:
        if count < 1:
            raise ValueError(
                f"group {self.addr}: "
                f"invalid number of joins to take {count}, "
                f"must greater than 0"
            )
        if count > len(self.joined_sources):
            count = len(self.joined_sources)

        if len(self.joined_sources) == 0:
            raise RuntimeError(f"group {self.addr}: no remaining sources")

        a = Group(self.addr).join(self.joined_sources[:count])
        b = Group(self.addr)
        if count < len(self.joined_sources):
            b.join(self.joined_sources[count:])
        if self.rp is not None:
            b.join_rp(self.rp).rpt_prune(self.rpt_pruned_sources)

        return a, b

    def take_sg_joins_and_rpt(self, count: int) -> Tuple["Group", "Group"]:
        if count < 0:
            raise ValueError(
                f"group {self.addr}: "
                f"invalid number of joins to take {count}, "
                f"must greater than or equal to 0"
            )
        if self.rp is None:
            raise RuntimeError(f"group {self.addr}: no RPT")

        if count > len(self.joined_sources):
            count = len(self.joined_sources)

        a = (
            Group(self.addr)
            .join(self.joined_sources[:count])
            .join_rp(self.rp)
            .rpt_prune(self.rpt_pruned_sources)
        )

        b = Group(self.addr).join(self.joined_sources[count:])

        return a, b
