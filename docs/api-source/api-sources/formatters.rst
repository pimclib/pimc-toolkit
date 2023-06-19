======================
Formatting Related API
======================

Include file ``pimc/formatters/IPv4Formatters.hpp``
---------------------------------------------------

This include file contains the declarations which allow formatting
:cpp:class:`pimc::net::IPv4Address` and :cpp:class:`pimc::net::IPv4Prefix` using
the ``fmt`` library.

Example:

.. code-block:: c++

   #include <fmt/format.h>
   
   #include "pimc/net/IPv4Address.hpp"
   #include "pimc/net/IPv4Prefix.hpp"
   #include "pimc/formatters/IPv4Formatters.hpp"

   void f(pimc::net::IPv4Address a, pimc::net::IPv4Prefix p) {
       fmt::print("Address: {}, Prefix: {}\n", a, p);
   }

The above example will produce the following possible output:

.. code-block:: text

   Address: 10.1.2.3, Prefix: 10.10.0.0/16
