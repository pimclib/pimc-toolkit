=================================================================
 PIM Sparse Mode Version 2 Data Structures and Utility Functions
=================================================================

These include files contain the PIM Sparse Mode Version 2 (:rfc:`7761`)
data structures and utility functions which help read and write PIM
data.

Include file ``pimc/packets/PIMSMv2.hpp``
=========================================

.. doxygenstruct:: pimc::PIMSMv2Hdr
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::PIMSMv2EncUAddr
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::PIMSMv2EncGAddr
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::PIMSMv2EncSrcAddr
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::PIMSMv2HelloOption
   :project: PimcLib
   :members:

Include file ``pimc/packets/PIMSMv2Utils.hpp``
==============================================

The following function writes a
`PIM SM v2 header <https://datatracker.ietf.org/doc/html/rfc7761.html#section-4.9>`_
with the specified type.


.. doxygenfunction:: pimc::pimsmv2::writeHdr
   :project: PimcLib

The following four function encode
`unicast, group, RP and source addresses <https://datatracker.ietf.org/doc/html/rfc7761.html#section-4.9.1>`_
in the PIM SM v2 specific format.
	     
.. doxygenfunction:: pimc::pimsmv2::writeIPv4Addr
   :project: PimcLib

.. doxygenfunction:: pimc::pimsmv2::writeIPv4Grp
   :project: PimcLib

.. doxygenfunction:: pimc::pimsmv2::writeIPv4RP
   :project: PimcLib

.. doxygenfunction:: pimc::pimsmv2::writeIPv4Src
   :project: PimcLib

The following three functions encode
`PIM SM v2 Hello packet options <https://datatracker.ietf.org/doc/html/rfc7761.html#section-4.9.2>`_.
	     
.. doxygenfunction:: pimc::pimsmv2::writeOptHoldtime
   :project: PimcLib

.. doxygenfunction:: pimc::pimsmv2::writeOptDrPriority
   :project: PimcLib

.. doxygenfunction:: pimc::pimsmv2::writeOptGenerationId
   :project: PimcLib

