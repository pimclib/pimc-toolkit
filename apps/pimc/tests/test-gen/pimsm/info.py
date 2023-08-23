import io
import dataclasses
from typing import Iterable, List

import pandas as pd

from .Group import Group
from .Update import Update
from .utils import dump_struct


def updates_summary(updates: List[Update]) -> pd.DataFrame:
    return pd.DataFrame.from_records(
        {"update": n + 1, "update_size": u.size(), "update_rem_size": u.remaining()}
        | dataclasses.asdict(g.summary())
        for n, u in enumerate(updates)
        for g in u.groups
    )


def dump_config(name: str, jp_cfg: List[Group], updates: List[Update]) -> str:
    jp_cfg_d = dict()
    for maddr, ge in [g.as_jp_group_entry() for g in jp_cfg]:
        if maddr in jp_cfg_d:
            raise RuntimeError(f"duplicate group {maddr} in J/P config")
        jp_cfg_d[maddr] = ge

    return dump_struct(
        {
            "name": name,
            "multicast": jp_cfg_d,
            "verify": [u.as_dict() for u in updates],
        }
    )


class _ErrBuf:
    def __init__(self, caption):
        self.buf = io.StringIO()
        self.buf.write(caption)
        self.used = False

    def write(self, msg):
        self.buf.write(msg)
        self.used = True

    def getvalue(self) -> str:
        if not self.used:
            return ""

        return self.buf.getvalue()


def diff_g(jp_g: Group, u_g: Group) -> str:
    buf = _ErrBuf(f"Group {jp_g.addr}:\n")

    org_joined = set(jp_g.joined_sources)
    rc_joined = set(u_g.joined_sources)

    missing_joined = org_joined - rc_joined
    if len(missing_joined) > 0:
        buf.write("  missing joined sources:\n")
        for s in sorted(missing_joined):
            buf.write(f"    {s}\n")

    extraneous_joined = rc_joined - org_joined
    if len(extraneous_joined) > 0:
        buf.write("  extraneous joined sourced:\n")
        for s in sorted(extraneous_joined):
            buf.write(f"    {s}\n")

    if jp_g.rp != u_g.rp:
        buf.write("  RP mismatch: ")
        if u_g.rp is None:
            buf.write(f"missing RP {jp_g.rp}\n")
        elif jp_g is None:
            buf.write(f"extraneous RP {u_g.rp}\n")
        else:
            buf.write(f"original RP {jp_g.rp} != update RP {u_g.rp}\n")

    org_pruned = set(jp_g.rpt_pruned_sources)
    rc_pruned = set(u_g.rpt_pruned_sources)

    missing_pruned = org_pruned - rc_pruned
    if len(missing_pruned) > 0:
        buf.write("  missing pruned sources:\n")
        for s in sorted(missing_pruned):
            buf.write(f"    {s}\n")

    extraneous_pruned = rc_pruned - org_pruned
    if len(extraneous_pruned) > 0:
        buf.write("  extraneous pruned sourced:\n")
        for s in sorted(extraneous_pruned):
            buf.write(f"    {s}\n")

    return buf.getvalue()


def diff_jpcfg_vs_updates(jp_cfg: Iterable[Group], updates: List[Update]):
    buf = io.StringIO()
    orig_jp = dict()

    for g in jp_cfg:
        if g.addr in orig_jp:
            raise RuntimeError(f"duplicate group {g} in J/P configuration")

        orig_jp[g.addr] = g

    rc_jp = dict()

    for u in updates:
        for g in u.groups:
            ge = rc_jp.get(g.addr)
            if ge is None:
                ge = Group(g.addr)
                rc_jp[g.addr] = ge
            if g.has_joined_sources():
                ge.join(g.joined_sources)
            if g.rp is not None:
                ge.rp = g.rp
                if len(g.rpt_pruned_sources) > 0:
                    ge.rpt_prune(g.rpt_pruned_sources)

    org_maddrs = set(orig_jp.keys())
    rc_maddrs = set(rc_jp.keys())

    missing_maddrs = org_maddrs - rc_maddrs
    if len(missing_maddrs) > 0:
        buf.write("Updates do not contain the following groups:\n")
        for maddr in sorted(missing_maddrs):
            buf.write(f"  {maddr}\n")

    extraneous_maddrs = rc_maddrs - org_maddrs
    if len(extraneous_maddrs) > 0:
        buf.write("Updates contain the following extraneous groups:\n")
        for maddr in sorted(extraneous_maddrs):
            buf.write(f" {maddr}\n")

    for maddr in org_maddrs:
        rg = rc_jp.get(maddr)
        if rg is not None:
            buf.write(diff_g(orig_jp[maddr], rg))

    return buf.getvalue()
