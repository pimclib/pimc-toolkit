# PIMC Network Toolkit

PIMC stands for PIM client, which is an app that can send
[PIM v2](https://datatracker.ietf.org/doc/html/rfc7761.html) multicast joins.
The toolkit and the library are named after this app.

PIMC toolkit is a set of network centric utility apps and the C++ library that
is used to build them.

## Apps

### pimc

**pimc** is a PIM sparce-mode v2 client. It's documented in 
[pimc](docs/man-source/pimc.rst)

Example of running **pimc**.

Given the configuration file

```yaml
---

logging:
  level: debug

pim:
  neighbor: 10.1.1.3
  interface: enp0s6

multicast:
  239.1.2.3:
    Join*:
      RP: 10.0.0.1
      Prune:
        - 10.1.3.100
        - 10.1.3.101
        - 10.1.3.102
    Join:
      - 10.1.2.100
      - 10.1.2.101
```

Running **pimc** produces the following output:

```text
$ pimc titan-pimc.yml
18:09:25.695662 DEBUG: PIM SM config:
PIM sparse-mode:
  neighbor: 10.1.1.3
  interface: enp0s6, #3, addr 10.1.1.20
  hello period: 30s
  hello hold time: 105s
  join/prune period: 60s
  join/prune hold time: 210s
  generation ID: 79b3e424

18:09:25.695662 DEBUG: Join/Prune Config:
 Join/Prune config:
  239.1.2.3
    Join(*,G): RP 10.0.0.1
    Prune(S,G,rpt):
      10.1.3.100
      10.1.3.101
      10.1.3.102
    Join(S,G):
      10.1.2.100
      10.1.2.101

18:09:25.695662 DEBUG: Will be sending 1 update:
Update #1 with 1 group:
Group 239.1.2.3
 3 joins, 3 prunes
 Joins:
   10.1.2.100
   10.1.2.101
   10.0.0.1, WC, rpt
 Prunes:
   10.1.3.100, rpt
   10.1.3.101, rpt
   10.1.3.102, rpt

18:09:25.695662 DEBUG: Once terminated will send 1 inverse update:
Update #1 with 1 group:
Group 239.1.2.3
 0 joins, 3 prunes
 Joins:
 Prunes:
   10.0.0.1, WC, rpt
   10.1.2.100
   10.1.2.101

18:09:25.695662 DEBUG: created IPv4 PIM socket
18:09:25.695662 DEBUG: bound the IPv4 PIM socket to device enp0s6 (#3)
18:09:25.695662 DEBUG: sent IPv4 PIM Hello [holdtime 105s, DR priority 0, generation ID 79b3e424]
18:09:56.695662 DEBUG: sent IPv4 PIM Hello [holdtime 105s, DR priority 0, generation ID 79b3e424]
18:10:25.695662 DEBUG: sent IPv4 PIM Join/Prune Update packet #1 with 1 group, neighbor 10.1.1.3, holdtime 210s
Group 239.1.2.3
 3 joins, 3 prunes
 Joins:
   10.1.2.100
   10.1.2.101
   10.0.0.1, WC, rpt
 Prunes:
   10.1.3.100, rpt
   10.1.3.101, rpt
   10.1.3.102, rpt

18:10:26.695662 DEBUG: sent IPv4 PIM Hello [holdtime 105s, DR priority 0, generation ID 79b3e424]
18:10:56.695662 DEBUG: sent IPv4 PIM Hello [holdtime 105s, DR priority 0, generation ID 79b3e424]
```

A Cisco IOS router does see the host where **pimc** is running an a PIM neighbor,
and it accepts the join/prune state sent by the host:

```text
R1#show ip pim neighbor
PIM Neighbor Table
Mode: B - Bidir Capable, DR - Designated Router, N - Default DR Priority,
      P - Proxy Capable, S - State Refresh Capable, G - GenID Capable
Neighbor          Interface                Uptime/Expires    Ver   DR
Address                                                            Prio/Mode
10.0.13.2         GigabitEthernet1/0       00:01:28/00:01:39 v2    1 / DR S P G
10.0.12.2         GigabitEthernet2/0       00:01:29/00:01:40 v2    1 / DR S P G
10.1.1.20         GigabitEthernet3/0       00:00:19/00:01:25 v2    0 / G

R1#show ip mroute
IP Multicast Routing Table
Flags: D - Dense, S - Sparse, B - Bidir Group, s - SSM Group, C - Connected,
       L - Local, P - Pruned, R - RP-bit set, F - Register flag,
       T - SPT-bit set, J - Join SPT, M - MSDP created entry, E - Extranet,
       X - Proxy Join Timer Running, A - Candidate for MSDP Advertisement,
       U - URD, I - Received Source Specific Host Report,
       Z - Multicast Tunnel, z - MDT-data group sender,
       Y - Joined MDT-data group, y - Sending to MDT-data group,
       G - Received BGP C-Mroute, g - Sent BGP C-Mroute,
       Q - Received BGP S-A Route, q - Sent BGP S-A Route,
       V - RD & Vector, v - Vector
Outgoing interface flags: H - Hardware switched, A - Assert winner
 Timers: Uptime/Expires
 Interface state: Interface, Next-Hop or VCD, State/Mode

(*, 239.1.2.3), 00:00:11/00:03:18, RP 10.0.0.1, flags: S
  Incoming interface: GigabitEthernet1/0, RPF nbr 10.0.13.2
  Outgoing interface list:
    GigabitEthernet3/0, Forward/Sparse, 00:00:11/00:03:18

(10.1.3.102, 239.1.2.3), 00:00:11/00:02:48, flags: PR
  Incoming interface: GigabitEthernet1/0, RPF nbr 10.0.13.2
  Outgoing interface list: Null

(10.1.3.101, 239.1.2.3), 00:00:11/00:02:48, flags: PR
  Incoming interface: GigabitEthernet1/0, RPF nbr 10.0.13.2
  Outgoing interface list: Null

(10.1.3.100, 239.1.2.3), 00:00:11/00:02:48, flags: PR
  Incoming interface: GigabitEthernet1/0, RPF nbr 10.0.13.2
  Outgoing interface list: Null

(10.1.2.101, 239.1.2.3), 00:00:11/00:02:48, flags: T
  Incoming interface: GigabitEthernet2/0, RPF nbr 10.0.12.2
  Outgoing interface list:
    GigabitEthernet3/0, Forward/Sparse, 00:00:11/00:03:18

(10.1.2.100, 239.1.2.3), 00:00:11/00:02:48, flags: T
  Incoming interface: GigabitEthernet2/0, RPF nbr 10.0.12.2
  Outgoing interface list:
    GigabitEthernet3/0, Forward/Sparse, 00:00:11/00:03:18
```

### mclst

**mclst** is a multicast listener/sender tool. It's documented in
[mclst](docs/man-source/mclst.rst).

Some examples of running **mclst**.

Basic receiver:

```text
$ mclst -i enp0s5 239.1.2.3:12345
12:04:49.67724  timeout
12:04:54.67724  timeout
12:04:54.67724  enp0s5 (#2), 10.211.55.5:45532->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #0, 2023-02-24 12:04:54.677240294, delta 101201ns, neptune.lan
12:04:55.67724  enp0s5 (#2), 10.211.55.5:45532->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #1, 2023-02-24 12:04:55.677240295, delta 190223ns, neptune.lan
12:04:56.67724  enp0s5 (#2), 10.211.55.5:45532->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #2, 2023-02-24 12:04:56.677240296, delta 204525ns, neptune.lan
12:04:57.67724  enp0s5 (#2), 10.211.55.5:45532->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #3, 2023-02-24 12:04:57.677240297, delta 182082ns, neptune.lan
12:04:58.67724  enp0s5 (#2), 10.211.55.5:45532->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #4, 2023-02-24 12:04:58.677240298, delta 130292ns, neptune.lan
^C

Traffic received for 239.1.2.3:12345 in 17.404553 sec

Source            DPort Pkts Bytes   APS      Rate
================= ===== ==== ===== ===== =========
10.211.55.5:45532 12345    5   405 81.00 186.16bps
```

Receiver in *portless* mode with the UDP payload shown in the Hex/ASCII view:

```text
$ sudo mclst -i enp0s5 -X 239.1.2.3
14:47:11.67725  enp0s5 (#2), 10.211.55.5:59199->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #10, 2023-02-24 14:47:11.677250031, delta 152354ns, neptune.lan
  a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 0a  .!..Irc.........
  17 46 ca 04 79 51 c5 44  00 0b 6e 65 70 74 75 6e  .F..yQ.D..neptun
  65 2e 6c 61 6e                                    e.lan
14:47:12.67725  enp0s5 (#2), 10.211.55.5:58735->239.1.2.3:54321, TTL 255, UDP size 128
  b1 8b 6a 9e 78 97 c9 b6  c5 48 cd 52 c9 de e7 6a  ..j.x....H.R...j
  f6 b9 87 b7 9a c7 c5 0d  41 f2 bd 86 f5 6d 42 4f  ........A....mBO
  05 9a 27 8e 07 bb bd de  2d 39 f9 c2 4e 7c e0 15  ..'.....-9..N|..
  75 7d ac ac 32 12 77 7e  3c c1 f7 3c 40 f3 c8 35  u}..2.w~<..<@..5
  0a a8 81 ce 8f 0a 3f cc  4d c3 05 71 b3 da 45 12  ......?.M..q..E.
  75 95 a6 2b c0 6b bb 83  c6 1d 92 26 54 3b 8a 14  u..+.k.....&T;..
  4f c6 c8 08 a1 29 fb 1f  b3 e6 27 1b 60 af a8 06  O....)....'.`...
  3f 76 da ed e8 88 49 4c  15 82 12 1c 78 1a 08 8f  ?v....IL....x...
14:47:12.67725  enp0s5 (#2), 10.211.55.5:59199->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #11, 2023-02-24 14:47:12.677250032, delta 219435ns, neptune.lan
  a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 0b  .!..Irc.........
  17 46 ca 04 b4 fd 94 6c  00 0b 6e 65 70 74 75 6e  .F.....l..neptun
  65 2e 6c 61 6e                                    e.lan
14:47:13.67725  enp0s5 (#2), 10.211.55.5:58735->239.1.2.3:54321, TTL 255, UDP size 128
  b1 8b 6a 9e 78 97 c9 b6  c5 48 cd 52 c9 de e7 6a  ..j.x....H.R...j
  f6 b9 87 b7 9a c7 c5 0d  41 f2 bd 86 f5 6d 42 4f  ........A....mBO
  05 9a 27 8e 07 bb bd de  2d 39 f9 c2 4e 7c e0 15  ..'.....-9..N|..
  75 7d ac ac 32 12 77 7e  3c c1 f7 3c 40 f3 c8 35  u}..2.w~<..<@..5
  0a a8 81 ce 8f 0a 3f cc  4d c3 05 71 b3 da 45 12  ......?.M..q..E.
  75 95 a6 2b c0 6b bb 83  c6 1d 92 26 54 3b 8a 14  u..+.k.....&T;..
  4f c6 c8 08 a1 29 fb 1f  b3 e6 27 1b 60 af a8 06  O....)....'.`...
  3f 76 da ed e8 88 49 4c  15 82 12 1c 78 1a 08 8f  ?v....IL....x...
14:47:13.67725  enp0s5 (#2), 10.211.55.5:59199->239.1.2.3:12345, TTL 255, UDP size 37
                mclst pkt #12, 2023-02-24 14:47:13.677250033, delta 233443ns, neptune.lan
  a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 0c  .!..Irc.........
  17 46 ca 04 f0 a1 fb c4  00 0b 6e 65 70 74 75 6e  .F........neptun
  65 2e 6c 61 6e                                    e.lan
14:47:14.67725  enp0s5 (#2), 10.211.55.5:58735->239.1.2.3:54321, TTL 255, UDP size 128
  b1 8b 6a 9e 78 97 c9 b6  c5 48 cd 52 c9 de e7 6a  ..j.x....H.R...j
  f6 b9 87 b7 9a c7 c5 0d  41 f2 bd 86 f5 6d 42 4f  ........A....mBO
  05 9a 27 8e 07 bb bd de  2d 39 f9 c2 4e 7c e0 15  ..'.....-9..N|..
  75 7d ac ac 32 12 77 7e  3c c1 f7 3c 40 f3 c8 35  u}..2.w~<..<@..5
  0a a8 81 ce 8f 0a 3f cc  4d c3 05 71 b3 da 45 12  ......?.M..q..E.
  75 95 a6 2b c0 6b bb 83  c6 1d 92 26 54 3b 8a 14  u..+.k.....&T;..
  4f c6 c8 08 a1 29 fb 1f  b3 e6 27 1b 60 af a8 06  O....)....'.`...
  3f 76 da ed e8 88 49 4c  15 82 12 1c 78 1a 08 8f  ?v....IL....x...
^C

Traffic received for 239.1.2.3:* in 2.55259 sec

Source            DPort Pkts Bytes    APS      Rate
================= ===== ==== ===== ====== =========
10.211.55.5:59199 12345    3   243  81.00 761.58bps
10.211.55.5:58735 54321    3   516 172.00  1.62Kbps
```

Sender:

```text
$ mclst -i enp0s5 239.1.2.3:12345 -s
12:04:54.67724  sent packet to 239.1.2.3:12345, seq #0
12:04:55.67724  sent packet to 239.1.2.3:12345, seq #1
12:04:56.67724  sent packet to 239.1.2.3:12345, seq #2
12:04:57.67724  sent packet to 239.1.2.3:12345, seq #3
12:04:58.67724  sent packet to 239.1.2.3:12345, seq #4
^C
Sent 6 packets
```

## PIMC Library

A relatively general-purpose C++ library, with some network centric functionality.

The library as well as the apps are written in C++20. 

## Building the library and the apps

The library as well as the apps are written in C++20.

The library and the apps can currently be built in Linux and macOS.

> :warning: The library relies on C++20 ranges, which are not available in clang that
> ships with XCode. To compile in macOS please install gcc (e.g. `brew install gcc`)

### Prerequisites

The following are required to build PIMC toolkit:

 * C++20 compliant compiler and C++20 library which supports ranges. (e.g., g++ 11.2)
 * cmake version 3.20 or later
 * For the documentation the following is required:
   * doxygen version 1.9.3 or later
   * Python 3.6 or later and the following packages
     * Sphinx version 5
     * breathe version 4.34

The other two prerequisites are [Google test]( https://github.com/google/googletest)
and [C++ fmt]( https://github.com/fmtlib/fmt) library. Both will be installed by cmake
from GitHub, so nothing special needs to be done for them.

#### Installing Python prerequisites in a Virtual Environment and building documentation

This is probably the most annoying part of the building process. Basically, unless the
documentation prerequisites are satisfied cmake won't succeed. Below is an example of
how to install python prerequisites in a virtual environment followed by how to build
**mclst** and the documentation. This assumes that the required C++ compiler is installed,
that cmake is installed and at least version 3.20 and that doxygen is installed and at
least version 1.9.3. 

The below commands were executed in Centos 8 with the preinstalled version of python.
These commands should be invoked in a directory where the PIMC Toolkit is being built,
which may not be the directory to which the PIMC Toolkit was cloned. We assume that
the directory where the PIMC Toolkit was cloned is in the environment variable PIMC.

<pre>
$ python3 --version
Python 3.6.8
$ python3 -m venv --prompt pimc-toolkit .venv
$ activate .venv/bin/activate
$ pip install --upgrade pip
 ...
 <em>[output skipped]</em>
 ...
$ pip install Sphinx sphinx-rtd-theme breathe
 ...
 <em>[output skipped]</em>
 ...
</pre>

At this point it should be possible to run cmake. Unless the required C++ compiler is the
only one installed on the system, it is advisable to create a cmake toolchain file with
locations of the C++ and C compilers. The example below shows how to run cmake with the
supplied [g++ 11.2 toolchain file]( cmake/linux-gcc11.2-toolchain.cmake).

<pre>
$ cmake \
     -DCMAKE_TOOLCHAIN_FILE=${PIMC}/cmake/linux-gcc11.2-toolchain.cmake \
     ${PIMC}
 ...
 <em>[cmake output skipped]</em>
 ...
</pre>

Just running `make pimc` and `make mclst` should build **pimc** and **mclst** respectively.

<pre>
$ make mclst
 ...
 <em>[make output skipped]</em>
 ...
</pre>

The binary will be located in the subdirectory `bin`.

To build the man pages `make man-pages` should be issued. The resulting man pages
are placed in `docs/man-pages`.
