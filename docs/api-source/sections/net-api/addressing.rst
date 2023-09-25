===================================
 IPv4 Address and Prefix Utilities
===================================

Include file ``pimc/net/IPv4Address.hpp``
=========================================

This include file contains class :cpp:class:`pimc::net::IPv4Address`, which is a wrapper over
an unsigned 32-bit integer and which provides utility method for IPv4 address related manipulation.

.. doxygenclass:: pimc::net::IPv4Address
   :project: PimcLib
   :members:

Include file ``pimc/net/IPv4Prefix.hpp``
========================================

This include file contains class :cpp:class:`pimc::net::IPv4Prefix`, which is a wrapper over
:cpp:class:`pimc::net::IPv4Address` and the associated prefix length. This class provides
utility methods for IPv4 prefix related manipulation.

.. doxygenclass:: pimc::net::IPv4Prefix
   :project: PimcLib
   :members:
