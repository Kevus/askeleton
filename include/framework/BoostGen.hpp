#pragma once

#include "Generator.hpp"

class BoostGen : public Generator {
public:
    BoostGen() = delete;
    ~BoostGen() = default;

    BoostGen(const std::string &targetName, const std::string &filePath,
             bool isFromClass = false);

    void generateFunctionAssert(const std::string &function,
                                const std::vector<InfoVariable> &parameters,
                                const InfoType &returnType) override;
    void generateMethodAssert(const std::string &method,
                              const std::vector<InfoVariable> &parameters,
                              const InfoType &returnType) override;
    void generateConstructorAssert(
        const std::vector<InfoVariable> &parameters) override;

private:
    std::string generatePointersAsserts(
        const std::vector<InfoVariable> &parameters) const override;

    const static std::string BOOST_ASSERT_POINTER;
};