===========
Parsers API
===========

Include file ``pimc/parsers/NumberParsers.hpp``
-----------------------------------------------

This include file contains a number of signed and unsigned decimal integer parsers.
For the unsigned integers there is a separate parser function for each of  ``uint64_t``,
``uint32_t``, ``uint16_t``, and ``uint8_t``. Likewise, for the signed integers there
is a separate parser function for each of ``int64_t``, ``int32_t``, ``int16_t``, and
``int8_t``. Each parser function has three overloads, the first one takes an iterator
and a sentinel, the second one takes a range (which allows parsing ``std::string``\ s and
``std::string_view``\ s amongts other range like objects), and the last one takes a
``char const*``. Each function returns a :cpp:class:`pimc::Result` whose value is the
parsed integer and whose error is the type parsing failure, either an overflow or
invalid input.

Example:

.. code-block:: c++

   #include <string_view>
   
   #include <fmt/format.h>

   #include "pimc/parsers/NumberParsers.hpp"

   void f(std::string_view s) {
       auto r = pimc::parseDecimalInt64(s);

       if (r)
           fmt::print("Parsed integer value: {}\n", *r);
       else {
           if (r.error() == pimc::NumberParseError::Overflow)
	       fmt::print("parsing {} results in overflow\n", s);
	   else
	       fmt::print("invalid input: '{}'\n", s);
       }
   }


.. doxygenenum:: pimc::NumberParseError
   :project: PimcLib


``uint64_t`` Parsers
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalUInt64(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt64(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt64(char const *s)
   :project: PimcLib

``uint32_t`` Parsers
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalUInt32(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt32(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt32(char const *s)
   :project: PimcLib

``uint16_t`` Parsers
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalUInt16(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt16(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt16(char const *s)
   :project: PimcLib

``uint8_t`` Parsers
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalUInt8(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt8(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalUInt8(char const *s)
   :project: PimcLib

``int64_t`` Parsers
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalInt64(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt64(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt64(char const *s)
   :project: PimcLib

``int32_t`` Parsers
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalInt32(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt32(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt32(char const *s)
   :project: PimcLib

``int16_t`` Parsers
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalInt16(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt16(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt16(char const *s)
   :project: PimcLib

``int8_t`` Parsers
^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: pimc::parseDecimalInt8(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt8(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseDecimalInt8(char const *s)
   :project: PimcLib


Include file ``pimc/parsers/IPv4Parsers.hpp``
---------------------------------------------

The include file ``pimc/parsers/IPv4Parsers.hpp`` contains parsers for :cpp:class:`pimc::net::IPv4Address`
and :cpp:class:`pimc::net::IPv4Prefix` classes. Each function has three overloads, the first one takes an
iterator and a sentinel, the second one takes a range (which allows parsing ``std::string``\ s and
``std::string_view``\ s amongts other range like objects), and the last one takes a ``char const*``.

Example:

.. code-block:: c++

   #include <string>
   
   #include <fmt/format.h>

   #include "pimc/parsers/IPv4Parsers.hpp"
   #include "pimc/formatters/IPv4Formatters.hpp"

   using namespace std::string_literals;

   void f(std::string_view addr, std::string_view pfx) {
       auto ar = parseIPv4Address(addr);

       if (ar)
           fmt::print("parsed IPv4Address: {}\n", *ar);
       else
           fmt::print("invalid IPv4 address notation: '{}'\n", addr);

       auto pr = parseIPv4Prefix(pfx);

       if (pr)
           fmt::print("parsed IPv4Prefix: {}\n", *pr);
       else
           fmt::print("invalid IPv4 prefix notation: '{}'\n", pfx);
   }

IPv4Address parsers
^^^^^^^^^^^^^^^^^^^
   

.. doxygenfunction:: pimc::parseIPv4Address(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseIPv4Address(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseIPv4Address(char const *s)
   :project: PimcLib

IPv4Prefix parsers
^^^^^^^^^^^^^^^^^^
	     
.. doxygenfunction:: pimc::parseIPv4Prefix(I first, S last)
   :project: PimcLib

.. doxygenfunction:: pimc::parseIPv4Prefix(R &&r)
   :project: PimcLib

.. doxygenfunction:: pimc::parseIPv4Prefix(char const *s)
   :project: PimcLib

