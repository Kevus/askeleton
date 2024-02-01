#pragma once
#include <string>

namespace askeleton{
	namespace routes {
		const std::string TEMPLATES_ROUTE = "Generator/Templates/";
		const std::string TEST_ROUTE = "Generated/UT/";

		const std::string BT_TEMPLATES_ROUTE = TEMPLATES_ROUTE + "BoostTest.tpl";
		const std::string TEST_FILE_ENDING = "_test.cpp";

		const auto generateTestClassRoute = [](const std::string &filename, const std::string &classname) {
			return TEST_ROUTE + filename + "/" + classname + TEST_FILE_ENDING;
		};
		const auto generateTestFileRoute = [](const std::string &filename) {
			return generateTestClassRoute(filename, filename);
		};
	} // namespace routes

	namespace fileitems {
		const std::string CLASS_NAME = "{className}";
		const std::string POINTER_INIT_TOKEN = "{pointerInitToken}";
		const std::string POINTER_DESTROY_TOKEN = "{pointerDestroyToken}";
		const std::string ASSERT = "{assert}";
	} // namespace fileitems
	
	namespace errors {
		const auto openFileError = [](const std::string &filename) {
			return "File " + filename + " couldn't be opened. Check permissions and the route.";
		};
	} // namespace errors
	
} // namespace constant

