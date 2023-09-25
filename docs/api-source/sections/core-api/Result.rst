=====================================
Include file ``pimc/core/Result.hpp``
=====================================

The file ``Result.hpp`` contains two class templates :cpp:class:`pimc::Failure`
and :cpp:class:`pimc::Result` and a number of utility function for manipulation
of these two objects.

The class template :cpp:class:`pimc::Result` provides a way to store either of
two values. An object :cpp:class:`pimc::Result` at any time either contains a
valid result of a computation of type ``T``, or an error object of type ``E``.

Both the stored result or the error are allocated directly within the storage of
the :cpp:class:`pimc::Result`, no dynamict allocation takes place.

Both the value result ``T`` or the error ``E`` may be values or lvalue references.
In addition ``T`` may also be ``void``.

The class template :cpp:class:`pimc::Failure` allows the computation to return
a Result in the failed state with the error value contained in the returned
Failure object.

The Result objects are comparable between each other, with the values whose type
is ``T`` and with the Failure object whose value type is comparable to the error
type of the Result object.

Example

.. code-block:: c++

   #include <string>
   #include <fmt/format.h>

   #include "pimc/core/Result.hpp"

   pimc::Result<unsigned, std::string> doubleOdd(unsigned x) {
       if (x & 1u)
           return x*2;

       return pimc::fail(fmt::format("expecting an odd value, got {}", x));
   }

   unsigned f(unsigned x) {
       auto r = doubleOdd(x);

       if (r) {
           fmt::print("Odd value {} multipled by 2 is {}\n", x, *r);
	   return *r;
       }

       fmt::print("error: {}\n", r.error());
       return x;
   }

   

Reference
=========

.. doxygenclass:: pimc::Result
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::Result< void, E >
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::Failure
   :project: PimcLib
   :members:

      
