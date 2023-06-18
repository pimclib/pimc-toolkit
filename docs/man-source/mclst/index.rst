============================================
 mclst - multicast listener and sender tool
============================================


SYNOPSIS
========

mclst -i intf [receiver options] group[:port]
mclst -i intf -s [sender options] group:port

DESCRIPTION
===========

The mclst utility is a network centric multicast listener and sender tool.
It has two distinct modes of operation:

  #. The receiver mode allows subscribing to multicast and showing the
     received traffic
  #. The sender mode allows Sending test multicast traffic to a desired
     IPv4 multicast group and UDP port

     
Subscribing to Multicast
------------------------

The mclst utility supports two ways to subscribe to multicast. The normal way
requires the user to specify the desired group and UDP port. The alternative
way lets the user to specify only the group, in which case mclst will show
multicast traffic destined for the group and any UDP ports. The latter is
referred to as the *portless* mode.

.. note::
   The *portless* mode exploits the fact that the network routers and switches
   do not use UDP ports for forwarding multicast traffic to the subscribers.
   This means that if some sources send traffic to the same group but to
   different destination UDP ports, all packets destined for this group will
   be received by any host subscribing to the group, regardless of the
   destination port. The sockets API on the receiving host will filter the
   received multicast traffic and it will deliver only the traffic that matches
   the UDP port to which the subscription was made.

   To overcome the automatic filtering mclst uses a raw IP UDP socket which
   allows it to receive all UDP packets that the host receives (not only
   the multicast packets). By default, programs cannot create raw sockets.

.. warning::
   The *portless* mode does not work in MacOS.

When receiving traffic mclst will show the following information:

  * The local host timestamp of when the packet was received. This is as accurate
    as the ``select()``/sockets API allow.
  * The actual interface on which the packet was received. In the case of multipe
    subscriptions to the same multicast group on the same host on different
    interfaces, regarless of where the packet is received, all processes which
    subscribed to the group will receive the packet. This may cause confusion
    as to where the traffic is arriving. It can also cause duplicate packets to
    be received by the subscribing processes if the same multicast group is
    available on multiple host's interfaces. The ability of mclst to show the
    interface may come very handy in troubleshooting of such situations.
  * The source IP address and source UDP port as well as the destination multicast
    group and destination UDP port.
  * The TTL of the packet as recveived by the host.

In addition, if a multicast packet was sent by mclst running in the sender mode,
the receiving mclst process will detect it by looking at the UDP payload and it
will show the remote host's sequence number, time and hostname.

The mclst utility` also supports source specific multicast subscriptions. The
caveat, however, is that it requires IGMPv3. This may or may not be enabled on
the host, and mclst doesn't have any control over it. If the IGMPv3 is enabled,
mclst process will send an IGMPv3 source specific join. Otherwise, it will fall
back to the usual any-source IGMPv2 join and the kernel will filter the traffic
by the requested source before delivering the packets to mclst.

By default mclst receives packets indefinitely. To exit mclst the user has to
interrupt mclst, for example by pressing Ctrl-C. Alternatively there is an option
to force mclst exit automatically after receiving a desired number of packets.
Once mclst exits it shows a summary of the statistics of the received multicast
traffic per each source/source UDP port/destination UDP port combination.
      
Sending multicast
-----------------

The mclst utility sends multicast packets to the specified destination multicast
group and UDP port at the rate of one packet per second.

There is a sender specific option to set the TTL of the generated traffic. By
default, however, it sends the traffic with the TTL of 255.

Similarly to the receiver mode, the mclst utility in the sender mode send
packets indefinitely. The same option can be used to force mclst terminate
after sending the requested number of packets.

Command Line Options
====================

General Options
---------------

.. option:: -i <interface>, --interface <interface>

	    This option specifies the interface on which to subscribe to multicast
	    traffic, or from which to send traffic. This option is mandatory.

.. option:: -c <number-of-packets>, --count <number-of-packets>

	    This option causes :program:`mclst` in the receiver mode to exit after
	    receiving the specified number of packets. Likewise, in the sender mode
	    :program:`mclst` will send the specified number of packets and then exit.

.. option:: --no-colors

	    By default :program:`mclst` uses ANSI terminal colors to show the
	    received traffic. This flag allows turning off the colored output. If,
	    however, the standard output or stanndard error are redirected to a
	    file, :program:`mclst` will not use colors.

.. option:: --show-config

	    This flag causes :program:`mclst` to check the command line parameters,
	    display its iterpretation of them and exit.


Receiver Mode Options
---------------------
	    
.. option:: -S <IP-address>, --source <IP-address>

	    With this option :program:`mclst` will attempt to perform a source
	    specific join using IGMPv3, where the source is the IP address specified
	    with this option.

.. option:: -t <seconds>, --timeout <seconds>

	    This option allows specifyin the timeout which will be reported by
	    :program:`mclst` if no traffic is received after the specified number
	    of seconds elapses.

.. option:: -X, --hex-ascii

	    This flag causes :program:`mclst` to show the UDP payload using the
	    split Hex/ASCII output similar to ``tcpdimp -XX``.

Sender Mode Options
-------------------
	    
.. option:: -s, --sender

	    This flag should be used to run :program:`mclst` in the sender mode. The
	    sender may not be used in the *portless* mode.

.. option:: --ttl <TTL>

	    This option allows specifying a desired TTL for the mclst beacon traffic.
	    If omitted the TTL is 255. This option accepts values in  range 1-255.
	    This option is only available when running :program:`mclst` in the sender
	    mode.

                
Examples
========

Basic Receiver
--------------

.. code-block:: text

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
   		

Receiver which shows the UDP payload in Hex/ASCII
-------------------------------------------------

.. code-block:: text
   
   $ mclst -i enp0s5 -X 239.1.2.3:12345
   12:06:33.67724  enp0s5 (#2), 10.211.55.5:34511->239.1.2.3:12345, TTL 255, UDP size 37
                   mclst pkt #4, 2023-02-24 12:06:33.677240393, delta 103166ns, neptune.lan
     a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 04  .!..Irc.........
     17 46 c1 40 59 f7 20 30  00 0b 6e 65 70 74 75 6e  .F.@Y. 0..neptun
     65 2e 6c 61 6e                                    e.lan
   12:06:34.67724  enp0s5 (#2), 10.211.55.5:34511->239.1.2.3:12345, TTL 255, UDP size 37
                   mclst pkt #5, 2023-02-24 12:06:34.677240394, delta 177855ns, neptune.lan
     a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 05  .!..Irc.........
     17 46 c1 40 95 9a 24 cd  00 0b 6e 65 70 74 75 6e  .F.@..$...neptun
     65 2e 6c 61 6e                                    e.lan
   12:06:35.67724  enp0s5 (#2), 10.211.55.5:34511->239.1.2.3:12345, TTL 255, UDP size 37
                   mclst pkt #6, 2023-02-24 12:06:35.677240395, delta 135213ns, neptune.lan
     a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 06  .!..Irc.........
     17 46 c1 40 d1 48 ae 17  00 0b 6e 65 70 74 75 6e  .F.@.H....neptun
     65 2e 6c 61 6e                                    e.lan
   12:06:36.67724  enp0s5 (#2), 10.211.55.5:34511->239.1.2.3:12345, TTL 255, UDP size 37
                   mclst pkt #7, 2023-02-24 12:06:36.677240396, delta 125241ns, neptune.lan
     a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 07  .!..Irc.........
     17 46 c1 41 0c fd 11 ab  00 0b 6e 65 70 74 75 6e  .F.A......neptun
     65 2e 6c 61 6e                                    e.lan
   12:06:37.67724  enp0s5 (#2), 10.211.55.5:34511->239.1.2.3:12345, TTL 255, UDP size 37
                   mclst pkt #8, 2023-02-24 12:06:37.677240397, delta 121449ns, neptune.lan
     a5 21 d9 a2 49 72 63 90  00 00 00 00 00 00 00 08  .!..Irc.........
     17 46 c1 41 48 9d 25 2f  00 0b 6e 65 70 74 75 6e  .F.AH.%/..neptun
     65 2e 6c 61 6e                                    e.lan
   ^C
   
   Traffic received for 239.1.2.3:12345 in 5.127033 sec
   
   Source            DPort Pkts Bytes   APS      Rate
   ================= ===== ==== ===== ===== =========
   10.211.55.5:34511 12345    5   405 81.00 631.94bps

Receiver in the portless mode
-----------------------------

.. code-block:: text

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
   
Sender
------

.. code-block:: text
   
   $ mclst -i enp0s5 239.1.2.3:12345 -s
   12:04:54.67724  sent packet to 239.1.2.3:12345, seq #0
   12:04:55.67724  sent packet to 239.1.2.3:12345, seq #1
   12:04:56.67724  sent packet to 239.1.2.3:12345, seq #2
   12:04:57.67724  sent packet to 239.1.2.3:12345, seq #3
   12:04:58.67724  sent packet to 239.1.2.3:12345, seq #4
   ^C
   Sent 6 packets
   
Viewing Configuration
---------------------

.. code-block:: bash

   $ ./mclst -s -i wlan0 239.1.1.1:2222 --show-config
   Send to 239.1.1.1:2222, 1pps, TTL 255
   Interface: wlan0 (192.168.0.51)
   Colors: YES
   
   Host IPv4 interfaces:
   
     Index Interface       Address
     ===== =============== ============
     1     lo              127.0.0.1
     3     wlan0           192.168.0.51
     4     docker0         172.17.0.1
     56    br-4064c9b52f9f 172.18.0.1
     107   br-62447bbeaa67 172.19.0.1
