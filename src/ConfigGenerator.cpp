#include "ConfigGenerator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "TypeFactoryRegistry.hpp"
#include "utils/default_values.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

RandomValuesGenerator ConfigGenerator::rvg;
const json &ConfigGenerator::config = getConfig();
const json &ConfigGenerator::tplItems = getTemplateItems();
std::string ConfigGenerator::dataProfile = "random";
std::optional<uint32_t> ConfigGenerator::seedValue = std::nullopt;

ConfigGenerator::ConfigGenerator(const string &target)
    : target(target), testFolder(getAskeletonHome() / config["route"]["ut"] / target),
      configFilePath(fs::path(testFolder) / (target + ".cfg")) {
    fs::create_directories(testFolder);
    fs::path configFileTemplate = getAskeletonHome() / config["route"]["templates"] /
                                  config["file"]["template"]["cfg_tpl"];

    map<string, string> replacements = {
        {tplItems["tplitem"]["file_path"], target},
        {tplItems["tplitem"]["target"], target},
        {tplItems["tplitem"]["date_of_generation"], getTodayString()}};

    replaceTokensInFile(configFileTemplate, configFilePath, replacements);

    std::ostringstream meta;
    meta << "//// DATA PROFILE: " << dataProfile << "\n";
    if (seedValue.has_value()) {
        meta << "//// SEED: " << seedValue.value() << "\n";
    }
    appendToConfigFile(meta.str());
}

void ConfigGenerator::setRuleValues(
    const std::map<std::string, std::map<std::string, std::vector<long long>>> &rules) {
    ruleValues = rules;
}

void ConfigGenerator::setStringRuleValues(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>
        &rules) {
    ruleStringValues = rules;
}

void ConfigGenerator::setSeed(uint32_t seed) {
    rvg.setSeed(seed);
    seedValue = seed;
}

void ConfigGenerator::setProfile(const std::string &profileName) {
    dataProfile = profileName;
    if (profileName == "boundary") {
        rvg.setProfile(RandomProfile::Boundary);
    } else if (profileName == "safe") {
        rvg.setProfile(RandomProfile::Safe);
    } else if (profileName == "stress") {
        rvg.setProfile(RandomProfile::Stress);
    } else {
        rvg.setProfile(RandomProfile::Random);
    }
}

void ConfigGenerator::generateTestCase(const string &functionName,
                                       const vector<InfoVariable> &params,
                                       const InfoType &returnType,
                                       unsigned invocationNumber) const {
    generateTestCaseWithSetup(functionName, params, {}, "", returnType, invocationNumber);
}

void ConfigGenerator::generateTestCaseWithSetup(
    const string &functionName, const vector<InfoVariable> &params,
    const vector<InfoVariable> &setupParams, const string &setupPrefix,
    const InfoType &returnType, unsigned invocationNumber) const {
    (void)returnType;
    // The cfg stores only source case inputs. Any mirrored `oracle_*` variables
    // are reconstructed in generated test code from these same keys.
    currentFunctionName = functionName;
    currentInvocation = invocationNumber;
    stringstream ss;

    ss << functionName << "_" << invocationNumber << ":\n{\n";
    ss << generateParam(params, false);
    if (!setupParams.empty()) {
        ss << generateParam(setupParams, false, setupPrefix);
    }
    ss << "\n};\n\n";

    appendToConfigFile(ss.str());
}

void ConfigGenerator::generateConstructorTest(const string &ctorName,
                                              const vector<InfoVariable> &params,
                                              unsigned invocationNumber) const {
    currentFunctionName = ctorName;
    currentInvocation = invocationNumber;
    stringstream ss;

    ss << ctorName << "_" << invocationNumber << ":\n{\n";
    ss << generateParam(params);
    ss << "\n};\n\n";

    appendToConfigFile(ss.str());
}

std::string ConfigGenerator::generateParam(const vector<InfoVariable> &params,
                                           bool generatePointers,
                                           const string &prefix) const {
    stringstream ss;

    std::set<std::string> stack;
    for (const InfoVariable &param : params) {
        if (param.isOutputOnly()) {
            continue;
        }
        ss << generateParam(param, generatePointers, prefix, 0, stack) << "\n";
    }

    return ss.str();
}

std::string ConfigGenerator::generateParam(const InfoVariable &param,
                                           bool generatePointers,
                                           const string &prefix) const {
    std::set<std::string> stack;
    return generateParam(param, generatePointers, prefix, 0, stack);
}

std::string ConfigGenerator::generateParam(const std::vector<InfoVariable> &params,
                                           bool generatePointers,
                                           const std::string &prefix,
                                           unsigned depth,
                                           std::set<std::string> &stack) const {
    stringstream ss;
    for (const InfoVariable &param : params) {
        if (param.isOutputOnly()) {
            continue;
        }
        ss << generateParam(param, generatePointers, prefix, depth, stack) << "\n";
    }
    return ss.str();
}

std::string ConfigGenerator::generateParam(const InfoVariable &param,
                                           bool generatePointers,
                                           const std::string &prefix, unsigned depth,
                                           std::set<std::string> &stack) const {
    pair<InfoVariable, InfoVariable> pointers = param.getPointers();
    InfoType underlying = param.getUnderlyingType();
    const auto factory = TypeFactoryRegistry::get().find(underlying, currentFunctionName);
    const bool hasFactory = factory.has_value() &&
                            factory->strategy != TypeInitStrategy::Random;

    string typeForValue = underlying.formatted;
    if (underlying.isMap() && typeForValue.find("<") == std::string::npos) {
        typeForValue = underlying.original;
    }
    const bool cStringPointer = param.isPointer() && underlying.original == "char";

    string value;
    auto funcStrIt = ruleStringValues.find(currentFunctionName);
    if (funcStrIt != ruleStringValues.end() &&
        containsSubstring(underlying.original, "string")) {
        const auto &paramRules = funcStrIt->second;
        auto ruleIt = paramRules.find(param.name);
        if (ruleIt != paramRules.end() && !ruleIt->second.empty()) {
            const auto &vals = ruleIt->second;
            value = vals[(currentInvocation - 1) % vals.size()];
        }
    }

    auto funcIt = ruleValues.find(currentFunctionName);
    if (funcIt != ruleValues.end()) {
        const auto &paramRules = funcIt->second;
        auto ruleIt = paramRules.find(param.name);
        const bool numericType = !underlying.isContainer() && !underlying.isRecord() &&
                                 !containsSubstring(underlying.original, "string");
        if (ruleIt != paramRules.end() && !ruleIt->second.empty() && numericType) {
            const auto &vals = ruleIt->second;
            long long selected = vals[(currentInvocation - 1) % vals.size()];
            value = std::to_string(selected);
        }
    }

    if (value.empty() && hasFactory) {
        if (factory->strategy == TypeInitStrategy::Dummy) {
            value = getDefaultValueForType(underlying).value_or(
                getZeroValueForType(underlying));
        } else if (factory->strategy == TypeInitStrategy::Zeroed) {
            value = getZeroValueForType(underlying);
        }
    }

    if (value.empty() && param.defaultValue.has_value()) {
        value = param.defaultValue.value();
    }

    stringstream ss;

    const bool isRecord = underlying.isRecord() && !underlying.isContainer();
    const bool hasFields = !underlying.getRecordFields().empty();

    if (hasFactory && isRecord) {
        return "";
    }

    if (isRecord && hasFields) {
        const std::string key = underlying.original;
        if (depth > 2 || stack.count(key) > 0) {
            return "";
        }
        stack.insert(key);
        ss << generateParam(pointers.first.getRecordFields(), generatePointers,
                            prefix + pointers.first.name + ".", depth + 1, stack);
        if (generatePointers && (param.isPointer() || param.isReference()))
            ss << generateParam(pointers.second.getRecordFields(), generatePointers,
                                prefix + pointers.second.name + ".", depth + 1,
                                stack);
        stack.erase(key);
    } else if (underlying.isOptional() || underlying.isPair() || underlying.isTuple()) {
        ss << generateStructuredParam(pointers.first.name, underlying, prefix, depth, stack);
        if (generatePointers && (param.isPointer() || param.isReference())) {
            const std::string extra =
                generateStructuredParam(pointers.second.name, underlying, prefix, depth, stack);
            if (!extra.empty()) {
                ss << "\n" << extra;
            }
        }
    } else {
        if (value.empty()) {
            value = generateScalarValue(cStringPointer ? InfoType("std::string") : underlying);
        }
        ss << "\t" << prefix + pointers.first.name << "=" << value << ";#"
           << param.original;
        if (generatePointers && (param.isPointer() || param.isReference()))
            ss << "\n\t" << prefix + pointers.second.name << "=" << value << ";#"
               << param.original;
    }

    return ss.str();
}

std::string ConfigGenerator::generateStructuredParam(const std::string &name,
                                                     const InfoType &type,
                                                     const std::string &prefix,
                                                     unsigned depth,
                                                     std::set<std::string> &stack) const {
    if (depth > 3) {
        return "";
    }

    const auto args = type.getTemplateArguments();
    std::ostringstream ss;

    if (type.isOptional() && !args.empty()) {
        const bool hasValue = (currentInvocation % 2) != 0;
        ss << "\t" << prefix + name << ".has_value=" << (hasValue ? "true" : "false")
           << ";#bool";
        if (hasValue) {
            InfoVariable nested{"value", args.front()};
            const std::string nestedContent =
                generateParam(nested, false, prefix + name + ".", depth + 1, stack);
            if (!nestedContent.empty()) {
                ss << "\n" << nestedContent;
            }
        }
        return ss.str();
    }

    if (type.isPair() && args.size() == 2) {
        InfoVariable first{"first", args[0]};
        InfoVariable second{"second", args[1]};
        ss << generateParam(first, false, prefix + name + ".", depth + 1, stack);
        const std::string secondContent =
            generateParam(second, false, prefix + name + ".", depth + 1, stack);
        if (!secondContent.empty()) {
            if (!ss.str().empty()) {
                ss << "\n";
            }
            ss << secondContent;
        }
        return ss.str();
    }

    if (type.isTuple() && !args.empty()) {
        for (size_t i = 0; i < args.size(); ++i) {
            InfoVariable item{std::to_string(i), args[i]};
            const std::string itemContent =
                generateParam(item, false, prefix + name + ".", depth + 1, stack);
            if (!itemContent.empty()) {
                if (!ss.str().empty()) {
                    ss << "\n";
                }
                ss << itemContent;
            }
        }
        return ss.str();
    }

    return "";
}

std::string ConfigGenerator::generateScalarValue(const InfoType &type) const {
    if (type.isOptional() || type.isPair() || type.isTuple()) {
        return "0";
    }

    std::string typeForValue = type.formatted;
    if (containsSubstring(type.original, "string")) {
        typeForValue = "string";
    } else if (type.isMap() && typeForValue.find("<") == std::string::npos) {
        typeForValue = type.original;
    }

    return rvg.getRandomValue(typeForValue);
}

void ConfigGenerator::appendToConfigFile(const string &content) const {
    std::ofstream configfileStream(configFilePath, ios_base::app);
    if (configfileStream.is_open()) {
        configfileStream << content;
    } else {
        cerr << askeleton::errors::openFileError(configFilePath) << endl;
    }
}
