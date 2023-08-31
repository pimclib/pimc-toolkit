import re

_addrRe = re.compile(r"^(\d+)\.(\d+)\.(\d+)\.(\d+)$")
# Local broadcast
_LB = 0xFFFFFFFF


def plen_mask(plen: int) -> int:
    """
    Computes a 32-bit unsigned integer whose value is equal to the
    subnet mask corresponding to the specified prefix length.

    :param plen: the prefix length. This value must be in range 0-32
     otherwise the returned value is undefined behavior.
    :return: 32-bit value of the subnet mask corresponding to the prefix
     length
    """
    return _LB - (_LB >> plen)


class IPv4Address(object):
    """
    An object representing an IPv4 address, e.g. 10.1.5.3.

    The IPv4Address implements all six comparison operations and is hashable.

    :param value: the 32 bit unsigned value of the address
    """

    def __init__(self, value: int) -> None:
        if value < 0 or value > _LB:
            raise ValueError(f"illegal IPv4 address 32-bit value {value}")
        self._value = value

    @classmethod
    def from_octets(cls, o1: int, o2: int, o3: int, o4: int) -> "IPv4Address":
        """
        Constructs an IPv4 address from the four individual octets

        :param o1: octet 1, the most significant octet
        :param o2: octet 2
        :param o3: octet 3
        :param o4: octet 4, the least significant octet
        :return: an IPv4 address
        :raises ValueError: if any of the octets is outside the range of
         values 0-255
        """
        if not (
            (-1 < o1 < 256) and (-1 < o2 < 256) and (-1 < o3 < 256) and (-1 < o3 < 256)
        ):
            raise ValueError(f"illegal IPv4 address octets {o1}.{o2}.{o3}.{o4}")
        return cls((o1 << 24) + (o2 << 16) + (o3 << 8) + o4)

    @classmethod
    def parse(cls, s: str) -> "IPv4Address":
        """
        Constructs an IPv4 address from the dotted decimal notation.

        :param s: the IP address in the dotted decimal notation
        :return: an IPv4 address
        :raises ValueError: if the specified string cannot be parsed as the
         dotted decimal notation or if any of the octets are outside of the
         range of values 0-255
        """
        m = _addrRe.match(s)

        if m is None:
            raise ValueError(f"illegal IPv4 address '{s}'")

        return cls.from_octets(int(m[1]), int(m[2]), int(m[3]), int(m[4]))

    def oct1(self) -> int:
        """
        Returns octet 1 of the address. Octet 1 is the most significant octet.

        :return: octet 1
        """
        return self._value >> 24

    def oct2(self) -> int:
        """
        Returns octet 2 of the address.

        :return: octet 2
        """
        return (self._value >> 16) & 0xFF

    def oct3(self) -> int:
        """
        Returns octet 3 of the address.

        :return: octet 3
        """
        return (self._value >> 8) & 0xFF

    def oct4(self) -> int:
        """
        Returns octet 4 of the address. Octet 4 is the least significant octet.

        :return: octet 4
        """
        return self._value & 0xFF

    @property
    def value(self) -> int:
        """
        Returns the 32-bit unsigned value of the IPv4 address

        :return: the 32-bit value of the IPv4 address
        """
        return self._value

    def __str__(self) -> str:
        return f"{self.oct1()}.{self.oct2()}.{self.oct3()}.{self.oct4()}"

    def __repr__(self) -> str:
        return (
            f"IPv4Address.from_octets("
            f"{self.oct1()},{self.oct2()},{self.oct3()},{self.oct4()})"
        )

    def __hash__(self) -> int:
        return hash(self._value)

    def __eq__(self, other) -> bool:
        if not isinstance(other, IPv4Address):
            return NotImplemented

        return self._value == other._value

    def __ne__(self, other) -> bool:
        return not (self == other)

    def __lt__(self, other) -> bool:
        if not isinstance(other, IPv4Address):
            return NotImplemented

        return self._value < other._value

    def __le__(self, other) -> bool:
        if not isinstance(other, IPv4Address):
            return NotImplemented

        return self._value <= other._value

    def __gt__(self, other) -> bool:
        if not isinstance(other, IPv4Address):
            return NotImplemented

        return self._value > other._value

    def __ge__(self, other) -> bool:
        if not isinstance(other, IPv4Address):
            return NotImplemented

        return self._value >= other._value

    def is_mcast(self) -> bool:
        """
        Checks if the address is multicast

        :return: True if the address is multicast, False otherwise
        """
        return (self._value >> 28) == 14

    def is_local_bcast(self) -> bool:
        """
        Checks if the address is local broadcast, i.e. 255.255.255.255

        :return: True if the address is local broadcast, False otherwise
        """
        return self._value == _LB

    def is_default(self) -> bool:
        """
        Checks if the address is default, i.e. 0.0.0.0

        :return: True if the address is default, False otherwise
        """
        return self._value == 0

    def is_loopback(self) -> bool:
        """
        Checks if the address is loopback, i.e. belongs to 127.0.0.0/8

        :return: True of the address is a loopback, False otherwise
        """
        return (self._value >> 24) == 127

    def is_mask(self) -> bool:
        """
        Check if the address is a subnet mask, i.e. its binary value
        begins with a contiguous sequence of 1s, followed by a contiguous
        sequence of 0s to the end. Either sequence can be empty, which
        means the other sequence has 32 elements, e.g. if the sequence of
        1s is empty then the sequence of 0s has 32 0s.

        :return: True if the address is a mask, False otherwise
        """
        if self._value == _LB:
            return True
        m = _LB - self._value
        return ((m + 1) & m) == 0

    def to_mask(self) -> int:
        """
        If the address is a subnet mask, returns the prefix length
        corresponding to the mask. If the address is not a mask, raises
        ValueError exception.

        :return: the prefix length value corresponding to the subnet
         mask
        :raise ValueError: if the address is not a subnet mask
        """
        if self._value == _LB:
            return 32
        if not self.is_mask():
            raise ValueError(f"{str(self)} is not mask")
        pl = 16
        m = _LB - self._value
        s = 16
        while s > 1:
            mn = m >> s
            sn = s >> 1
            if mn > 0:
                pl -= sn
                m = mn
            else:
                pl += sn

            s = sn

        if m == 1:
            return pl

        return pl - 1

    def apply_mask(self, mask) -> "IPv4Address":
        """
        Returns a new address whose value is a bitwise AND applied
        to the value of this address and that of the mask.

        :param mask: the address to apply as a mask
        :return: a new address whose value is equal to the bitwise AND
         operation applied to the values of this address and that of
         the mask
        """
        if not isinstance(mask, IPv4Address):
            raise NotImplemented

        return IPv4Address(self._value & mask._value)

    def apply_plen(self, plen: int) -> "IPv4Address":
        """
        Returns a new address whose most significant ``plen`` bits are
        to the ``plen`` most significant bits of this address and whose
        remaining bits are zero.

        :param plen: the prefix length to apply. Must be in range 0-32
        :return: a new address whose ``plen`` most significant bits are
         equal to the ``plen`` most significant bits of this address and
         whose remaining bits are zero
        :raises ValueError: if the ``plen`` parameter is not in the range
         0-32
        """
        if plen < 0 or plen > 32:
            raise ValueError(f"illegal prefix length {plen}")

        return IPv4Address(self._value & plen_mask(plen))
