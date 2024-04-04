#pragma once
#include <string>

namespace askeleton {
const std::string ASKELETON_VARNAME = "ASKELETON_HOME";

namespace files {
const std::string SUPPORTED_TYPES = "SupportedTypes.txt";
const std::string TEMPLATE_BOOST = "BoostTest.tpl";
}; // namespace files

namespace routes {
const std::string TEMPLATES_ROUTE = "Generator/Templates/";
const std::string TEST_ROUTE = "Generated/UT/";

const std::string BT_TEMPLATES_ROUTE = TEMPLATES_ROUTE + files::TEMPLATE_BOOST;
const std::string TEST_FILE_ENDING = "_test.cpp";

const auto generateTestClassRoute = [](const std::string &filename,
                                       const std::string &classname) {
    return TEST_ROUTE + filename + "/" + classname + TEST_FILE_ENDING;
};
const auto generateTestFileRoute = [](const std::string &filename) {
    return generateTestClassRoute(filename, filename);
};
} // namespace routes

// TEMPLATE ITEMS
namespace tplitems {
const std::string CLASS_NAME = "{className}";
const std::string FILE_NAME = "{fileName}";
const std::string CPP_PATH = "{cppPath}";

// BOOST_TEST TEMPLATE
const std::string POINTER_INIT_TOKEN = "{pointerInitToken}";
const std::string POINTER_DESTROY_TOKEN = "{pointerDestroyToken}";
const std::string ASSERT = "//{assert}";

// FIXTURE TEMPLATE
const std::string FILE_PATH = "{filePath}";
const std::string CFG_NAME = "{cfgName}";
const std::string DATE_OF_GENERATION = "{dateOfGeneration}";
const std::string INCLUDES = "{includes}";
const std::string NAMESPACES = "{namespaces}";
const std::string READ_OBJECT = "//{readObject}";
const std::string NEW_METHODS = "{newMethods}";
const std::string CLASS_NAME_TEST = "{classNameTest}";
const std::string OVERLOAD_OPERATOR = "//{overloadOperator}";
} // namespace tplitems

namespace errors {
const auto openFileError = [](const std::string &filename) {
    return "File " + filename +
           " couldn't be opened. Check permissions and the route.";
};
} // namespace errors

} // namespace askeleton
