================
UNIX Related API
================

Include file ``pimc/unix/GetOptLong.hpp``
-----------------------------------------

This include file contains a wrapper class :cpp:class:`pimc::GetOptLong` over the
``getopt_long`` function and several supplimentary classes.

:cpp:class:`pimc::GetOptLong` operates in two stages:

     #. The building stage where the internal structures required for for the command
	line processing, as well as the corresponding arguments to ``getopt_long`` are
	created.

     #. The processing stage, which consists of the invocation of ``getopt_long`` over
	the command line arguments and the creation of the the result object, which
	contains the processed command line data.

The building stage is implemented in the form of the builder pattern where an instance
of the class :cpp:class:`pimc::GetOptLong` is obtained through a call to the static
function :cpp:func:`pimc::GetOptLong::prog()` or :cpp:func:`pimc::GetOptLong::with()`
and the options are added to it through three member functions
:cpp:func:`flag() <pimc::GetOptLong::flag()>`
:cpp:func:`optional() <pimc::GetOptLong::optional()>` and
:cpp:func:`required() <pimc::GetOptLong::required()>`. Once all options are added,
the processing stage is initiated via a call to the
:cpp:func:`args() <pimc::GetOptLong::args()>` function, which returns an instance of
:cpp:class:`pimc::GetOptLongResult` object.

The building stage differentiates between two different types of command line options
-- flags and values. Flags are the option which do not have arguments, and because of
that their value are boolean true or false. The values are the options which always
take one and only one argument. :cpp:class:`pimc::GetOptLong` does not offer the third
alternative available with ``getopt_long``, namely they options which may take an
optional argument.

Both the flags and values require a unique 32-bit unsigned ID. The ID with the value
0xFFFFFFFF is reserved for the built-in ``-h|--help`` flag. The same ID is used to
identify the option value in the result object.

Both the flags and values must have a short option and/or a long one. The short option
is a single unique character, which on the command line appears after a single dash.
The long option is a unique string of at least two characters long, which on the
command line appears after two dashes. At least one of these must be defined for any
option.

Finally, both the flags and values must have the help string. The help string is
used to generate help displayed when the program is invoked with the  built-in flag
``-h|--help``. The help is formatted automatically using the whitespace characters
to separate the words. To create an unbreakable space the space character can be
escaped with ``\``, which is also used to escape itself. The escape character has
no special meaning in the case of all other characters, in which case it's simply
copied.

Unlike the flags, the values have some additional characteristics. The values may
be required or optional. (The flags are optional by definition -- a missing flag
results in false, whereas a present flag results in true). The values may
also be specified as singular or multiple. The singular values may appear on the
command line only once, while the multiple values can appear multiple times.

Because a value always requires an argument, its definition also requires the name
of the metavariable which is used in the generated help to name the argument to
the value option.

Once all of the flag and value options are added during the building stage, processing
of the actual command line arguments is initiated by calling
:cpp:func:`pimc::GetOptLong::args()`. If processing is successful this function
returns ``pimc::GetOptLongResult`` object, which can be queried for the flags and
values. If the command line is invalid, :cpp:func:`pimc::GetOptLong::args()` will
throw :cpp:struct:`pimc::GetOptLongError` exception.

To get a flag from the ``pimc::GetOptLongResult`` the following call should be
used:

.. code-block:: c++

   auto flag = result.flag(flagId);

where ``flagId`` is the numeric ID of the flag specified during the building stage.
If the flag was set, the returned value is ``true``, if not it's ``false``.

To get a value from the ``pimc::GetOptLongResult`` the following call should be
used:

.. code-block:: c++

   auto v = result.values(valueId);

where ``valueId`` is the numeric ID of the optional or required value specified
during the building stage. The returned value is always a const reverence to
``std::vector<std::string>``. The size of the vector depends on whether the
value option was required or optional and whether the ``multiple`` flags was
specified during the building stage. The table below shows the possible size
values.

+-------------+-----------------------+----------------------+
| Option Type | Multiple is ``false`` | Multiple is ``true`` |
+=============+=======================+======================+
| *optional*  | size: 0 or 1          | size 0 or *many*     |
+-------------+-----------------------+----------------------+
| *required*  | size: 1               | size 1 or *many*     |
+-------------+-----------------------+----------------------+


Allowed characters in short and long options and metavariable names
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

+---------------+------------------------------------------------------------------------+
| Item          | Allowed characters                                                     |
+===============+========================================================================+
| Short Option  | ``A-Z``, ``a-z``, ``0-9``                                              |
+---------------+------------------------------------------------------------------------+
| Long Option   | First character: ``A-Z``, ``a-z``, ``0-9``                             |
|               |                                                                        |
|               | Subsequent characters: ``A-Z``, ``a-z``, ``0-9``, ``_``, ``-``         |
+---------------+------------------------------------------------------------------------+
| Metavariable  | First character: ``A-Z``, ``a-z``, ``0-9``                             |
|               |                                                                        |
|               | Subsequent characters: ``A-Z``, ``a-z``, ``0-9``, ``_``, ``-``, ``=``  |
+---------------+------------------------------------------------------------------------+

Error handling
^^^^^^^^^^^^^^

The following two exceptions are thrown by classes  :cpp:class:`pimc::GetOptLong` and
:cpp:class:`GetOptLongResult`

:cpp:struct:`pimc::GetOptLongError`

     This is thrown when invalid operation is requested while adding options and/or
     querying the result. For example if a duplicate numeric ID is used in an option, or
     if a flag option result is queried as the value options. This exception is indicative
     of the programmer errors and therefore it should not be caught. Instead the reported
     errors should be corrected

     
:cpp:struct:`pimc::CommandLineError`
   
     This exception is throw when the command line does not match the defined options.
     This exception is indicative of the user error and therefore it must be caught and
     the error should be reported to the user.

     
Example
^^^^^^^

.. code-block:: c++

   #include <cstdint>
   #include <string>
   #include <fmt/format.h>
   
   #include "pimc/unix/GetOptLong.hpp"
   
   #define OID(id) static_cast<unsigned>(Options::id)
   
   using namespace std::string_literals;
   
   namespace {
   enum class Options : uint32_t {
       Verbose = 0,
       Define = 1,
       EvalID = 2,
       Include = 3,
       LogDir = 4,
   };
   
   } // anon.namespace
   
   int main(int argc, char * const* argv) {
       try {
           auto glor =
                   pimc::GetOptLong::with("[Options] file [file ...]"s)
                   .flag(OID(Verbose), 'v', "verbose"s, "Verbose output"s)
                   .optional(
                           OID(Define), 'D', ""s, "Key=Value"s,
                           "Define a key/value pair. Multiple key value "
                           "pairs can be specified by repeating this option. "
                           "They key must be a valid python identifier, which"
                           "must be separated from the value by the equal sign."s,
                           pimc::GetOptLong::Multiple)
                   .required(
                           OID(EvalID), 'e', "eval-id"s, "ID"s,
                           "Define the evaluation ID. This value is required")
                   .optional(
                           OID(Include), 'I', "include"s, "IncludeValue"s,
                           "Include the thing into operation. Multiple include "
                           "options can be specified by repeating this option",
                           pimc::GetOptLong::Multiple)
                   .optional(
                           OID(LogDir), pimc::GetOptLong::LongOnly, "log-dir"s, "Dir"s,
                           "Set log directory to the specified path. If this option "
                           "is not present, the log directory is the current directory")
                   .args(argc, argv);
   
           bool verpose = glor.flag(OID(Verbose));
           auto const& defines = glor.values(OID(Define));
           auto const& evalId = glor.values(OID(EvalID))[0];
           auto const& includes = glor.values(OID(Include));
           auto const& logDir = glor.values(OID(LogDir));
           auto const& files = glor.positional();
   
   	// Do something with the option values...
   	
       } catch (pimc::CommandLineError const& ex) {
           fmt::print(stderr, "error: {}\n", ex.what());
           return 1;
       }
   
       return 0;
   }
   
GetOptLong Reference
^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: pimc::GetOptLong
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::GetOptLongResult
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::GetOptLongError
   :project: PimcLib

.. doxygenstruct:: pimc::CommandLineError
   :project: PimcLib
