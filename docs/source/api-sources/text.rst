================
Text Related API
================

Include file ``pimc/text/CString.hpp``
--------------------------------------

This include file contains :cpp:struct:`pimc::cssentinel`, which is a sentinel for
a ``char const*`` pointer to a zero-terminated string.

Example:

.. code-block:: c++

   #include <concepts>
   #include <iterator>

   #include "pimc/text/CString.hpp"

   template <std::input_iterator I, std::sentinel_for<I> S>
   int countChars(I first, S last) {
       int count{0};
       for (I ii{first}; ii != last; ++ii) ++count;
       return count;
   }

   int countChars(char const* s) {
       return countChars(s, cssentinel{});
   }

.. doxygenstruct:: pimc::cssentinel
   :project: PimcLib

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
