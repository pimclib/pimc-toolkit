import re
from collections import defaultdict
from typing import Optional, List, MutableMapping, Mapping

from ..pimsm.IPv4Address import IPv4Address
from ..pimsm.group import Group

_tre_addr = r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}"
_re_rp_mroute = re.compile(
    r"^\(\*\s*,\s*({A})\)\s*,\s*\S+,\s+RP\s+({A}),\s+flags:(.*)$".format(A=_tre_addr),
)
_re_src_mroute = re.compile(
    r"^\(({A})\s*,\s*({A})\)\s*,\s*\S+,\s+flags:(.*)$".format(A=_tre_addr),
)
_spt_flags = {"", "T"}


class GE:

    def __init__(self):
        self.rp: Optional[IPv4Address] = None
        self.spt_joined_sources: List[IPv4Address] = list()
        self.rpt_pruned_sources: List[IPv4Address] = list()

    def join_rp(self, rp: IPv4Address) -> None:
        self.rp = rp

    def spt_join_source(self, s: IPv4Address) -> None:
        self.spt_joined_sources.append(s)

    def rpt_prune_source(self, s: IPv4Address) -> None:
        self.rpt_pruned_sources.append(s)

    def to_group(self, group: IPv4Address) -> Group:
        g = Group(group)
        g.join(self.spt_joined_sources)
        if self.rp is not None:
            g.join_rp(self.rp)
        g.rpt_prune(self.rpt_pruned_sources)
        return g


class MRouteParser:

    def __init__(self):
        self.groups: MutableMapping[IPv4Address, GE] = defaultdict(GE)

    def feed_line(self, line: str) -> None:
        m = _re_src_mroute.match(line)

        if m is not None:
            grp = IPv4Address.parse(m[2])
            src = IPv4Address.parse(m[1])
            flags = m[3].strip()

            if flags in _spt_flags:
                self.groups[grp].spt_join_source(src)
            elif flags == "PR":
                self.groups[grp].rpt_prune_source(src)
            else:
                print(f"warning: group {grp}, source {src}: unrecognized flags '{flags}'")

            return

        m = _re_rp_mroute.match(line)

        if m is not None:
            grp = IPv4Address.parse(m[1])
            rp = IPv4Address.parse(m[2])
            flags = m[3].strip()

            if flags == "S":
                self.groups[grp].join_rp(rp)
            else:
                print(f"warning: group {grp}, RP {rp}: unrecognized flags '{flags}'")

    def to_groups(self) -> Mapping[IPv4Address, Group]:
        return {grp: ge.to_group(grp) for grp, ge in self.groups.items()}
