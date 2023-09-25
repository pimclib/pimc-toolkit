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

Include file ``pimc/text/HexAsciiBlock.hpp``
--------------------------------------------

This include file contains :cpp:func:`pimc::formatHexAscii`, which formats binary
data into a hext view on the left and printable characters view on the right, similar
to how ``tcpdump`` formats packets contents.

Example:

.. code-block:: c++

   #include <cstdint>
   
   #include <fmt/format.h>

   void f(uint8_t const* data, size_t sz) {
       fmt::memory_buffer buf;
       auto bi = std::back_inserter(buf);
       fmt::format_to(bi, "Hex/ASCII view:\n\n);
       pimc::formatHexAscii(bi, data, sz);
       fmt::print("{}", fmt::to_string(buf));
   }

The above example will produce the following possible input:

.. code-block:: text

   Hex/ASCII view:
   
     85 cb 8e 6b 8f 3d 02 2e  2e cb 4b fd 95 55 91 11  ...k.=....K..U..
     43 3e 26 df 07 5e ce 21  0c 69 75 f3 19 5b ad 86  C>&..^.!.iu..[..
     a6                                                .

.. doxygenfunction:: formatHexAscii
   :project: PimcLib
