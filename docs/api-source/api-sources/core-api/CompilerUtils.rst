============================================
Include file ``pimc/core/CompilerUtils.hpp``
============================================

A number of macros affecting how the compiler handles the code.

.. doxygendefine:: PIMC_LIKELY
   :project: PimcLib

for example:

.. code-block:: c++

  #include "pimc/core/CompilerUtils.hpp"

  int f(int v) {
      if (PIMC_LIKELY(v > 0)) {
	     // The compiler will arrange the code in this branch
	     // to follow the test for v > 0
	 } else {
	     // The code in this branch will be jumped to
	 }
  }

.. doxygendefine:: PIMC_UNLIKELY
   :project: PimcLib

The effect of this macro is exactly the opposite of :c:macro:`PIMC_LIKELY`.

.. doxygendefine:: PIMC_NO_INLINE
   :project: PimcLib

.. doxygendefine:: PIMC_HOT_PATH
   :project: PimcLib

.. doxygendefine:: PIMC_COLD_PATH
   :project: PimcLib

.. doxygendefine:: PIMC_ALWAYS_INLINE
   :project: PimcLib
