==================================
 pimc - PIM sparse-mode v2 client
==================================

SYNOPSIS
========

pimc config.yml

DESCRIPTION
===========

The pimc program is a network application, which allows subscribing to multicast
using the PIM sparse-mode version 2 protocol.

The program requires a configuration file in the YAML format with the following
structure:

.. code-block:: yaml

---

   pim:
     neighbor: 172.16.0.1
     interface: eth0
   
   multicast:
     239.1.2.3:
       Join*:
         # RP may not appear is the sources listed in Join and Prune (rpt)
         # sections
         RP: 192.168.0.55
         # None of the sources listed in Prune rpt may appear
         # in Joins
         Prune:
           - 192.168.5.100
           - 192.168.5.101
           - 192.168.5.102
       # None of the joined sources may be equal to RP or appear in the
       # Prune (rpt) list
       Join:
         - 192.168.0.35
         - 192.168.0.36

   logging:
     level: debug
     dir: /path/to/log-dir
   
Section ``pim``
---------------

The section ``pim`` is mandatory and it contains two mandatory parameters --
``neighbor`` and ``interface``. The ``neighbor`` parameter is the IP address of the
PIM neighbor for which the join/prune updates are destined. The ``interface``
parameter indicates the local host's interface from which the PIM hello and update
messages are sent.

Section ``multicast``
---------------------

The section ``multicast`` is mandatory and it describes the PIM join and prune
state that the host will send to the PIM neighbor. Each entry in the ``multicast``
section is a multicast group address. Under the multicast group there may be two
possible entries: ``Join*`` and ``Join``.

The ``Join*`` entry describes the shared tree, which results in Join(*,G) and optionally
Prune(S,G,rpt) states sent to the PIM neighbor. The ``Join*`` entry requires the ``RP``
entry, which specifies the IP address of the RP for the group. Optionally, the entry
``Prune`` may be specified, which contains a list of the sources, which must be pruned
from the shared tree. If the ``Join*`` entry is accompanied only by the ``RP`` entry,
the result is that the host will send Join(*,G) state for the group to the neighbor.
In the presence of the optional ``Prune`` list of sources, the Join(*,G) will be
accompanied by the Prune(S,G,rpt) states for all of the sources in the ``Prune`` list.

.. note::
   The number of sources which can be specified in the ``Prune`` entry of the
   group's ``Join*`` entry is limited to 180, which is the maximum number of the
   RPT pruned sources, which can be packed into a single PIM join/prune update.
   PIM does not offer a mechanism to spread more than 180 RPT pruned sources into
   multiple join/prune updates.

The ``Join`` entry describes the source specific tree. If present, it contains a
list of the sources for which the host must send Join(S,G) states to the neighbor.
There is no limitation on how many sources can be specified in the ``Join`` list.

Depending on the size of the ``multicast`` configuration, pimc may need to transmit
the join/prune state to the neighbor in multiple messages.

Section ``logging``
-------------------

The section ``logging`` is optional. It can have two optional entries -- ``level``
and ``dir``. If level is specified, it can take logging level values, of which only
the debug is useful. If the level is set to debug, pimc will report every PIM message
that it sends. The entry ``dir`` is used to specify the directory in which pimc will
save the log file; if omitted, pimc will send the log messages to stdout.

Command Line Options
====================

pimc has the following command like options:

.. option:: --show-config

	    Check the command line parameters, show their interpretation and
	    a table of the IPv4 interfaces and exit.

.. option:: -h, --help, -v, --version

	    Display usage summary or pimc library version information.

Example
=======

.. code-block:: text

   $ cat km1-pimc.cfg
   ---
   
   logging:
     level: debug
   
   pim:
     neighbor: 172.16.0.1
     interface: eth0
   
   multicast:
     239.1.2.3:
       Join*:
         # RP may not appear is the sources listed in Join and Prune (rpt)
         # sections
         RP: 192.168.0.55
         # None of the sources listed in Prune rpt may appear
         # in Joins
         Prune:
           - 192.168.5.100
           - 192.168.5.101
           - 192.168.5.102
       # None of the joined sources may be equal to RP or appear in the
       # Prune (rpt) list
       Join:
         - 192.168.0.35
         - 192.168.0.36
   
   $ pimc km1-pimc.cfg
   17:35:23.695318 DEBUG: PIM SM config:
   PIM sparse-mode:
     neighbor: 172.16.0.1
     interface: eth0, #2, addr 172.16.0.51
     hello period: 30s
     hello hold time: 105s
     join/prune period: 60s
     join/prune hold time: 210s
     generation ID: 83a2ff79
   
   17:35:23.695318 DEBUG: Join/Prune Config:
    Join/Prune config:
     239.1.2.3
       Join(*,G): RP 192.168.0.55
       Prune(S,G,rpt):
         192.168.5.100
         192.168.5.101
         192.168.5.102
       Join(S,G):
         192.168.0.35
         192.168.0.36
   
   17:35:23.695318 DEBUG: Will be sending 1 update:
   Update #1 with 1 group:
   Group 239.1.2.3
    3 joins, 3 prunes
    Joins:
      192.168.0.35
      192.168.0.36
      192.168.0.55, WC, rpt
    Prunes:
      192.168.5.100, rpt
      192.168.5.101, rpt
      192.168.5.102, rpt
   
   17:35:23.695318 DEBUG: Once terminated will send 1 inverse update:
   Update #1 with 1 group:
   Group 239.1.2.3
    0 joins, 3 prunes
    Joins:
    Prunes:
      192.168.0.55, WC, rpt
      192.168.0.35
      192.168.0.36
   
   17:35:23.695318 DEBUG: created IPv4 PIM socket
   17:35:23.695318 DEBUG: bound the IPv4 PIM socket to device eth0 (#2)
   17:35:23.695318 DEBUG: sent IPv4 Hello [holdtime 105s, DR priority 0, generation ID 83a2ff79]
   17:35:53.695318 DEBUG: sent IPv4 Hello [holdtime 105s, DR priority 0, generation ID 83a2ff79]
   17:36:23.695318 DEBUG: sent IPv4 Hello [holdtime 105s, DR priority 0, generation ID 83a2ff79]
   17:36:23.695318 DEBUG: sent IPv4 Join/Prune Update packet #1 with 1 groups, neighbor 172.16.0.1, holdtime 210s
   Group 239.1.2.3
    3 joins, 3 prunes
    Joins:
      192.168.0.35
      192.168.0.36
      192.168.0.55, WC, rpt
    Prunes:
      192.168.5.100, rpt
      192.168.5.101, rpt
      192.168.5.102, rpt
   
   17:36:53.695318 DEBUG: sent IPv4 Hello [holdtime 105s, DR priority 0, generation ID 83a2ff79]
   ^C17:36:58.695318 DEBUG: sent IPv4 Join/Prune Update packet #1 with 1 groups, neighbor 172.16.0.1, holdtime 210s
   Group 239.1.2.3
    0 joins, 3 prunes
    Joins:
    Prunes:
      192.168.0.55, WC, rpt
      192.168.0.35
      192.168.0.36
   
   17:36:58.695318 DEBUG: sent IPv4 Goodbye [DR priority 0, generation ID 83a2ff79]

