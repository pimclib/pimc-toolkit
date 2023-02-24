==================
System Related API
==================

Include file ``pimc/system/Exceptions.hpp``
-------------------------------------------

This include file contains the function ``pimc::raise()``, which throws an
exception specified via the template parameter ``Ex`` and whose message is formatted
using the ``fmt`` style formatting mechanism.

Example:

.. code-block:: c++

   #include "pimc/system/Exceptions.hpp"

   unsigned odd(unsigned v) {
       if (v & 1) return v;

       raise<std::runtime_error>("Expecting odd unsigned value, not {}", v);
   }

Include file ``pimc/system/SysError.hpp``
-----------------------------------------

This include file contains the following function declaration

.. code-block:: c++

   std::string sysError(int syserr = errno);

The function ``pimc::sysError()`` can be used to create an error message which corresponds
to the system error code, typically communicated via the macro called ``errno``.
