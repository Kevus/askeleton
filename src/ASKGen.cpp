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

namespace {
std::optional<long long> extractIntegerValue(const Expr *expr) {
    if (!expr)
        return std::nullopt;

    const Expr *cleanExpr = expr->IgnoreParenImpCasts();
    if (const auto *intLiteral = dyn_cast<IntegerLiteral>(cleanExpr)) {
        return intLiteral->getValue().getSExtValue();
    }

    if (const auto *boolLiteral = dyn_cast<CXXBoolLiteralExpr>(cleanExpr)) {
        return boolLiteral->getValue() ? 1 : 0;
    }

    if (const auto *declRef = dyn_cast<DeclRefExpr>(cleanExpr)) {
        if (const auto *enumConstant = dyn_cast<EnumConstantDecl>(declRef->getDecl())) {
            return enumConstant->getInitVal().getSExtValue();
        }

        if (const auto *varDecl = dyn_cast<VarDecl>(declRef->getDecl())) {
            if (varDecl->hasInit()) {
                return extractIntegerValue(varDecl->getInit());
            }
        }
    }

    if (const auto *unary = dyn_cast<UnaryOperator>(cleanExpr)) {
        if (unary->getOpcode() == UO_Minus) {
            const Expr *subExpr = unary->getSubExpr()->IgnoreParenImpCasts();
            if (const auto *subInt = dyn_cast<IntegerLiteral>(subExpr)) {
                return -subInt->getValue().getSExtValue();
            }
        }
    }

    return std::nullopt;
}
} // namespace

ASKGen::ASKGen(bool ruleDataEnabled, unsigned ruleMaxCases)
    : ruleDataEnabled(ruleDataEnabled), ruleMaxCases(std::max(1u, ruleMaxCases)) {}

void printDebugInfo(const vector<InfoVariable> &parameters, const InfoType &returnType) {
    cout << "Params list: ";
    if (parameters.empty()) {
        cout << "no params";
    } else {
        unsigned i = 1;
        for (const auto &param : parameters) {
            cout << i++ << " - " << param.original << " " << param.name;
            if (&param != &parameters.back())
                cout << ", ";
        }
    }

    cout << "\nReturn type: " << returnType.original << "\n";
}

void ASKGen::run(const MatchFinder::MatchResult &Result) {
    apply_FD1(Result);
    apply_MD1(Result);
    // apply_CT1(Result); // Necessary for structs and classes
    apply_CC1(Result);

    if (ruleDataEnabled)
        apply_DG1(Result);
    // apply_DG2(Result);
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
                fs::path filePath = fs::absolute(
                    Context->getSourceManager().getFilename(UT->getBeginLoc()).str());
                string fileName = extractFileName(filePath);
                string target = fileName;

                // Print auxiliary
                // ======================================================================
                llvm::outs() << ANSI_BLUE << "Found FunctionDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString() << " in file " << fileName
                             << "\n"
                             << ANSI_RESET;
                // Print auxiliary
                // ======================================================================

                auto generator = getGenerator(target, filePath, false);
                auto configGenerator = getConfigGenerator(target);
                if (ruleDataEnabled) {
                    collectRuleValuesFromFunction(UT);
                    configGenerator->setRuleValues(ruleValues);
                }

                try {
                    generateTest(*generator, *configGenerator, UT);
                    llvm::outs()
                        << ANSI_GREEN << "Test for function "
                        << UT->getNameInfo().getAsString() << " generated successfully\n"
                        << ANSI_RESET;
                } catch (const ComplexTypeException &e) {
                    llvm::outs() << ANSI_MAGENTA << "Skipping test for function "
                                 << UT->getNameInfo().getAsString()
                                 << " due to complex type: " << e.type << "\n"
                                 << ANSI_RESET;
                }

                llvm::outs() << "--------------\n";
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
                string parentname = UT->getParent()->getName().str();

                // Print auxiliary
                // ======================================================================
                llvm::outs() << ANSI_BLUE << "Found CxxMethodDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString() << " from class "
                             << parentname << "\n"
                             << ANSI_RESET;
                // Print auxiliary
                // ======================================================================

                auto generator = getGenerator(parentname, source_file, true);
                auto configGenerator = getConfigGenerator(parentname);
                if (ruleDataEnabled) {
                    collectRuleValuesFromFunction(UT);
                    configGenerator->setRuleValues(ruleValues);
                }

                try {
                    generateTest(*generator, *configGenerator, UT);
                    llvm::outs()
                        << ANSI_GREEN << "Test for method "
                        << UT->getNameInfo().getAsString() << " generated successfully\n"
                        << ANSI_RESET;
                } catch (const ComplexTypeException &e) {
                    llvm::outs() << ANSI_MAGENTA << "Skipping test for method "
                                 << UT->getNameInfo().getAsString()
                                 << " due to complex type: " << e.type << "\n"
                                 << ANSI_RESET;
                }

                llvm::outs() << "--------------\n";
            }
        }
    }
}

void ASKGen::apply_CT1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXRecordDecl *UT = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("CT1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // Get the file name
            string source_file =
                Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');

            string filename = source_file.substr(first, last - first);

            InfoType record(QualType(UT->getTypeForDecl(), 0));
            auto generator = getGenerator(filename, source_file, true);

            try {
                generateReadMethod(*generator, record);
                llvm::outs() << "Read method for class " << UT->getName().str()
                             << " generated successfully\n";
            } catch (const ComplexTypeException &e) {
                llvm::outs() << "Skipping read method for class " << UT->getName().str()
                             << " due to complex type: " << e.type << "\n";
            }

            // Print auxiliary
            // ======================================================================
            llvm::outs() << "Found CXXRecordDecl (struct-customtype) at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << record.original << " in file " << filename << "\n";
            // Print auxiliary
            // ======================================================================
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
            string fileName = extractFileName(filePath);
            string target = UT->getParent()->getName().str();

            // Print auxiliary
            // ======================================================================
            llvm::outs() << ANSI_BLUE << "Found CXXConstructorDecl at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << UT->getNameInfo().getAsString() << " from class " << target
                         << "\n"
                         << ANSI_RESET;
            // Print auxiliary
            // ======================================================================

            auto generator = getGenerator(target, filePath, true);
            auto configGenerator = getConfigGenerator(target);

            try {
                generateTest(*generator, *configGenerator, UT);
                llvm::outs() << ANSI_GREEN << "Test for method "
                             << UT->getNameInfo().getAsString()
                             << " generated successfully\n";
            } catch (const ComplexTypeException &e) {
                llvm::outs() << ANSI_MAGENTA << "Skipping test for method "
                             << UT->getNameInfo().getAsString()
                             << " due to complex type: " << e.type << "\n";
            }

            llvm::outs() << ANSI_RESET << "--------------\n";
        }
    }
}

void ASKGen::apply_PD1(const MatchFinder::MatchResult &Result) {
    // TO-DO: make this SHOW when a private member is called
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
        } else if (const auto *literal =
                       dyn_cast<StringLiteral>(defaultExpr->IgnoreParenImpCasts())) {
            std::string rawValue = literal->getString().str();
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
                                                const std::string &filePath,
                                                bool isFromClass) {
    auto pos = generators.find(target);

    if (pos == generators.end()) {
        std::shared_ptr<Generator> generator;

        switch (getFramework()) {
        case CATCH:
            generator = std::make_shared<CatchGenerator>(target, filePath, isFromClass);
            break;
        case GTEST:
            generator = std::make_shared<GTestGenerator>(target, filePath, isFromClass);
            break;
        default:
            generator = std::make_shared<BoostGen>(target, filePath, isFromClass);
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

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
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
}

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
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
}

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
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
}

void ASKGen::generateTestData(string source, string function_name, string param,
                              string type, string value) {
    // function.value=a,b,c,d,e
    map<string, vector<string>> function_values;
    string outputPath = "Generated/UT/" + source + "/" + source + "_data.txt";
    vector<string> values;
    string fileContent;

    ifstream tmp_output(outputPath);
    for (string line; getline(tmp_output, line);) {
        // for each line...
        auto delimiter = line.find(":");

        string fvalue = line.substr(0, delimiter);
        line = line.substr(delimiter + 1);

        delimiter = line.find(",");
        while (delimiter != string::npos) {
            auto key = line.substr(0, delimiter);
            line = line.substr(delimiter + 1);

            values.push_back(key);

            delimiter = line.find(",");
        }

        values.push_back(line);
        function_values.insert(std::pair<string, vector<string>>(fvalue, values));
        values.clear();
    }

    if (function_values.find(function_name + "." + param) != function_values.end()) {
        values = function_values.at(function_name + "." + param);
    }

    vector<string> realvalues = obtainTestData(type, value);
    for (auto it : realvalues)
        values.push_back(it);

    function_values[(function_name + "." + param)] = values;

    stringstream ss;

    for (auto it : function_values) {
        ss << it.first << ":";

        for (unsigned long i = 0; i < it.second.size(); i++) {
            ss << it.second[i];
            if (i < it.second.size() - 1)
                ss << ",";
        }

        ss << "\n";
    }

    ofstream outputFile(outputPath);
    outputFile << ss.str();
}

vector<string> ASKGen::obtainTestData(string type, string value) {
    vector<string> result;
    replaceAll(value, "\'", "");
    replaceAll(value, "\"", "");
    result.push_back(value);

    if (type.find("bool") != string::npos || type.find("_Bool") != string::npos) {
        result.push_back((value == "true") ? "false" : "true");
    } else if (type.find("string") != string::npos) {
        result.push_back(value + "_another");
    } else if (type.find("char") != string::npos) {
        char res = value.c_str()[0];
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("int") != string::npos) {
        int res = stoi(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("double") != string::npos) {
        double res = stod(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("float") != string::npos) {
        float res = stof(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    }

    return result;
}

ASKGen::~ASKGen() {
    if (function_occurrences.empty()) {
        llvm::outs() << "ASkeleTon has not found any function to generate tests for\n";
        remove_all(getAskeletonHome() / getConfig()["route"]["generated"]);
    }
}
