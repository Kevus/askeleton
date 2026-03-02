#include "ASKGen.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

#include "color.h"
#include "framework/BoostGen.hpp"
#include "framework/CatchGen.hpp"
#include "framework/GTestGen.hpp"
#include "Logging.hpp"
#include "utils/ast_values.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

using namespace clang::ast_matchers;
using namespace clang;
using namespace std;
namespace fs = std::filesystem;


ASKGen::ASKGen(bool ruleDataEnabled, unsigned ruleMaxCases, Report *reporter,
               RunStats *stats)
    : ruleDataEnabled(ruleDataEnabled), ruleMaxCases(std::max(1u, ruleMaxCases)),
      reporter(reporter), stats(stats) {}

void printDebugInfo(const vector<InfoVariable> &parameters, const InfoType &returnType) {
    if (Logger::instance().level() < LogLevel::Debug)
        return;

    std::ostringstream ss;
    ss << "Params list: ";
    if (parameters.empty()) {
        ss << "no params";
    } else {
        unsigned i = 1;
        for (const auto &param : parameters) {
            ss << i++ << " - " << param.original << " " << param.name;
            if (&param != &parameters.back())
                ss << ", ";
        }
    }
    ss << "\nReturn type: " << returnType.original;
    Logger::instance().debug(ss.str());
}

namespace {
struct ConstructorSelection {
    std::vector<InfoVariable> params;
    bool useDefaultConstructor = false;
};

std::string buildSignature(const FunctionDecl *decl) {
    if (!decl)
        return "";
    std::ostringstream ss;
    ss << decl->getReturnType().getAsString() << " ";
    ss << decl->getQualifiedNameAsString() << "(";
    bool first = true;
    for (const auto *param : decl->parameters()) {
        if (!first)
            ss << ", ";
        first = false;
        ss << param->getOriginalType().getAsString();
    }
    ss << ")";
    return ss.str();
}

std::string buildSignature(const CXXMethodDecl *decl) {
    if (!decl)
        return "";
    std::ostringstream ss;
    ss << decl->getReturnType().getAsString() << " ";
    ss << decl->getQualifiedNameAsString() << "(";
    bool first = true;
    for (const auto *param : decl->parameters()) {
        if (!first)
            ss << ", ";
        first = false;
        ss << param->getOriginalType().getAsString();
    }
    ss << ")";
    return ss.str();
}

std::string buildSignature(const CXXConstructorDecl *decl) {
    if (!decl)
        return "";
    std::ostringstream ss;
    ss << decl->getQualifiedNameAsString() << "(";
    bool first = true;
    for (const auto *param : decl->parameters()) {
        if (!first)
            ss << ", ";
        first = false;
        ss << param->getOriginalType().getAsString();
    }
    ss << ")";
    return ss.str();
}

std::optional<ConstructorSelection>
selectConstructorForInstantiation(const CXXRecordDecl *record) {
    if (!record) {
        return std::nullopt;
    }

    std::optional<ConstructorSelection> best;
    for (const auto *ctor : record->ctors()) {
        if (!ctor || ctor->isImplicit() || ctor->isDeleted() || ctor->isCopyOrMoveConstructor()) {
            continue;
        }
        if (ctor->getAccess() != AS_public) {
            continue;
        }

        ConstructorSelection candidate;
        candidate.useDefaultConstructor = ctor->isDefaultConstructor();
        bool usable = true;
        for (const auto *param : ctor->parameters()) {
            InfoVariable info(param);
            if (info.isReference()) {
                usable = false;
                break;
            }
            candidate.params.push_back(info);
        }
        if (!usable) {
            continue;
        }

        if (candidate.useDefaultConstructor) {
            return candidate;
        }
        if (!best.has_value() || candidate.params.size() < best->params.size()) {
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
} // namespace

void ASKGen::run(const MatchFinder::MatchResult &Result) {
    apply_FD1(Result);
    apply_MD1(Result);
    apply_CC1(Result);

    if (ruleDataEnabled)
        apply_DG1(Result);
}

void ASKGen::apply_FD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const FunctionDecl *UT = Result.Nodes.getNodeAs<clang::FunctionDecl>("FD1")) {

        FullSourceLoc FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // In this case, we do not want class functions
            if (!isa<CXXMethodDecl>(UT)) {

                // Get the file name
                const std::string rawFilePath =
                    Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
                if (rawFilePath.empty()) {
                    llvm::outs() << ANSI_YELLOW
                                 << "Skipping function with empty source path: "
                                 << UT->getNameInfo().getAsString() << "\n"
                                 << ANSI_RESET;
                    return;
                }
                fs::path filePath = fs::absolute(rawFilePath);
                string fileName = extractFileName(filePath);
                string target = fileName;

                Logger::instance().recordFileSeen(filePath.string());
                Logger::instance().verbose(
                    std::string("Found FunctionDecl: ") +
                    UT->getNameInfo().getAsString() + " in " + fileName);

                auto generator = getGenerator(target, target, filePath, false);
                auto configGenerator = getConfigGenerator(target);
                if (ruleDataEnabled) {
                    collectRuleValuesFromFunction(UT);
                    configGenerator->setRuleValues(ruleValues);
                }

                ReportEntry entry;
                if (reporter) {
                    entry.kind = "function";
                    entry.name = UT->getNameInfo().getAsString();
                    entry.qualified_name = UT->getQualifiedNameAsString();
                    entry.file = filePath.string();
                    entry.line = FullLocation.getSpellingLineNumber();
                    entry.column = FullLocation.getSpellingColumnNumber();
                    entry.target = target;
                    entry.is_class = false;
                    entry.signature = buildSignature(UT);
                }

                try {
                    unsigned cases = generateTest(*generator, *configGenerator, UT);
                    Logger::instance().verbose(
                        std::string("Generated test for function ") +
                        UT->getNameInfo().getAsString());
                    if (reporter) {
                        entry.status = "generated";
                        entry.test_cases = cases;
                        reporter->addEntry(entry);
                    }
                    if (stats) {
                        stats->found++;
                        stats->generated++;
                        stats->by_kind["function"]++;
                        if (!entry.target.empty())
                            stats->by_target[entry.target]++;
                    }
                } catch (const ComplexTypeException &e) {
                    Logger::instance().verbose(
                        std::string("Skipped function ") +
                        UT->getNameInfo().getAsString() + " (complex type: " + e.type +
                        ")");
                    if (reporter) {
                        entry.status = "skipped";
                        entry.reason = "complex_type";
                        entry.detail = e.type;
                        reporter->addEntry(entry);
                    }
                    if (stats) {
                        stats->found++;
                        stats->skipped++;
                        stats->by_kind["function"]++;
                        stats->skipped_by_reason["complex_type"]++;
                        if (!entry.target.empty())
                            stats->by_target[entry.target]++;
                    }
                }

                Logger::instance().debug("--------------");
            }
        }
    }
}

void ASKGen::apply_MD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXMethodDecl *UT = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("MD1")) {

        FullSourceLoc FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // No constructor, destructor or operator
            if (!isa<CXXConstructorDecl>(UT) && !isa<CXXDestructorDecl>(UT) &&
                !UT->isOverloadedOperator()) {
                string source_file =
                    Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
                if (source_file.empty()) {
                    llvm::outs() << ANSI_YELLOW
                                 << "Skipping method with empty source path: "
                                 << UT->getNameInfo().getAsString() << "\n"
                                 << ANSI_RESET;
                    return;
                }
                string parentname = UT->getParent()->getName().str();

                Logger::instance().recordFileSeen(source_file);
                Logger::instance().verbose(
                    std::string("Found CxxMethodDecl: ") +
                    UT->getNameInfo().getAsString() + " from class " + parentname);

                const std::string qualifiedParent =
                    UT->getParent()->getQualifiedNameAsString();
                auto generator =
                    getGenerator(parentname, qualifiedParent, source_file, true);
                auto configGenerator = getConfigGenerator(parentname);
                if (ruleDataEnabled) {
                    collectRuleValuesFromFunction(UT);
                    configGenerator->setRuleValues(ruleValues);
                }

                ReportEntry entry;
                if (reporter) {
                    entry.kind = "method";
                    entry.name = UT->getNameInfo().getAsString();
                    entry.qualified_name = UT->getQualifiedNameAsString();
                    entry.file = source_file;
                    entry.line = FullLocation.getSpellingLineNumber();
                    entry.column = FullLocation.getSpellingColumnNumber();
                    entry.target = parentname;
                    entry.is_class = true;
                    entry.signature = buildSignature(UT);
                }

                try {
                    unsigned cases = generateTest(*generator, *configGenerator, UT);
                    Logger::instance().verbose(
                        std::string("Generated test for method ") +
                        UT->getNameInfo().getAsString());
                    if (reporter) {
                        entry.status = "generated";
                        entry.test_cases = cases;
                        reporter->addEntry(entry);
                    }
                    if (stats) {
                        stats->found++;
                        stats->generated++;
                        stats->by_kind["method"]++;
                        if (!entry.target.empty())
                            stats->by_target[entry.target]++;
                    }
                } catch (const ComplexTypeException &e) {
                    Logger::instance().verbose(
                        std::string("Skipped method ") +
                        UT->getNameInfo().getAsString() + " (complex type: " + e.type +
                        ")");
                    if (reporter) {
                        entry.status = "skipped";
                        entry.reason = "complex_type";
                        entry.detail = e.type;
                        reporter->addEntry(entry);
                    }
                    if (stats) {
                        stats->found++;
                        stats->skipped++;
                        stats->by_kind["method"]++;
                        stats->skipped_by_reason["complex_type"]++;
                        if (!entry.target.empty())
                            stats->by_target[entry.target]++;
                    }
                }

                Logger::instance().debug("--------------");
            }
        }
    }
}


void ASKGen::apply_CC1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXConstructorDecl *UT =
            Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("CC1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            string filePath =
                Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
            if (filePath.empty()) {
                llvm::outs() << ANSI_YELLOW
                             << "Skipping constructor with empty source path: "
                             << UT->getNameInfo().getAsString() << "\n"
                             << ANSI_RESET;
                return;
            }
            string fileName = extractFileName(filePath);
            string target = UT->getParent()->getName().str();

            Logger::instance().recordFileSeen(filePath);
            Logger::instance().verbose(
                std::string("Found CXXConstructorDecl: ") +
                UT->getNameInfo().getAsString() + " from class " + target);

                const std::string qualifiedTarget =
                    UT->getParent()->getQualifiedNameAsString();
                auto generator = getGenerator(target, qualifiedTarget, filePath, true);
            auto configGenerator = getConfigGenerator(target);

            ReportEntry entry;
            if (reporter) {
                entry.kind = "constructor";
                entry.name = UT->getNameInfo().getAsString();
                entry.qualified_name = UT->getQualifiedNameAsString();
                entry.file = filePath;
                entry.line = FullLocation.getSpellingLineNumber();
                entry.column = FullLocation.getSpellingColumnNumber();
                entry.target = target;
                entry.is_class = true;
                entry.signature = buildSignature(UT);
            }

            try {
                unsigned cases = generateTest(*generator, *configGenerator, UT);
                Logger::instance().verbose(
                    std::string("Generated test for constructor ") +
                    UT->getNameInfo().getAsString());
                if (reporter) {
                    entry.status = "generated";
                    entry.test_cases = cases;
                    reporter->addEntry(entry);
                }
                if (stats) {
                    stats->found++;
                    stats->generated++;
                    stats->by_kind["constructor"]++;
                    if (!entry.target.empty())
                        stats->by_target[entry.target]++;
                }
            } catch (const ComplexTypeException &e) {
                Logger::instance().verbose(
                    std::string("Skipped constructor ") +
                    UT->getNameInfo().getAsString() + " (complex type: " + e.type +
                    ")");
                if (reporter) {
                    entry.status = "skipped";
                    entry.reason = "complex_type";
                    entry.detail = e.type;
                    reporter->addEntry(entry);
                }
                if (stats) {
                    stats->found++;
                    stats->skipped++;
                    stats->by_kind["constructor"]++;
                    stats->skipped_by_reason["complex_type"]++;
                    if (!entry.target.empty())
                        stats->by_target[entry.target]++;
                }
            }

            Logger::instance().debug("--------------");
        }
    }
}


void ASKGen::apply_DG1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    const auto *op = Result.Nodes.getNodeAs<clang::BinaryOperator>("DG1");
    const auto *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("DG1b");
    if (!op || !FD)
        return;

    FullSourceLoc FullLocation = Context->getFullLoc(op->getBeginLoc());
    if (!FullLocation.isValid() ||
        Context->getSourceManager().isInSystemHeader(FullLocation)) {
        return;
    }

    const Expr *lhs = op->getLHS()->IgnoreParenImpCasts();
    const Expr *rhs = op->getRHS()->IgnoreParenImpCasts();
    const auto *lhsRef = dyn_cast<DeclRefExpr>(lhs);
    const auto *rhsRef = dyn_cast<DeclRefExpr>(rhs);
    const auto lhsValue = extractIntegerValue(lhs);
    const auto rhsValue = extractIntegerValue(rhs);

    const ParmVarDecl *param = nullptr;
    long long literalValue = 0;
    BinaryOperatorKind opcode = op->getOpcode();

    if (lhsRef && rhsValue.has_value()) {
        param = dyn_cast<ParmVarDecl>(lhsRef->getDecl());
        literalValue = rhsValue.value();
    } else if (rhsRef && lhsValue.has_value()) {
        param = dyn_cast<ParmVarDecl>(rhsRef->getDecl());
        literalValue = lhsValue.value();
        switch (opcode) {
        case BO_LT:
            opcode = BO_GT;
            break;
        case BO_LE:
            opcode = BO_GE;
            break;
        case BO_GT:
            opcode = BO_LT;
            break;
        case BO_GE:
            opcode = BO_LE;
            break;
        default:
            break;
        }
    } else {
        return;
    }

    if (!param)
        return;

    addRuleValues(FD, param, opcode, literalValue);
}

void ASKGen::addRuleValues(const FunctionDecl *FD, const ParmVarDecl *param,
                           BinaryOperatorKind opcode, long long literalValue) {
    std::vector<long long> candidates;
    switch (opcode) {
    case BO_EQ:
        candidates = {literalValue};
        break;
    case BO_NE:
        candidates = {literalValue - 1, literalValue + 1};
        break;
    case BO_GT:
    case BO_GE:
    case BO_LT:
    case BO_LE:
        candidates = {literalValue - 1, literalValue, literalValue + 1};
        break;
    default:
        return;
    }

    addRuleValuesForParamName(FD, param->getName().str(), candidates);
    addRuleValuesForParamName(FD, param->getQualifiedNameAsString(), candidates);
}

void ASKGen::addAssignmentRuleValues(const FunctionDecl *FD,
                                     const ParmVarDecl *param,
                                     long long assignedValue) {
    if (!param)
        return;

    std::vector<long long> candidates = {assignedValue - 1, assignedValue,
                                         assignedValue + 1};
    addRuleValuesForParamName(FD, param->getName().str(), candidates);
    addRuleValuesForParamName(FD, param->getQualifiedNameAsString(), candidates);
}

void ASKGen::addStringRuleValuesForParamName(
    const FunctionDecl *FD, const std::string &paramName,
    const std::vector<std::string> &candidates) {
    if (paramName.empty() || !FD)
        return;

    auto &perFunc = ruleStringValues[FD->getName().str()];
    auto &vals = perFunc[paramName];
    for (const auto &v : candidates) {
        if (std::find(vals.begin(), vals.end(), v) == vals.end())
            vals.push_back(v);
    }
}
void ASKGen::addRuleValuesForParamName(
    const FunctionDecl *FD, const std::string &paramName,
    const std::vector<long long> &candidates) {
    if (paramName.empty() || !FD)
        return;

    auto &perFunc = ruleValues[FD->getName().str()];
    auto &vals = perFunc[paramName];
    for (long long v : candidates) {
        if (std::find(vals.begin(), vals.end(), v) == vals.end())
            vals.push_back(v);
    }
}

void ASKGen::setRuleValuesForParamName(
    const FunctionDecl *FD, const std::string &paramName,
    const std::vector<long long> &candidates) {
    if (paramName.empty() || !FD)
        return;

    auto &perFunc = ruleValues[FD->getName().str()];
    auto &vals = perFunc[paramName];
    vals.clear();
    for (long long v : candidates) {
        if (std::find(vals.begin(), vals.end(), v) == vals.end())
            vals.push_back(v);
    }
}

unsigned ASKGen::computeRuleInvocationLimit(
    const std::map<std::string, std::vector<long long>> &rulesForFunction) const {
    unsigned maxCandidates = 0;
    for (const auto &entry : rulesForFunction) {
        maxCandidates = std::max<unsigned>(maxCandidates, entry.second.size());
    }

    if (maxCandidates == 0)
        return 1;

    return std::min(ruleMaxCases, maxCandidates);
}

void ASKGen::collectRuleValuesFromFunction(const FunctionDecl *FD) {
    if (!FD || !FD->hasBody())
        return;

    ASTContext &context = FD->getASTContext();
    for (const ParmVarDecl *param : FD->parameters()) {
        if (!param || !param->hasDefaultArg())
            continue;
        const Expr *defaultExpr = param->getDefaultArg();
        if (!defaultExpr)
            continue;
        Expr::EvalResult result;
        if (defaultExpr->EvaluateAsInt(result, context)) {
            addAssignmentRuleValues(FD, param, result.Val.getInt().getSExtValue());
        } else if (auto value = extractIntegerValue(defaultExpr)) {
            addAssignmentRuleValues(FD, param, value.value());
        } else if (auto strValue = extractStringLiteral(defaultExpr)) {
            std::string rawValue = strValue.value();
            std::vector<std::string> candidates = {rawValue};
            if (!rawValue.empty()) {
                candidates.push_back(rawValue + "_alt");
            } else {
                candidates.push_back("alt");
            }
            addStringRuleValuesForParamName(FD, param->getName().str(),
                                            candidates);
            addStringRuleValuesForParamName(
                FD, param->getQualifiedNameAsString(), candidates);
        }
    }

    class RuleVisitor : public RecursiveASTVisitor<RuleVisitor> {
    public:
        RuleVisitor(ASKGen &owner, const FunctionDecl *fd, ASTContext &context)
            : owner(owner), fd(fd), context(context) {}

        struct ComparisonInfo {
            const ParmVarDecl *param = nullptr;
            long long literalValue = 0;
            BinaryOperatorKind opcode = BO_EQ;
        };

        std::optional<ComparisonInfo> extractComparison(BinaryOperator *op) const {
            if (!op || !op->isComparisonOp())
                return std::nullopt;

            const Expr *lhs = op->getLHS()->IgnoreParenImpCasts();
            const Expr *rhs = op->getRHS()->IgnoreParenImpCasts();
            const auto *lhsRef = dyn_cast<DeclRefExpr>(lhs);
            const auto *rhsRef = dyn_cast<DeclRefExpr>(rhs);
            const auto lhsValue = extractIntegerValue(lhs);
            const auto rhsValue = extractIntegerValue(rhs);

            ComparisonInfo info;
            info.opcode = op->getOpcode();

            if (auto moduloInfo = extractModuloComparison(lhs, rhs, info.opcode)) {
                return moduloInfo;
            }

            if (lhsRef && rhsValue.has_value()) {
                info.param = dyn_cast<ParmVarDecl>(lhsRef->getDecl());
                info.literalValue = rhsValue.value();
                return info;
            }

            if (rhsRef && lhsValue.has_value()) {
                info.param = dyn_cast<ParmVarDecl>(rhsRef->getDecl());
                info.literalValue = lhsValue.value();
                switch (info.opcode) {
                case BO_LT:
                    info.opcode = BO_GT;
                    break;
                case BO_LE:
                    info.opcode = BO_GE;
                    break;
                case BO_GT:
                    info.opcode = BO_LT;
                    break;
                case BO_GE:
                    info.opcode = BO_LE;
                    break;
                default:
                    break;
                }
                return info;
            }

            return std::nullopt;
        }

        std::optional<ComparisonInfo> extractModuloComparison(
            const Expr *lhs, const Expr *rhs, BinaryOperatorKind opcode) const {
            const auto *lhsOp = dyn_cast<BinaryOperator>(lhs);
            if (!lhsOp || lhsOp->getOpcode() != BO_Rem)
                return std::nullopt;

            const auto *lhsParamRef =
                dyn_cast<DeclRefExpr>(lhsOp->getLHS()->IgnoreParenImpCasts());
            if (!lhsParamRef)
                return std::nullopt;

            const auto divisor = extractIntegerValue(lhsOp->getRHS());
            const auto rhsValue = extractIntegerValue(rhs);
            if (!divisor.has_value() || !rhsValue.has_value())
                return std::nullopt;

            if (rhsValue.value() != 0)
                return std::nullopt;

            const auto *param = dyn_cast<ParmVarDecl>(lhsParamRef->getDecl());
            if (!param)
                return std::nullopt;

            std::vector<long long> candidates;
            if (opcode == BO_EQ) {
                candidates = {0, divisor.value(), divisor.value() * 2};
            } else if (opcode == BO_NE) {
                candidates = {1, divisor.value() + 1};
            } else {
                return std::nullopt;
            }

            owner.addRuleValuesForParamName(fd, param->getName().str(), candidates);
            owner.addRuleValuesForParamName(fd, param->getQualifiedNameAsString(),
                                            candidates);
            return std::nullopt;
        }

        bool TraverseBinaryOperator(BinaryOperator *op) {
            if (!op)
                return true;

            if (op->getOpcode() == BO_LAnd || op->getOpcode() == BO_LOr) {
                handleLogicalOperator(op);
                return true;
            }

            return RecursiveASTVisitor<RuleVisitor>::TraverseBinaryOperator(op);
        }

        bool VisitBinaryOperator(BinaryOperator *op) {
            if (!op || !op->isComparisonOp())
                return handleNonComparison(op);

            auto comp = extractComparison(op);
            if (comp && comp->param) {
                owner.addRuleValues(fd, comp->param, comp->opcode,
                                    comp->literalValue);
            }
            return true;
        }

        bool handleNonComparison(BinaryOperator *op) {
            if (!op)
                return true;

            if (op->isAssignmentOp()) {
                const Expr *lhs = op->getLHS()->IgnoreParenImpCasts();
                const Expr *rhs = op->getRHS()->IgnoreParenImpCasts();
                const auto *lhsRef = dyn_cast<DeclRefExpr>(lhs);
                if (!lhsRef)
                    return true;

                const auto *param = dyn_cast<ParmVarDecl>(lhsRef->getDecl());
                if (!param)
                    return true;

                Expr::EvalResult result;
                if (rhs && rhs->EvaluateAsInt(result, context)) {
                    owner.addAssignmentRuleValues(
                        fd, param, result.Val.getInt().getSExtValue());
                } else if (auto rhsValue = extractIntegerValue(rhs)) {
                    owner.addAssignmentRuleValues(fd, param, rhsValue.value());
                }

                return true;
            }

            if (op->getOpcode() == BO_Div || op->getOpcode() == BO_Rem) {
                const Expr *rhs = op->getRHS()->IgnoreParenImpCasts();
                const auto *rhsRef = dyn_cast<DeclRefExpr>(rhs);
                if (!rhsRef)
                    return true;

                const auto *param = dyn_cast<ParmVarDecl>(rhsRef->getDecl());
                if (!param)
                    return true;

                std::vector<long long> candidates = {1, 2, -1};
                owner.addRuleValuesForParamName(fd, param->getName().str(),
                                                candidates);
                owner.addRuleValuesForParamName(
                    fd, param->getQualifiedNameAsString(), candidates);
            }

            return true;
        }

        void handleLogicalOperator(BinaryOperator *op) {
            auto *lhsOp = dyn_cast<BinaryOperator>(op->getLHS()->IgnoreParenImpCasts());
            auto *rhsOp = dyn_cast<BinaryOperator>(op->getRHS()->IgnoreParenImpCasts());
            auto lhsComp = extractComparison(lhsOp);
            auto rhsComp = extractComparison(rhsOp);
            if (!lhsComp || !rhsComp || lhsComp->param != rhsComp->param ||
                !lhsComp->param) {
                return;
            }

            if (op->getOpcode() == BO_LOr) {
                owner.addRuleValues(fd, lhsComp->param, lhsComp->opcode,
                                    lhsComp->literalValue);
                owner.addRuleValues(fd, rhsComp->param, rhsComp->opcode,
                                    rhsComp->literalValue);
                return;
            }

            std::optional<long long> lowerBound;
            std::optional<long long> upperBound;

            auto applyBound = [&](const ComparisonInfo &info) {
                switch (info.opcode) {
                case BO_GT:
                    if (!lowerBound || *lowerBound < info.literalValue + 1)
                        lowerBound = info.literalValue + 1;
                    break;
                case BO_GE:
                    if (!lowerBound || *lowerBound < info.literalValue)
                        lowerBound = info.literalValue;
                    break;
                case BO_LT:
                    if (!upperBound || *upperBound > info.literalValue - 1)
                        upperBound = info.literalValue - 1;
                    break;
                case BO_LE:
                    if (!upperBound || *upperBound > info.literalValue)
                        upperBound = info.literalValue;
                    break;
                default:
                    break;
                }
            };

            applyBound(*lhsComp);
            applyBound(*rhsComp);

            if (lowerBound && upperBound && *lowerBound <= *upperBound) {
                std::vector<long long> candidates;
                candidates.push_back(*lowerBound);
                if (*lowerBound + 1 <= *upperBound)
                    candidates.push_back(*lowerBound + 1);
                if (*upperBound >= *lowerBound) {
                    long long highEdge = *upperBound;
                    if (std::find(candidates.begin(), candidates.end(), highEdge) ==
                        candidates.end())
                        candidates.push_back(highEdge);
                    long long nearHigh = highEdge - 1;
                    if (nearHigh >= *lowerBound &&
                        std::find(candidates.begin(), candidates.end(), nearHigh) ==
                            candidates.end())
                        candidates.push_back(nearHigh);
                }

                owner.setRuleValuesForParamName(
                    fd, lhsComp->param->getName().str(), candidates);
                owner.setRuleValuesForParamName(
                    fd, lhsComp->param->getQualifiedNameAsString(), candidates);
            } else {
                owner.addRuleValues(fd, lhsComp->param, lhsComp->opcode,
                                    lhsComp->literalValue);
                owner.addRuleValues(fd, rhsComp->param, rhsComp->opcode,
                                    rhsComp->literalValue);
            }
        }

        bool VisitSwitchStmt(SwitchStmt *stmt) {
            if (!stmt)
                return true;

            const Expr *cond = stmt->getCond();
            if (!cond)
                return true;

            const Expr *condExpr = cond->IgnoreParenImpCasts();
            const auto *condRef = dyn_cast<DeclRefExpr>(condExpr);
            if (!condRef)
                return true;

            const auto *param = dyn_cast<ParmVarDecl>(condRef->getDecl());
            if (!param)
                return true;

            std::vector<long long> candidates;
            long long maxValue = 0;
            bool hasValue = false;

            for (SwitchCase *caseStmt = stmt->getSwitchCaseList(); caseStmt;
                 caseStmt = caseStmt->getNextSwitchCase()) {
                if (auto *caseClause = dyn_cast<CaseStmt>(caseStmt)) {
                    if (auto value = extractIntegerValue(caseClause->getLHS())) {
                        candidates.push_back(*value);
                        if (!hasValue || *value > maxValue)
                            maxValue = *value;
                        hasValue = true;
                    }
                    if (caseClause->getRHS()) {
                        if (auto rhsValue =
                                extractIntegerValue(caseClause->getRHS())) {
                            candidates.push_back(*rhsValue);
                            if (!hasValue || *rhsValue > maxValue)
                                maxValue = *rhsValue;
                            hasValue = true;
                        }
                    }
                }
            }

            if (!candidates.empty()) {
                std::reverse(candidates.begin(), candidates.end());
                if (hasValue)
                    candidates.push_back(maxValue + 1);
                owner.addRuleValuesForParamName(
                    fd, param->getName().str(), candidates);
                owner.addRuleValuesForParamName(
                    fd, param->getQualifiedNameAsString(), candidates);
            }

            return true;
        }

    private:
        ASKGen &owner;
        const FunctionDecl *fd;
        ASTContext &context;
    };

    RuleVisitor visitor(*this, FD, context);
    visitor.TraverseStmt(FD->getBody());
}

// void ASKGen::apply_DG2(const MatchFinder::MatchResult &Result) {
//     ASTContext *Context = Result.Context;

//     if (const SwitchStmt *UT =
//             Result.Nodes.getNodeAs<clang::SwitchStmt>("DG2")) {

//         const FunctionDecl *FD =
//             Result.Nodes.getNodeAs<clang::FunctionDecl>("DG2b");
//         FullSourceLoc FullLocation;

//         FullLocation = Context->getFullLoc(UT->getBeginLoc());

//         if (FullLocation.isValid() &&
//             !Context->getSourceManager().isInSystemHeader(FullLocation)) {

//             string source_file = Context->getSourceManager()
//                                      .getFilename(UT->getBeginLoc())
//                                      .str();
//             unsigned first = source_file.find_last_of('/') + 1;
//             unsigned last = source_file.find_last_of('.');
//             string filename = source_file.substr(first, last - first);

//             // string test = UT->getCond()->getAsString();
//             // llvm::outs() << "test: " << test << "\n";
//             // string cname = UT->getCond()->getName().str();
//             // string ctype = UT->getCond()->getType().getAsString();

//             // llvm::outs() << /*"Cname: " << cname << */" Ctype: " << ctype
//             <<
//             // "\n";

//             /*string LHS_string = convertExpressionToString(UT->getLHS(),
//             Context->getSourceManager()); string RHS_string =
//             convertExpressionToString(UT->getRHS(),
//             Context->getSourceManager()); string LHS_type =
//             UT->getLHS()->getType().getAsString(); string RHS_type =
//             UT->getRHS()->getType().getAsString();

//             //llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " -
//             RHS: " << RHS_string << " " << RHS_type << "\n";

//             string source_file =
//             Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
//             unsigned first = source_file.find_last_of('/') + 1;
//             unsigned last = source_file.find_last_of('.');
//             string filename = source_file.substr(first, last-first);

//             string type;
//             if(!isNumeric(LHS_string) && isNumeric(RHS_string) &&
//             isInParameters(LHS_string, FD->parameters(), type))
//             {
//                 generateTestData(filename, FD->getName().str(), LHS_string,
//             type, RHS_string); } else if (isNumeric(LHS_string) &&
//             !isNumeric(RHS_string) && isInParameters(RHS_string,
//             FD->parameters(), type))
//             {
//                 generateTestData(filename, FD->getName().str(), RHS_string,
//             type, LHS_string); } else
//             {
//                 llvm::outs() << "non-numeric condition\n";
//             }*/

//             // Print auxiliary
//             //
//             ======================================================================
//             llvm::outs() << "Found SwitchStmt at "
//                          << FullLocation.getSpellingLineNumber() << ":"
//                          << FullLocation.getSpellingColumnNumber() << " - ";

//             llvm::outs() << " from function " << FD->getName().str() << "\n";
//             // Print auxiliary
//             //
//             ======================================================================
//         }
//     }
// }

void ASKGen::generateReadMethod(Generator &testGen,
                                const std::vector<InfoVariable> &variables) {
    for (const auto &variable : variables) {
        generateReadMethod(testGen, variable);
    }
}

void ASKGen::generateReadMethod(Generator &testGen, const std::vector<InfoType> &types) {
    for (const auto &type : types) {
        generateReadMethod(testGen, type);
    }
}

void ASKGen::generateReadMethod(Generator &testGen, const InfoVariable &variable) {
    testGen.createTypeReadToFixture(variable);
}

void ASKGen::generateReadMethod(Generator &testGen, const InfoType &type) {
    testGen.createTypeReadToFixture(type);
}

std::shared_ptr<Generator> ASKGen::getGenerator(const std::string &target,
                                                const std::string &targetQualifiedName,
                                                const std::string &filePath,
                                                bool isFromClass) {
    auto pos = generators.find(target);

    if (pos == generators.end()) {
        std::shared_ptr<Generator> generator;

        switch (getFramework()) {
        case CATCH:
            generator = std::make_shared<CatchGenerator>(
                target, targetQualifiedName, filePath, isFromClass);
            break;
        case GTEST:
            generator = std::make_shared<GTestGenerator>(
                target, targetQualifiedName, filePath, isFromClass);
            break;
        default:
            generator = std::make_shared<BoostGen>(
                target, targetQualifiedName, filePath, isFromClass);
            break;
        }
        generators.insert({target, generator});
        return generator;
    }

    return pos->second;
}

std::shared_ptr<ConfigGenerator> ASKGen::getConfigGenerator(const std::string &target) {
    auto pos = configGenerators.find(target);

    if (pos == configGenerators.end()) {
        auto configGenerator = std::make_shared<ConfigGenerator>(target);
        configGenerators.insert({target, configGenerator});
        return configGenerator;
    }

    return pos->second;
}

std::vector<InfoVariable>
ASKGen::getParameters(const std::vector<ParmVarDecl *> &params) {
    std::vector<InfoVariable> parameters;
    std::transform(params.begin(), params.end(), std::back_inserter(parameters),
                   [](const ParmVarDecl *param) { return param; });
    return parameters;
}

unsigned ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                              const FunctionDecl *UT) {
    unsigned ruleInvocations = 1;
    if (ruleDataEnabled) {
        collectRuleValuesFromFunction(UT);
        auto it = ruleValues.find(UT->getName().str());
        if (it != ruleValues.end()) {
            const auto &rulesForFunction = it->second;
            testGen.setRuleValues({{UT->getName().str(), rulesForFunction}});
            configGenerator.setRuleValues({{UT->getName().str(), rulesForFunction}});
            ruleInvocations = computeRuleInvocationLimit(rulesForFunction);
        }
        auto strIt = ruleStringValues.find(UT->getName().str());
        if (strIt != ruleStringValues.end()) {
            testGen.setStringRuleValues({{UT->getName().str(), strIt->second}});
            configGenerator.setStringRuleValues(
                {{UT->getName().str(), strIt->second}});
        }
    }
    InfoType returnType(UT->getReturnType());
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));

    std::string functionName = UT->getName().str();
    if (function_occurrences[functionName]++ > 1) {
        functionName += "_" + std::to_string(function_occurrences[functionName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, returnType);
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);
    generateReadMethod(testGen, returnType);

    for (unsigned i = 0; i < ruleInvocations; ++i) {
        testGen.generateFunctionAssert(functionName, parameters, returnType);
    }
    return ruleInvocations;
}

unsigned ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                              const CXXMethodDecl *UT) {
    unsigned ruleInvocations = 1;
    if (ruleDataEnabled) {
        collectRuleValuesFromFunction(UT);
        auto it = ruleValues.find(UT->getName().str());
        if (it != ruleValues.end()) {
            const auto &rulesForFunction = it->second;
            testGen.setRuleValues({{UT->getName().str(), rulesForFunction}});
            configGenerator.setRuleValues({{UT->getName().str(), rulesForFunction}});
            ruleInvocations = computeRuleInvocationLimit(rulesForFunction);
        }
        auto strIt = ruleStringValues.find(UT->getName().str());
        if (strIt != ruleStringValues.end()) {
            testGen.setStringRuleValues({{UT->getName().str(), strIt->second}});
            configGenerator.setStringRuleValues(
                {{UT->getName().str(), strIt->second}});
        }
    }
    InfoType returnType(UT->getReturnType());
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));
    bool isStatic = UT->isStatic();

    if (!isStatic) {
        auto ctorSelection = selectConstructorForInstantiation(UT->getParent());
        if (!ctorSelection.has_value() ||
            !testGen.setInstanceConstruction(ctorSelection->params,
                                             ctorSelection->useDefaultConstructor)) {
            throw ComplexTypeException(UT->getParent()->getQualifiedNameAsString());
        }
    }

    std::string functionName = UT->getNameInfo().getAsString();
    if (function_occurrences[functionName]++ > 1) {
        functionName += "_" + std::to_string(function_occurrences[functionName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, returnType);
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);
    generateReadMethod(testGen, returnType);

    for (unsigned i = 0; i < ruleInvocations; ++i) {
        testGen.generateMethodAssert(functionName, parameters, returnType, isStatic);
    }
    return ruleInvocations;
}

unsigned ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                              const CXXConstructorDecl *UT) {
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));
    std::string constructorName = UT->getParent()->getName().str();

    if (function_occurrences[constructorName]++ > 1) {
        constructorName += "_" + std::to_string(function_occurrences[constructorName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, {"no return"});
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);

    testGen.generateConstructorAssert(parameters);
    return 1;
}


ASKGen::~ASKGen() {
    if (function_occurrences.empty()) {
        Logger::instance().info(
            "ASkeleTon has not found any function to generate tests for");
        remove_all(getAskeletonHome() / getConfig()["route"]["generated"]);
    }
}
