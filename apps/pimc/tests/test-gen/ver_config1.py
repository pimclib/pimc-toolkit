import sys

from pimc_testgen.pimsm.group import Group
from pimc_testgen.pimsm.info import diff_g
from pimc_testgen.cisco_ios import MRouteParser

g = (
    Group("239.1.1.1")
    .join_range("10.1.2.20", "10.1.2.200")
    .join_rp("10.0.0.1")
    .rpt_prune_range("10.1.3.50", "10.1.3.200")
)
grp = g.addr


def main():
    if len(sys.argv) != 2:
        sys.exit(f"usage: {sys.argv[0]} <ios_output_file>")

    mrp = MRouteParser()
    ios_out_fn = sys.argv[1]
    try:
        with open(ios_out_fn) as ios_in:
            for line in ios_in:
                mrp.feed_line(line)
    except OSError as err:
        sys.exit(f"error: unable to read file '{ios_out_fn}': {err.strerror}")

    r = mrp.to_groups()
    if len(r) != 1:
        print(f"error: expecting 1 group, got {len(r)}")

    if len(r) > 0:
        if grp not in r:
            sys.exit(f"error: group {grp} not in the result")

        cmpr = diff_g(g, r[grp])

        if len(cmpr) > 0:
            sys.exit(f"error: group entries are not the same:\n\n {cmpr}")
        else:
            print("group entries are the same")


if __name__ == "__main__":
    main()
