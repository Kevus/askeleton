#pragma once

#include "Generator.hpp"
#include "VariableInfo.hpp"
#include <string>
#include <vector>

class GTestGenerator : public Generator {
public:
    GTestGenerator() = delete;
    ~GTestGenerator() = default;

    GTestGenerator(const std::string &targetName,
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

    void generateFullAssert(const std::string &function,
                            const std::vector<InfoVariable> &parameters,
                            const InfoType &returnType, bool isStatic) override;
    void copyMainFile() const;
};
