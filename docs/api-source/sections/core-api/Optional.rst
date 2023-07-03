=========================================
 Include file ``pimc/core/Optional.hpp``
=========================================

The file ``Optional.hpp`` contains the class template
:cpp:class:`pimc::Optional`. This class provides a way to store
a value of a certain type or nothing. An object :cpp:class:`pimc::Optional`
at any time either contains a value or is empty.

The value is allocated directly within the storage of the
:cpp:class:`pimc::Optional`, no dynamic allocation takes place.

The value type of :cpp:class:`pimc::Optional` may be a value or a value
reference.

The :cpp:class:`pimc::Optional` objects are comparable between each other
and with the values which are comparable with the value that the Optional
holds.

Example:

.. code-block:: c++

   #include <string>
   #include <unordered_map>

   #include "pimc/core/Optional.hpp"

   class PropMap {
   public:


	pimc::Optional<std::string const&> getProp(std::string const& k) {
	    if (auto ii = m_.find(k); ii != m_.end())
	        return ii->second;

	    return {};
	}
   private:
	std::unordered_map<std::string, std::string> m_;
   };

In this example the function ``PropMap::getProp()`` returns an optional value
which contains the property value if the supplied key exists in the map,
otherwise it returns an empty optional.

Reference
=========

.. doxygenclass:: pimc::Optional
   :project: PimcLib
   :members:
