====================================
 Structured Packet Viewer Utilities
====================================

Include file ``pimc/packets/PacketView.hpp``
============================================

This file contains two classes :cpp:class:`pimc::PacketView` and :cpp:class:`pimc::ReversePacketView`
which provide means to walk the packet data in the forward and reverse order.

.. doxygenclass:: pimc::PacketView
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::ReversePacketView
   :project: PimcLib
   :members:

Include file ``pimc/packets/PacketWriter.hpp``
==============================================

This include file contains the class :cpp:class:`pimc::PacketWriter` which facilitates
writing structured data into packet memory.

.. doxygenclass:: pimc::PacketWriter
   :project: PimcLib
   :members:
