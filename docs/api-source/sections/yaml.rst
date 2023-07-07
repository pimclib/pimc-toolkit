===========================
 YAML Processing Utilities
===========================

The YAML utility classes and functions which help process configuration files expressed
in the YAML format.

Example:

.. code-block:: cpp

   #include <fmt/format.h>
   
   #include "pimc/yaml/LoadAll.hpp"
   #include "pimc/yaml/Structured.hpp"
   #include "pimc/yaml/ErrorHandler.hpp"
   
   namespace pimc {
   
   void processConfig(YAML::Node const& n, char const* yamlfn) {
       yaml::StderrErrorHandler ec{yamlfn};
       auto root = yaml::ValueContext::root(n);
   
       std::vector<std::string> names;
       std::vector<int> namesLines;
   
       std::string value;
       int valueLine;
   
       Optional<std::string> style;
       int styleLine;
   
       std::string typeName;
       int typeNameLine;
       Optional<std::string> typeHint;
       int typeHintLine;
   
       auto cfg = ec.chk(root.getMapping());
       if (cfg) {
           auto rNames = ec.chk(
                   cfg->required("names").flatMap(yaml::sequence("names")));
   
           if (rNames) {
               names.reserve(rNames->size());
               namesLines.reserve(rNames->size());
               for (auto const& rName: rNames->list()) {
                   auto sName = ec.chk(rName.getScalar("name"));
                   if (sName) {
                       names.emplace_back(sName->value());
                       namesLines.emplace_back(sName->line());
                   }
               }
           }
   
           auto rValue = ec.chk(
                   cfg->required("value").flatMap(yaml::scalar("value")));
           if (rValue) {
               value = rValue->value();
               valueLine = rValue->line();
           }
   
           auto oStyle = cfg->optional("style");
           if (oStyle) {
               auto rStyle = ec.chk(oStyle->getScalar("style"));
               if (rStyle) {
                   style = rStyle->value();
                   styleLine = rStyle->line();
               }
           }
   
           auto rType = ec.chk(
                   cfg->required("type").flatMap(yaml::mapping("type")));
           if (rType) {
               auto rTypeName = ec.chk(
                       rType->required("name").flatMap(yaml::scalar("type name")));
               if (rTypeName) {
                   typeName = rTypeName->value();
                   typeNameLine = rTypeName->line();
               }
   
               auto oTypeHint = rType->optional("hint");
               if (oTypeHint) {
                   auto rTypeHint = ec.chk(oTypeHint->getScalar("type hint"));
                   if (rTypeHint) {
                       typeHint = rTypeHint->value();
                       typeHintLine = rTypeHint->line();
                   }
               }
   
               ec.chk(rType->extraneous());
           }
   
           ec.chk(cfg->extraneous());
       }
   
       if (ec.errors() == 0) {
           fmt::print("Names:\n");
           for (size_t i{0}; i < names.size(); ++i) {
               fmt::print("  {} @ {}\n", names[i], namesLines[i]);
           }
   
           fmt::print("Value: '{}' @ {}\n", value, valueLine);
   
           if (style)
               fmt::print("Style: '{}' @ {}\n", style.value(), styleLine);
   
           fmt::print("Type:\n");
           fmt::print("  name: '{}' @ {}\n", typeName, typeNameLine);
   
           if (typeHint) {
               fmt::print("  hint: '{}' @ {}\n", typeHint.value(), typeHintLine);
           }
       }
   }
   
   } // namespace pimc
   
   int main(int argc, char** argv) {
       if (argc != 2) {
           fmt::print(stderr, "usage: {} <yaml-file>\n", argv[0]);
           return 2;
       }
   
       auto r = pimc::yaml::loadAll(argv[1]);
       if (not r) {
           fmt::print(stderr, "error: {}\n", r.error());
           return 1;
       }
   
       fmt::print("yaml file loaded successfully\n");
   
       auto cfgs = std::move(r).value();
   
       int i{0};
       for (auto& n: cfgs) {
           fmt::print("*** DOCUMENT #{}\n", i);
           pimc::processConfig(n, argv[1]);
       }
   
       return 0;
   }
   

Reference
=========

Include file ``pimc/yaml/LoadAll.hpp``
------------------------------------------

This include file contains one function :cpp:func:`pimc::yaml::loadAll`, which
loads all documents from a YAML file.

.. doxygenfunction:: pimc::yaml::loadAll
   :project: PimcLib

.. _yaml-structured-yaml-hpp:
	     
Include file ``pimc/yaml/Structured.hpp``
---------------------------------------------

This include file contains the utility classes and functions which allow processing
the parsed YAML data in a structured manner.

.. doxygenclass:: pimc::yaml::NodeContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::yaml::ValueContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::yaml::MappingContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::yaml::SequenceContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::yaml::Scalar
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::yaml::ErrorContext
   :project: PimcLib
   :members:

.. doxygenfunction:: pimc::yaml::scalar()
   :project: PimcLib

.. doxygenfunction:: pimc::yaml::scalar(std::string const& name)
   :project: PimcLib

.. doxygenfunction:: pimc::yaml::mapping()
   :project: PimcLib

.. doxygenfunction:: pimc::yaml::mapping(std::string name)
   :project: PimcLib

.. doxygenfunction:: pimc::yaml::sequence()
   :project: PimcLib

.. doxygenfunction:: pimc::yaml::sequence(std::string name)
   :project: PimcLib

Include file ``pimc/yaml/ErrorHandler.hpp``
---------------------------------------------------------

This include file contains utility classes which help keep track of errors while
interpreting the parsed YAML data using the functionality in the include file
:ref:`"pimc/yaml/StructuredYaml.hpp" <yaml-structured-yaml-hpp>`.

.. doxygenenum:: pimc::yaml::ErrorContextShow
   :project: PimcLib

.. doxygenclass:: pimc::yaml::ErrorHandler
   :project: PimcLib
   :members:

.. doxygenstruct:: pimc::yaml::StderrErrorHandler
   :project: PimcLib
   :members:

Include file ``pimc/yaml/BuilderBase.hpp``
------------------------------------------

This include file contains a utilities which help create builder style YAML
processors.

.. doxygenconcept:: pimc::yaml::ErrorConsumer
   :project: PimcLib

.. doxygenclass:: pimc::yaml::BuilderBase
   :project: PimcLib
   :members:
