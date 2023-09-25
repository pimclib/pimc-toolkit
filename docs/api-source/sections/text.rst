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

