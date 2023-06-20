================
Time Related API
================

Include file ``pimc/time/TimeUtils.hpp``
----------------------------------------

This file contains a single function ``gethostnanos()`` which returns the current host time in
nanoseconds since the epoch, i.e. 1970-01-01 00:00.0 UTC, as ``uint64_t``.

Example:

.. code-block:: c++

   #include <fmt/format.h>

   #include "pimc/time/TimeUtils.hpp"

   void f() {
       fmt::format("Current time in nanoseconds: {}\n", gethostnanos());
   }

		
