from pathlib import Path

from IPython.display import display, HTML

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

    df_sum = updates_summary(updates)

    if len(df_sum) > 0:
        gb = df_sum.groupby(["update", "update_size", "update_rem_size"])
        for h, df in gb:
            display(
                HTML(f"<p><b>Update {h[0]}</b>: size {h[1]}, remaining size {h[2]}</p>")
            )
            display(HTML(df[["group", "joins", "prunes", "size"]].to_html(index=False)))
    else:
        print("Empty updates summary dataframe")

    diff = diff_jpcfg_vs_updates(gs, updates)
    if len(diff) > 0:
        print(diff)
    else:
        yml = str(Path.home() / "tmp/new_pv_cfg.yml")
        write_config(yml, name, gs, updates, inverse_updates)
