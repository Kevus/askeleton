#include "GenerationEligibility.hpp"

#include <algorithm>

#include "TypeFactoryRegistry.hpp"

using namespace clang;

namespace {

bool canMaterializeConstructorParams(const std::vector<InfoVariable> &params,
                                     const std::string &functionName) {
    try {
        validateTypesMaterialization(params, functionName);
        return true;
    } catch (const ComplexTypeException &) {
        return false;
    }
}

unsigned countIndirections(const InfoType &type) {
    if (!type.type.isNull()) {
        unsigned depth = 0;
        QualType current = type.type;
        while (!current.isNull() &&
               (current->isPointerType() || current->isReferenceType())) {
            ++depth;
            current = current->getPointeeType();
        }
        return depth;
    }

    unsigned depth = 0;
    for (char c : type.original) {
        if (c == '*' || c == '&') {
            ++depth;
        }
    }
    return depth;
}

const CXXRecordDecl *getRecordDecl(const InfoType &type) {
    if (type.type.isNull()) {
        return nullptr;
    }
    return type.type->getAsCXXRecordDecl();
}

bool hasPublicUsableDestructor(const CXXRecordDecl *record) {
    if (!record) {
        return true;
    }

    const auto *dtor = record->getDestructor();
    if (!dtor) {
        return true;
    }

    return !dtor->isDeleted() && dtor->getAccess() == AS_public;
}

bool canDefaultConstructForFixture(const CXXRecordDecl *record) {
    if (!record) {
        return true;
    }
    if (!record->hasDefaultConstructor()) {
        return false;
    }

    for (const auto *ctor : record->ctors()) {
        if (!ctor || !ctor->isDefaultConstructor()) {
            continue;
        }
        return !ctor->isDeleted() && ctor->getAccess() == AS_public;
    }

    return true;
}

bool usesExplicitFactory(const InfoType &type, const std::string &functionName) {
    const auto factory = TypeFactoryRegistry::get().find(type, functionName);
    return factory.has_value() && factory->strategy == TypeInitStrategy::Factory &&
           !factory->expr.empty();
}

[[noreturn]] void throwUnsupportedType(const std::string &reason,
                                       const std::string &typeName,
                                       const std::string &detail) {
    throw ComplexTypeException(reason, detail + ": " + typeName);
}

} // namespace

bool requiresMutableAliasHandling(const std::vector<InfoVariable> &params) {
    return std::any_of(params.begin(), params.end(), [](const InfoVariable &param) {
        return (param.isPointer() || param.isReference()) && param.isMutableOutput();
    });
}

unsigned minimumCoverageInvocations(const std::vector<InfoVariable> &params) {
    return std::any_of(params.begin(), params.end(), [](const InfoVariable &param) {
               return param.getUnderlyingType().isOptional();
           })
               ? 2u
               : 1u;
}

void validateTypeMaterialization(const InfoType &type,
                                 const std::string &functionName) {
    if (countIndirections(type) > 1) {
        throwUnsupportedType("unsupported_indirection", type.original,
                             "more than one pointer/reference indirection is not supported");
    }

    const InfoType materialized =
        (type.isPointer() || type.isReference()) ? type.getUnderlyingType() : type;

    if (materialized.isPointer() || materialized.isReference()) {
        throwUnsupportedType("unsupported_indirection", type.original,
                             "nested pointer/reference materialization is not supported");
    }

    if (!materialized.isRecord()) {
        return;
    }

    const auto *record = getRecordDecl(materialized);
    if (!record) {
        return;
    }

    if (record->isAbstract()) {
        throwUnsupportedType("abstract_record", materialized.original,
                             "abstract record cannot be instantiated for fixtures");
    }

    if (!hasPublicUsableDestructor(record)) {
        throwUnsupportedType("non_public_lifecycle", materialized.original,
                             "record does not have a public usable destructor");
    }

    if (!usesExplicitFactory(materialized, functionName) &&
        !canDefaultConstructForFixture(record)) {
        throwUnsupportedType("missing_fixture_strategy", materialized.original,
                             "record requires a default constructor or explicit "
                             "factory for fixture setup");
    }
}

void validateTypesMaterialization(const std::vector<InfoVariable> &variables,
                                  const std::string &functionName) {
    for (const auto &variable : variables) {
        validateTypeMaterialization(variable, functionName);
    }
}

std::optional<ConstructorSelection>
selectConstructorForInstantiation(const CXXRecordDecl *record,
                                  const std::string &functionName) {
    if (!record) {
        return std::nullopt;
    }

    std::optional<ConstructorSelection> best;
    for (const auto *ctor : record->ctors()) {
        if (!ctor || ctor->isImplicit() || ctor->isDeleted() ||
            ctor->isCopyOrMoveConstructor()) {
            continue;
        }
        if (ctor->getAccess() != AS_public) {
            continue;
        }

        ConstructorSelection candidate;
        candidate.useDefaultConstructor = ctor->isDefaultConstructor();
        candidate.totalParameters = ctor->getNumParams();

        unsigned requiredCount = ctor->getNumParams();
        while (requiredCount > 0 &&
               ctor->getParamDecl(requiredCount - 1)->hasDefaultArg()) {
            --requiredCount;
        }

        for (unsigned i = 0; i < requiredCount; ++i) {
            candidate.params.emplace_back(ctor->getParamDecl(i));
        }

        if (candidate.useDefaultConstructor) {
            return candidate;
        }

        if (!canMaterializeConstructorParams(candidate.params, functionName)) {
            continue;
        }

        if (!best.has_value() || candidate.params.size() < best->params.size() ||
            (candidate.params.size() == best->params.size() &&
             candidate.totalParameters < best->totalParameters)) {
            best.emplace(std::move(candidate));
        }
    }

    if (best.has_value()) {
        return best;
    }

    if (record->hasDefaultConstructor()) {
        return ConstructorSelection{};
    }

    return std::nullopt;
}
