from enum import Enum

import IPython.display as ipd


class IPyStatus(Enum):
    NotInIPython = 0
    InTerminal = 1
    InNotebook = 2
    Unknown = -1


def get_ipystatus() -> IPyStatus:
    """
    Returns IPyStatus value indicating if the script is running in
    ipython and if so in what kind

    :return: IPyStatus value
    """
    try:
        shell = get_ipython().__class__.__name__
        if shell == "ZMQInteractiveShell":
            return IPyStatus.InNotebook
        elif shell == "TerminalInteractiveShell":
            return IPyStatus.InTerminal
        else:
            return IPyStatus.Unknown
    except NameError:
        return IPyStatus.NotInIPython


def notebook_width(pct: int = 97) -> None:
    """
    Set the notebook width to the value pct.

    :param pct: the percentage of the browser window width
    :return:
    """
    if not (0 < pct <= 100):
        raise ValueError(f"invalid width value '{pct}'")

    ipd.display(ipd.HTML(f"<style>.container {{ width:{pct}% !important; }}</style>"))
