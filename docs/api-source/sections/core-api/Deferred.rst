=======================================
Include file ``pimc/core/Deferred.hpp``
=======================================

This file contains a class template :cpp:class:`pimc::Deferred` which ensures
that a desired action is always executed on exist from a function regardless of
branching. This file also contains a convenience utility function :cpp:func:`pimc::defer`
which creates the ``Deferred`` object with the specified computation.

Example

.. code-block:: c++

   #include "pimc/core/Deferred.hpp"

   bool f(int a, int b) {
       // Assuming that the resource must be released once used
       // by calling releaseResource(resource);
       auto resource = getResource();
       auto d = pimc::defer([resource&] { releaseResource(resource); });

       if (a > 1000)
           throw(1);

       if (a < 10) {
           if (b > 100)
	       throw(2);
       }

       return true;
   }

.. doxygenclass:: pimc::Deferred
   :project: PimcLib
   :members:

.. doxygenfunction:: pimc::defer
   :project: PimcLib
