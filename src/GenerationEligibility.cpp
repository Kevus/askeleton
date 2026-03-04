#include "GenerationEligibility.hpp"

#include <algorithm>

#include "InstanceStrategyRegistry.hpp"
#include "TypeFactoryRegistry.hpp"
#include "clang/AST/ASTContext.h"

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
    const auto *record = type.type->getAsCXXRecordDecl();
    if (!record) {
        return nullptr;
    }
    return record->getDefinition();
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

const CXXRecordDecl *canonicalRecord(const CXXRecordDecl *record) {
    if (!record) {
        return nullptr;
    }

    if (const auto *definition = record->getDefinition()) {
        record = definition;
    }
    return record->getCanonicalDecl();
}

const CXXRecordDecl *recordFromQualType(QualType type) {
    if (type.isNull()) {
        return nullptr;
    }

    type = type.getNonReferenceType().getCanonicalType().getUnqualifiedType();
    const auto *record = type->getAsCXXRecordDecl();
    if (!record) {
        return nullptr;
    }

    return canonicalRecord(record);
}

bool isSameOrDerivedRecord(const CXXRecordDecl *target, QualType candidateType) {
    const auto *targetRecord = canonicalRecord(target);
    const auto *candidateRecord = recordFromQualType(candidateType);
    if (!targetRecord || !candidateRecord) {
        return false;
    }

    if (targetRecord == candidateRecord) {
        return true;
    }

    return candidateRecord->isDerivedFrom(targetRecord);
}

std::optional<InstanceSubjectKind>
compatibleReturnSubject(const CXXRecordDecl *target, const FunctionDecl *function) {
    if (!target || !function) {
        return std::nullopt;
    }

    QualType returnType = function->getReturnType();
    if (returnType.isNull()) {
        return std::nullopt;
    }

    InstanceSubjectKind subjectKind = InstanceSubjectKind::Value;
    QualType objectType = returnType;
    if (returnType->isPointerType()) {
        subjectKind = InstanceSubjectKind::Pointer;
        objectType = returnType->getPointeeType();
    } else if (returnType->isReferenceType()) {
        subjectKind = InstanceSubjectKind::Reference;
        objectType = returnType.getNonReferenceType();
    }

    if (!isSameOrDerivedRecord(target, objectType)) {
        return std::nullopt;
    }

    return subjectKind;
}

std::vector<InfoVariable> buildRequiredParams(const FunctionDecl *function) {
    std::vector<InfoVariable> params;
    if (!function) {
        return params;
    }

    unsigned requiredCount = function->getNumParams();
    while (requiredCount > 0 &&
           function->getParamDecl(requiredCount - 1)->hasDefaultArg()) {
        --requiredCount;
    }

    params.reserve(requiredCount);
    for (unsigned i = 0; i < requiredCount; ++i) {
        params.emplace_back(function->getParamDecl(i));
    }
    return params;
}

std::optional<InstancePlan> tryConstructorPlan(const CXXRecordDecl *record,
                                               const std::string &functionName) {
    std::optional<InstancePlan> best;
    for (const auto *ctor : record->ctors()) {
        if (!ctor || ctor->isImplicit() || ctor->isDeleted() ||
            ctor->isCopyOrMoveConstructor()) {
            continue;
        }
        if (ctor->getAccess() != AS_public) {
            continue;
        }

        InstancePlan candidate;
        candidate.kind = ctor->isDefaultConstructor()
                             ? InstancePlanKind::DefaultConstructor
                             : InstancePlanKind::Constructor;
        candidate.totalParameters = ctor->getNumParams();
        candidate.setupParams = buildRequiredParams(ctor);

        if (candidate.usesDefaultConstructor()) {
            return candidate;
        }

        if (!canMaterializeConstructorParams(candidate.setupParams, functionName)) {
            continue;
        }

        if (!best.has_value() ||
            candidate.setupParams.size() < best->setupParams.size() ||
            (candidate.setupParams.size() == best->setupParams.size() &&
             candidate.totalParameters < best->totalParameters)) {
            best.emplace(std::move(candidate));
        }
    }

    if (best.has_value()) {
        return best;
    }

    if (record->hasDefaultConstructor()) {
        return InstancePlan{};
    }

    return std::nullopt;
}

std::optional<InstancePlan> tryConfiguredInstancePlan(const CXXRecordDecl *record,
                                                      const std::string &functionName) {
    if (!record) {
        return std::nullopt;
    }

    const auto strategy =
        InstanceStrategyRegistry::get().find(
            InfoType(record->getASTContext().getRecordType(record)), functionName);
    if (!strategy.has_value()) {
        return std::nullopt;
    }

    InstancePlan plan;
    plan.kind = InstancePlanKind::Configured;
    plan.subjectKind = strategy->subjectKind;
    if (!strategy->ownerType.empty() && !strategy->ownerCallable.empty()) {
        plan.kind = InstancePlanKind::OwnerFactory;
        plan.ownerTypeName = strategy->ownerType;
        plan.callableExpr = strategy->ownerCallable;
        plan.ownerPlan = std::make_shared<InstancePlan>();
        plan.ownerPlan->kind = strategy->ownerExpr.empty() ? InstancePlanKind::DefaultConstructor
                                                           : InstancePlanKind::Configured;
        plan.ownerPlan->initExpr = strategy->ownerExpr;
        plan.ownerPlan->subjectKind = strategy->ownerSubjectKind;
    } else {
        plan.initExpr = strategy->expr;
    }
    return plan;
}

std::optional<InstancePlan> tryStaticFactoryPlan(const CXXRecordDecl *record,
                                                 const std::string &functionName) {
    std::optional<InstancePlan> best;

    for (const auto *method : record->methods()) {
        if (!method || !method->isStatic() || method->isDeleted() ||
            method->getAccess() != AS_public) {
            continue;
        }
        const auto subjectKind = compatibleReturnSubject(record, method);
        if (!subjectKind.has_value()) {
            continue;
        }

        InstancePlan candidate;
        candidate.kind = InstancePlanKind::StaticFactory;
        candidate.subjectKind = *subjectKind;
        candidate.callableExpr =
            record->getQualifiedNameAsString() + "::" + method->getNameAsString();
        candidate.totalParameters = method->getNumParams();
        candidate.setupParams = buildRequiredParams(method);
        if (!canMaterializeConstructorParams(candidate.setupParams, functionName)) {
            continue;
        }

        if (!best.has_value() ||
            candidate.setupParams.size() < best->setupParams.size() ||
            (candidate.setupParams.size() == best->setupParams.size() &&
             candidate.totalParameters < best->totalParameters)) {
            best.emplace(std::move(candidate));
        }
    }

    return best;
}

void considerFreeFactoryInContext(const CXXRecordDecl *record,
                                  const std::string &functionName,
                                  const DeclContext *context,
                                  std::optional<InstancePlan> &best) {
    if (!context) {
        return;
    }

    for (const auto *decl : context->decls()) {
        const auto *function = dyn_cast<FunctionDecl>(decl);
        if (!function || isa<CXXMethodDecl>(function) || function->isDeleted()) {
            continue;
        }
        if (!function->isExternallyVisible()) {
            continue;
        }
        const auto subjectKind = compatibleReturnSubject(record, function);
        if (!subjectKind.has_value()) {
            continue;
        }

        InstancePlan candidate;
        candidate.kind = InstancePlanKind::FreeFactory;
        candidate.subjectKind = *subjectKind;
        candidate.callableExpr = function->getQualifiedNameAsString();
        candidate.totalParameters = function->getNumParams();
        candidate.setupParams = buildRequiredParams(function);
        if (!canMaterializeConstructorParams(candidate.setupParams, functionName)) {
            continue;
        }

        if (!best.has_value() ||
            candidate.setupParams.size() < best->setupParams.size() ||
            (candidate.setupParams.size() == best->setupParams.size() &&
             candidate.totalParameters < best->totalParameters)) {
            best.emplace(std::move(candidate));
        }
    }
}

std::optional<InstancePlan> tryFreeFactoryPlan(const CXXRecordDecl *record,
                                               const std::string &functionName) {
    std::optional<InstancePlan> best;
    considerFreeFactoryInContext(record, functionName, record->getDeclContext(), best);

    const auto *tu = record->getASTContext().getTranslationUnitDecl();
    if (!best.has_value() && tu != record->getDeclContext()) {
        considerFreeFactoryInContext(record, functionName, tu, best);
    }

    return best;
}

std::optional<InstancePlan> resolveBaseInstancePlan(const CXXRecordDecl *record,
                                                    const std::string &functionName) {
    if (auto configuredPlan = tryConfiguredInstancePlan(record, functionName)) {
        return configuredPlan;
    }

    if (auto ctorPlan = tryConstructorPlan(record, functionName)) {
        return ctorPlan;
    }

    if (auto staticFactoryPlan = tryStaticFactoryPlan(record, functionName)) {
        return staticFactoryPlan;
    }

    return tryFreeFactoryPlan(record, functionName);
}

void considerOwnerFactoriesInContext(const CXXRecordDecl *targetRecord,
                                     const std::string &functionName,
                                     const DeclContext *context,
                                     std::optional<InstancePlan> &best) {
    if (!targetRecord || !context) {
        return;
    }

    for (const auto *decl : context->decls()) {
        const auto *ownerRecord = dyn_cast<CXXRecordDecl>(decl);
        if (!ownerRecord || ownerRecord == targetRecord) {
            continue;
        }
        ownerRecord = ownerRecord->getDefinition();
        if (!ownerRecord || ownerRecord->isImplicit()) {
            continue;
        }
        if (!hasPublicUsableDestructor(ownerRecord)) {
            continue;
        }

        auto ownerPlan = resolveBaseInstancePlan(ownerRecord, "");
        if (!ownerPlan.has_value()) {
            continue;
        }

        for (const auto *method : ownerRecord->methods()) {
            if (!method || method->isStatic() || method->isDeleted() ||
                method->getAccess() != AS_public) {
                continue;
            }
            const auto subjectKind = compatibleReturnSubject(targetRecord, method);
            if (!subjectKind.has_value()) {
                continue;
            }

            InstancePlan candidate;
            candidate.kind = InstancePlanKind::OwnerFactory;
            candidate.subjectKind = *subjectKind;
            candidate.callableExpr = method->getNameAsString();
            candidate.ownerTypeName = ownerRecord->getQualifiedNameAsString();
            candidate.ownerPlan = std::make_shared<InstancePlan>(*ownerPlan);
            candidate.totalParameters =
                candidate.ownerPlan->totalParameters + method->getNumParams();
            candidate.setupParams = buildRequiredParams(method);
            if (!canMaterializeConstructorParams(candidate.setupParams, functionName)) {
                continue;
            }

            const size_t candidateInputs = candidate.setupParams.size() +
                                           candidate.ownerPlan->setupParams.size();
            const size_t bestInputs =
                best.has_value()
                    ? (best->setupParams.size() +
                       (best->ownerPlan ? best->ownerPlan->setupParams.size() : 0))
                    : 0;
            if (!best.has_value() || candidateInputs < bestInputs ||
                (candidateInputs == bestInputs &&
                 candidate.totalParameters < best->totalParameters)) {
                best.emplace(std::move(candidate));
            }
        }
    }
}

std::optional<InstancePlan> tryOwnerFactoryPlan(const CXXRecordDecl *record,
                                                const std::string &functionName) {
    std::optional<InstancePlan> best;
    considerOwnerFactoriesInContext(record, functionName, record->getDeclContext(), best);

    const auto *tu = record->getASTContext().getTranslationUnitDecl();
    if (!best.has_value() && tu != record->getDeclContext()) {
        considerOwnerFactoriesInContext(record, functionName, tu, best);
    }

    return best;
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

std::optional<InstancePlan> resolveInstancePlan(const CXXRecordDecl *record,
                                                const std::string &functionName) {
    if (!record) {
        return std::nullopt;
    }
    record = record->getDefinition();
    if (!record) {
        return std::nullopt;
    }

    if (auto basePlan = resolveBaseInstancePlan(record, functionName)) {
        return basePlan;
    }

    return tryOwnerFactoryPlan(record, functionName);
}
