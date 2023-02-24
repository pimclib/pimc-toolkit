===============================================
Multicast Listener/Sender Tool :program:`mclst`
===============================================

Overview
--------

:program:`mclst` is a network scentric multicast listener and sender tool.
It has two distinct modes of operation as follows:

  #. The receiver mode allows subscribing to multicast and showing the
     received traffic
  #. The sender mode allows Sending test multicast traffic to a desired
     IPv4 multicast group and UDP port

.. _overview-subscribing:
     
Subscribing to Multicast
^^^^^^^^^^^^^^^^^^^^^^^^

.. _receiver-modes:

:program:`mclst` supports two ways to subscribe to multicast. The normal way
requires the user to specify the desired group and UDP port. The alternative
way allows the user to specify only the group, in which case :program:`mclst`
will show multicast traffic destined for the group and all UDP ports. The latter
*portless* mode, however, requires privileged execution of :program:`mclst`.
The easiest way to do so is to run :program:`mclst` under :program:`sudo`.
Alternatively :program:`mclst` binary may be granted ``CAP_NET_RAW`` capability
(see `capabilities`_), which would allow any user to run :program:`mclst` in
the portless mode without :program:`sudo`.

.. warning::
   The *portless* mode does not work in MacOS.

When receiving traffic :program:`mclst` will show the following information:

  * The local host timestamp of when the packet was received. This is as accurate
    as the `select()`_ / sockets API allow.
  * The actual interface on which the packet was received. In the case of multipe
    subscriptions to the same multicast group on the same host on different
    interfaces, regarless of where the packet is received, all processes which
    subscribed to the group will receive the packet. This may cause confusion
    as to where the traffic is arriving. It can also cause duplicate packets to
    be received by the subscribing processes if the same multicast group is
    available on multiple host's interfaces. The ability of :program:`mclst`
    to show the interface may come very handy in troubleshooting of such situations.
  * The source IP address and source UDP port as well as the destination multicast
    group and destination UDP port.
  * The TTL of the packet as recveived by the host.

If addition, if a multicast packet was sent by :program:`mclst` running in the
sender mode, the receiving :program:`mclst` process will detect it by looking at
the UDP payload and it will show the remote host's sequence number, remote host's
time and remote host's name. The packets sent by :program:`mclst` are called
*mclst beacon* packets.

There is also an option to show the UDP payload using the splict Hex/ASCII mode
similar to how :program:`tcpdump` shows the contents of the packets when used
with the ``-XX`` option.

.. _source-specific-joins:

:program:`mclst` also supports source specific multicast subscriptions. The caveat,
however, is that it requires IGMPv3. This may or may not be enabled on the host,
and :program:`mclst` doesn't have any control over it. If the IGMPv3 is enabled,
:program:`mclst` process will send an IGMPv3 source specific join request. Otherwise,
it will fall back to the usual any-source IGMPv2 join request and the host will
filter the traffic by the requested source before delivering the packets to
:program:`mclst`.

By default :program:`mclst` receives packets indefinitely. To exit :program:`mclst`
the user has to press :kbd:`Ctrl-C`. Alternatively there is an option to force
:program:`mclst` exit automatically after receiving a desired number of packets.
Once :program:`mclst` exits it shows a summary of the statistics of the received
multicast traffic per each source/source UDP port/destination UDP port combination.

Sending multicast
^^^^^^^^^^^^^^^^^

:program:`mclst` supports a simple multicast publisher, which sends a *mclst beacon*
packet every second. As mentioned in :ref:`overview-subscribing` when the receiving
:program:`mclst` processes receive the beacon traffic, they show the information that
was put in it by the sending :program:`mclst` process.

:program:`mclst` provides a way to specify the TTL of the generated traffic. By
default, however, it sends the traffic with the TTL of 255.

Running :program:`mclst`
------------------------

Receiver Mode
^^^^^^^^^^^^^

To run :program:`mclst` in the receiver mode the minimal command line form is as
follows:

.. code-block:: text

   mclst -i <interface> <target>

The :code:`-i <interface>` option specifies the host's interface where a multicast
IGMP join will be ussued.

The :code:`<target>` specifies the multicast group to which :program:`mclst` should
subsribe.

In the normal receiver mode, the :code:`<target>` is a multicast group followed by
:code:`:` followed by a UDP port number, e.g. 239.1.2.3:12345.

For example:

.. code-block:: text
		
   mclst -i enp0s5 239.1.2.3:12345

In the portless receiver mode, the :code:`<target>` is just the multicast group.
(See `receiver modes <receiver-modes>`_).

For example:

.. code-block:: text
		
   sudo mclst -i enp0s5 239.1.2.3

Sender Mode
^^^^^^^^^^^

To run :program:`mclst` in the sender mode the minimal command line form is as
follows:

.. code-block:: text

   mclst -i <interface> -s <group>:<UDP-port>

For example:

.. code-block:: text

   mclst -i enp0s5 -s 239.1.2.3:12345


Command Line Options
^^^^^^^^^^^^^^^^^^^^

.. option:: -i <interface>, --interface <interface>

	    This option specifies the interface on which to subscribe to multicast
	    traffic, or from which to send traffic. This option is mandatory.

.. option:: -S <IP-address>, --source <IP-address>

	    With this option :program:`mclst` will attempt to perform a source
	    specific join using IGMPv3, where the source is the IP address specified
	    with this option. (See the caveats in
	    :ref:`source specific joins <source-specific-joins>`)

.. option:: -t <seconds>, --timeout <seconds>

	    This option allows specifyin the timeout which will be reported by
	    :program:`mclst` if no traffic is received after the specified number
	    of seconds elapses.

.. option:: -X, --hex-ascii

	    This flag causes :program:`mclst` to show the UDP payload using the
	    split Hex/ASCII output similar to ``tcpdimp -XX``.

.. option:: -s, --sender

	    This flag should be used to run :program:`mclst` in the sender mode. The
	    sender may not be used in the *portless* mode.

.. option:: --ttl <TTL>

	    This option allows specifying a desired TTL for the mclst beacon traffic.
	    If omitted the TTL is 255. This option accepts values in  range 1-255.
	    This option is only available when running :program:`mclst` in the sender
	    mode.

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

Examples
--------

Basic Receiver
^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
^^^^^^

.. code-block:: text
   
   $ mclst -i enp0s5 239.1.2.3:12345 -s
   12:04:54.67724  sent packet to 239.1.2.3:12345, seq #0
   12:04:55.67724  sent packet to 239.1.2.3:12345, seq #1
   12:04:56.67724  sent packet to 239.1.2.3:12345, seq #2
   12:04:57.67724  sent packet to 239.1.2.3:12345, seq #3
   12:04:58.67724  sent packet to 239.1.2.3:12345, seq #4
   ^C
   Sent 6 packets
   

.. _capabilities: https://man7.org/linux/man-pages/man7/capabilities.7.html
.. _select(): https://man7.org/linux/man-pages/man2/select.2.html
