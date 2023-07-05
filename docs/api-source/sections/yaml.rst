===========================
 YAML Processing Utilities
===========================

The YAML utility classes and function help process configuration files expressed
in the YAML format.

For example:

.. code-block:: cpp

   void processConfig(YAML::Node const& n, const char* yamlfn) {
       ErrorReporter er{yamlfn};
       auto root = ValueContext::root(n);
   
       std::vector<std::string> names;
   
       auto m0 = root.getMapping("Config");
       if (m0) {
           auto names1 = m0->required("names").flatMap(sequence("Names"));
           fmt::print("** sequence 'names' at {}\n", names1->line());
   
           if (names1) {
               auto& nl1 = names1.value();
               names.reserve(nl1.size());
               namesLines.reserve(nl1.size());
               for (size_t i{0}; i < nl1.size(); ++i) {
                   auto rn = nl1[i]->getScalar("name");
   
                   if (rn) {
                       namesLines.emplace_back(rn->line());
                       names.emplace_back(std::move(rn)->value());
                   }
                   else er.error(rn.error());
               }
           } else er.error(names1.error());
       } else er.error(m0.error());
   }
   
   int main(int argc, char** argv) {
       if (argc != 2) {
           fmt::print(stderr, "usage: {} <yaml-file>\n", argv[0]);
           return 2;
       }
   
       auto r = pimc::yamlLoadAll(argv[1]);
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

Include file ``pimc/yaml/YamlLoadAll.hpp``
------------------------------------------

.. doxygenfunction:: pimc::yamlLoadAll
   :project: PimcLib

Include file ``pimc/yaml/StructuredYaml.hpp``
---------------------------------------------

.. doxygenclass:: pimc::NodeContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::ValueContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::MappingContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::SequenceContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::ScalarContext
   :project: PimcLib
   :members:

.. doxygenclass:: pimc::ErrorContext
   :project: PimcLib
   :members:

.. doxygenfunction:: pimc::scalar()
   :project: PimcLib

.. doxygenfunction:: pimc::scalar(std::string const& name)
   :project: PimcLib

.. doxygenfunction:: pimc::mapping()
   :project: PimcLib

.. doxygenfunction:: pimc::mapping(std::string name)
   :project: PimcLib

.. doxygenfunction:: pimc::sequence()
   :project: PimcLib

.. doxygenfunction:: pimc::sequence(std::string name)
   :project: PimcLib
