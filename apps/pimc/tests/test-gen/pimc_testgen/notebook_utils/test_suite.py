from pathlib import Path

from .show import show_text, show_df, Tag

from ..pimsm import (
    updates_summary,
    diff_jpcfg_vs_updates,
    write_config,
    pack,
    inverse_pack,
)


def test_suite(name: str, *gs):
    updates = pack(gs)
    inverse_updates = inverse_pack(gs)

    df_us = updates_summary(updates)

    if len(df_us) > 0:
        show_text(Tag("H1", "Update Summaries"))
        gb = df_us.groupby(["update", "update_size", "update_rem_size"])
        for h, df in gb:
            show_text(
                Tag(
                    "p",
                    Tag("b", f"Update {h[0]}"),
                    ": ",
                    f"size {h[1]}, remaining size {h[2]}",
                )
            )
            show_df(df[["group", "joins", "prunes", "size"]])
    else:
        show_text(Tag("p", "Empty updates summary dataframe"))

    df_ius = updates_summary(inverse_updates)

    if len(df_ius) > 0:
        show_text(Tag("H1", "Inverse Update Summaries"))
        gb = df_ius.groupby(["update", "update_size", "update_rem_size"])
        for h, df in gb:
            show_text(
                Tag(
                    "p",
                    Tag("b", f"Update {h[0]}"),
                    ": ",
                    f"size {h[1]}, remaining size {h[2]}",
                )
            )
            show_df(df[["group", "joins", "prunes", "size"]])
    else:
        show_text(Tag("p", "Empty inverse updates summary dataframe"))

    diff = diff_jpcfg_vs_updates(gs, updates)
    if len(diff) > 0:
        show_text(Tag("H1", "Error:\n"), Tag("pre", "\n", diff, "\n"))
    else:
        yml = str(Path.home() / "tmp/new_pv_cfg.yml")
        write_config(yml, name, gs, updates, inverse_updates)
