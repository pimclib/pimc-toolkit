from pimc_testgen.pimsm.group import Group
from pimc_testgen.pimsm.info import dump_struct

g = (
    Group("239.1.1.1")
    .join_range("10.1.2.20", "10.1.2.200")
    .join_rp("10.0.0.1")
    .rpt_prune_range("10.1.3.50", "10.1.3.200")
)

grp, grp_ent = g.as_jp_group_entry()

cfg_d = {
    "logging": {
        "level": "debug",
    },
    "pim": {
        "neighbor": "10.1.1.3",
        "interface": "enp0s6",
    },
    "multicast": {
        grp: grp_ent
    }
}

print(dump_struct(cfg_d))
