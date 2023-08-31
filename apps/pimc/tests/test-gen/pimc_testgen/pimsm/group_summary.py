import dataclasses

from .IPv4Address import IPv4Address


@dataclasses.dataclass
class GroupSummary:
    group: IPv4Address
    joins: int
    prunes: int
    size: int
