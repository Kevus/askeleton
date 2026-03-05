#pragma once

#include "Generator.hpp"
#include "VariableInfo.hpp"

#include <map>
#include <string>
#include <vector>

class BoostGen : public Generator {
public:
    BoostGen() = delete;
    ~BoostGen() = default;

    BoostGen(const std::string &targetName,
             const std::string &targetQualifiedName,
             const std::string &filePath,
             bool isFromClass = false);

    void generateFunctionAssert(const std::string &function,
                                const std::vector<InfoVariable> &parameters,
                                const InfoType &returnType) override;
    void generateMethodAssert(const std::string &method,
                              const std::vector<InfoVariable> &parameters,
                              const InfoType &returnType, bool isStatic = false) override;
    void generateConstructorAssert(const std::vector<InfoVariable> &parameters) override;
    bool supportsConstructorTests() const override;

private:
    std::string generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                                        const std::string &function) const override;
    std::string generateAssertForFunction(const std::string &function,
                                          const std::vector<InfoVariable> &parameters,
                                          const InfoType &returnType,
                                          bool isStatic) const;
    void generateFullAssert(const std::string &function,
                            const std::vector<InfoVariable> &parameters,
                            const InfoType &returnType, bool isStatic) override;
};
