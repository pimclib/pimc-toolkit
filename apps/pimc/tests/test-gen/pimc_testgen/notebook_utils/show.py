import io

from IPython.display import display, HTML

from .jupyter import IPyStatus, get_ipystatus

_nb = get_ipystatus() != IPyStatus.NotInIPython


class Tag:
    def __init__(self, tag: str, *cnt):
        self.tag = tag
        self.cnt = cnt


def show_text(*args):
    buf = io.StringIO()

    def unfold(el):
        if isinstance(el, Tag):
            if _nb:
                buf.write(f"<{el.tag}>")
            for iel in el.cnt:
                unfold(iel)
            if _nb:
                buf.write(f"</{el.tag}>")
        else:
            buf.write(f"{el}")

    for arg in args:
        unfold(arg)

    t = buf.getvalue()
    if _nb:
        display(HTML(t))
    else:
        print(t)


def show_df(df, *, index: bool = False):
    if _nb:
        display(HTML(df.to_html(index=index)))
    else:
        print(df.to_string(index=index))
