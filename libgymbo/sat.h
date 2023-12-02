#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "type.h"

namespace gymbosat {

/**
 * @brief Enum representing different logical operation codes.
 */
typedef enum { VAR, AND, OR, NOT, CONST } OpCode;

/**
 * @brief Enum representing different polarities in logical expressions.
 */
typedef enum { Positive, Negative, Mixed } Polarity;

/**
 * @class Expr
 * @brief Base class for representing logical expressions.
 */
class Expr : public std::enable_shared_from_this<Expr> {
   public:
    OpCode opcode;    /**< Operation code for the expression. */
    std::string name; /**< Name associated with the expression. */

    /**
     * @brief Destructor for the Expr class.
     */
    virtual ~Expr() {}

    /**
     * @brief Convert the logical expression to a string representation.
     * @return String representation of the logical expression.
     */
    virtual std::string to_string() = 0;

    /**
     * @brief Evaluate the logical expression.
     * @return Result of the logical evaluation.
     */
    virtual bool evaluate() = 0;

    /**
     * @brief Check if the logical expression can be simplified by removing
     * constants.
     * @return True if the expression can be simplified, false otherwise.
     */
    virtual bool unConst() = 0;

    /**
     * @brief Identify and return free variables in the logical expression.
     * @return Pair indicating if there are free variables and the first free
     * variable found.
     */
    virtual std::pair<bool, std::string> freeVar() = 0;

    /**
     * @brief Make a guess for a variable's value and return the resulting
     * expression.
     * @param var Variable name.
     * @param val Guessed value for the variable.
     * @return Shared pointer to the resulting logical expression.
     */
    virtual std::shared_ptr<Expr> guessVar(std::string var, bool val) = 0;

    /**
     * @brief Simplify the logical expression.
     * @return Shared pointer to the simplified logical expression.
     */
    virtual std::shared_ptr<Expr> simplify() = 0;

    /**
     * @brief Fix negations in the logical expression.
     * @return Shared pointer to the logical expression with fixed negations.
     */
    virtual std::shared_ptr<Expr> fixNegations() = 0;

    /**
     * @brief Distribute operations in the logical expression.
     * @return Shared pointer to the logical expression with distributed
     * operations.
     */
    virtual std::shared_ptr<Expr> distribute() = 0;

    /**
     * @brief Get the left child expression.
     * @return Shared pointer to the left child expression.
     */
    virtual std::shared_ptr<Expr> getLeft() = 0;

    /**
     * @brief Get the right child expression.
     * @return Shared pointer to the right child expression.
     */
    virtual std::shared_ptr<Expr> getRight() = 0;

    /**
     * @brief Get the literals present in the logical expression.
     * @return Unordered set of literals.
     */
    virtual std::unordered_set<std::string> literals() = 0;

    /**
     * @brief Get the polarities of literals with respect to a specific
     * variable.
     * @param var Variable name.
     * @return Unordered map of literals to their polarities.
     */
    virtual std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) = 0;

    /**
     * @brief Get a unit clause from the logical expression.
     * @return Pair representing a unit clause: (literal, value).
     */
    virtual std::pair<std::string, bool> unitClause() = 0;

    /**
     * @brief Get a vector of clauses present in the logical expression.
     * @return Vector of shared pointers to clause expressions.
     */
    virtual std::vector<std::shared_ptr<Expr>> clauses() = 0;
};

/**
 * @class Var
 * @brief Class representing a variable in a logical expression.
 */
class Var : public Expr {
    std::string name;

   public:
    Var(std::string name) : name(name) { opcode = OpCode::VAR; }
    std::string to_string() override { return name; }
    std::pair<bool, std::string> freeVar() override { return {true, name}; }
    std::shared_ptr<Expr> simplify() override { return shared_from_this(); }
    bool evaluate() override { return false; }
    bool unConst() override { return false; }
    std::shared_ptr<Expr> guessVar(std::string var, bool val) override;

    std::shared_ptr<Expr> fixNegations() override { return shared_from_this(); }
    std::shared_ptr<Expr> distribute() override { return shared_from_this(); }
    std::shared_ptr<Expr> getLeft() override { return std::shared_ptr<Expr>(); }
    std::shared_ptr<Expr> getRight() override {
        return std::shared_ptr<Expr>();
    }

    std::unordered_set<std::string> literals() override { return {name}; }
    std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) override {
        std::unordered_map<std::string, Polarity> polarity_map;
        if (name == var) {
            polarity_map[var] = Polarity::Positive;
        }
        return polarity_map;
    }

    std::pair<std::string, bool> unitClause() override { return {name, true}; }
    std::vector<std::shared_ptr<Expr>> clauses() override {
        return {shared_from_this()};
    }
};

/**
 * @class And
 * @brief Class representing the logical AND operation.
 */
class And : public Expr {
   public:
    And(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right)
        : left(left), right(right) {
        opcode = OpCode::AND;
        name = "And";
    }

    std::string to_string() override {
        return "(" + left->to_string() + " && " + right->to_string() + ")";
    }

    bool evaluate() override { return left->evaluate() && right->evaluate(); }

    std::pair<bool, std::string> freeVar() override {
        std::pair<bool, std::string> leftVar = left->freeVar();
        if (leftVar.first) {
            return leftVar;
        }
        return right->freeVar();
    }

    bool unConst() override { return false; }

    std::shared_ptr<Expr> fixNegations() override {
        return std::make_shared<And>(left->fixNegations(),
                                     right->fixNegations());
    }

    std::shared_ptr<Expr> distribute() override {
        return std::make_shared<And>(left->distribute(), right->distribute());
    }

    std::shared_ptr<Expr> getLeft() override { return left; }
    std::shared_ptr<Expr> getRight() override { return right; }

    std::unordered_set<std::string> literals() override {
        auto leftLiterals = left->literals();
        auto rightLiterals = right->literals();
        leftLiterals.insert(rightLiterals.begin(), rightLiterals.end());
        return leftLiterals;
    }

    std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) override {
        std::unordered_map<std::string, Polarity> polarity_map;

        std::unordered_map<std::string, Polarity> left_polarity =
            left->literalPolarity(var);
        std::unordered_map<std::string, Polarity> right_polarity =
            right->literalPolarity(var);

        for (auto &pair : left_polarity) {
            polarity_map[pair.first] = pair.second;
        }

        for (auto &pair : right_polarity) {
            if (polarity_map.find(pair.first) == polarity_map.end()) {
                polarity_map[pair.first] = pair.second;
            } else if (polarity_map[pair.first] != pair.second) {
                polarity_map[pair.first] = Polarity::Mixed;
            }
        }

        return polarity_map;
    }

    std::pair<std::string, bool> unitClause() override { return {"", false}; }

    std::vector<std::shared_ptr<Expr>> clauses() override {
        auto left_clause = left->clauses();
        auto right_clause = right->clauses();
        left_clause.insert(left_clause.end(), right_clause.begin(),
                           right_clause.end());
        return left_clause;
    }

    std::shared_ptr<Expr> guessVar(std::string var, bool val) override;
    std::shared_ptr<Expr> simplify() override;

   private:
    std::shared_ptr<Expr> left, right;
};

/**
 * @class Or
 * @brief Class representing the logical OR operation.
 */
class Or : public Expr {
   public:
    Or(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right)
        : left(left), right(right) {
        opcode = OpCode::OR;
        name = "Or";
    }

    std::string to_string() override {
        return "(" + left->to_string() + " || " + right->to_string() + ")";
    }

    bool evaluate() override { return left->evaluate() || right->evaluate(); }

    std::pair<bool, std::string> freeVar() override {
        std::pair<bool, std::string> leftVar = left->freeVar();
        if (leftVar.first) {
            return leftVar;
        }
        return right->freeVar();
    }

    bool unConst() override { return false; }

    std::shared_ptr<Expr> fixNegations() override {
        return std::make_shared<Or>(left->fixNegations(),
                                    right->fixNegations());
    }

    std::shared_ptr<Expr> distribute() override {
        if (right->opcode == AND) {
            return std::make_shared<And>(
                std::make_shared<Or>(left->distribute(),
                                     right->getLeft()->distribute()),
                std::make_shared<Or>(left->distribute(),
                                     right->getRight()->distribute()));
        } else if (left->opcode == AND) {
            return std::make_shared<And>(
                std::make_shared<Or>(right->distribute(),
                                     left->getLeft()->distribute()),
                std::make_shared<Or>(right->distribute(),
                                     left->getRight()->distribute()));
        }
        return std::make_shared<Or>(left->distribute(), right->distribute());
    }

    std::shared_ptr<Expr> getLeft() override { return left; }

    std::shared_ptr<Expr> getRight() override { return right; }

    std::unordered_set<std::string> literals() override {
        auto leftLiterals = left->literals();
        auto rightLiterals = right->literals();
        leftLiterals.insert(rightLiterals.begin(), rightLiterals.end());
        return leftLiterals;
    }

    std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) override {
        std::unordered_map<std::string, Polarity> polarity_map;

        std::unordered_map<std::string, Polarity> left_polarity =
            left->literalPolarity(var);
        std::unordered_map<std::string, Polarity> right_polarity =
            right->literalPolarity(var);

        for (auto &pair : left_polarity) {
            polarity_map[pair.first] = pair.second;
        }

        for (auto &pair : right_polarity) {
            if (polarity_map.find(pair.first) == polarity_map.end()) {
                polarity_map[pair.first] = pair.second;
            } else if (polarity_map[pair.first] != pair.second) {
                polarity_map[pair.first] = Polarity::Mixed;
            }
        }

        return polarity_map;
    }

    std::pair<std::string, bool> unitClause() override { return {"", false}; }

    std::vector<std::shared_ptr<Expr>> clauses() override {
        return {shared_from_this()};
    }

    std::shared_ptr<Expr> guessVar(std::string var, bool val) override;
    std::shared_ptr<Expr> simplify() override;

   private:
    std::shared_ptr<Expr> left, right;
};

/**
 * @class Not
 * @brief Class representing the logical NOT operation.
 */
class Not : public Expr {
   public:
    Not(std::shared_ptr<Expr> expr) : expr(expr) {
        opcode = OpCode::NOT;
        name = "Not";
    }

    std::string to_string() override {
        return "(!(" + expr->to_string() + "))";
    }

    bool evaluate() override { return !expr->evaluate(); }

    std::pair<bool, std::string> freeVar() override { return expr->freeVar(); }

    std::shared_ptr<Expr> distribute() override {
        return std::make_shared<Not>(expr->distribute());
    }

    bool unConst() override { return false; }

    std::shared_ptr<Expr> getLeft() override { return std::shared_ptr<Expr>(); }
    std::shared_ptr<Expr> getRight() override {
        return std::shared_ptr<Expr>();
    }

    std::unordered_set<std::string> literals() override {
        return expr->literals();
    }

    std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) override {
        std::unordered_map<std::string, Polarity> polarity_map;
        if (expr->opcode == OpCode::VAR && expr->name == var) {
            polarity_map[expr->name] = Polarity::Negative;
        }
        return polarity_map;
    }

    std::pair<std::string, bool> unitClause() override {
        if (expr->opcode == OpCode::VAR) {
            return {expr->name, false};
        }
        return {"", false};
    }

    std::vector<std::shared_ptr<Expr>> clauses() override {
        return {shared_from_this()};
    }

    std::shared_ptr<Expr> fixNegations() override;
    std::shared_ptr<Expr> guessVar(std::string var, bool val) override;
    std::shared_ptr<Expr> simplify() override;

   private:
    std::shared_ptr<Expr> expr;
};

/**
 * @class Const
 * @brief Class representing a boolean constant in a logical expression.
 */
class Const : public Expr {
   public:
    Const(bool value) : value(value) {
        opcode = OpCode::CONST;
        std::string name = "Const";
    }

    std::string to_string() override { return value ? "True" : "False"; }

    bool evaluate() override { return value; }

    std::pair<bool, std::string> freeVar() override { return {false, ""}; }

    bool unConst() override { return value; }

    std::shared_ptr<Expr> simplify() override { return shared_from_this(); };
    std::shared_ptr<Expr> fixNegations() override { return shared_from_this(); }
    std::shared_ptr<Expr> distribute() override { return shared_from_this(); }
    std::shared_ptr<Expr> getLeft() override { return std::shared_ptr<Expr>(); }
    std::shared_ptr<Expr> getRight() override {
        return std::shared_ptr<Expr>();
    }

    std::unordered_set<std::string> literals() override { return {}; }

    std::shared_ptr<Expr> guessVar(std::string var, bool val) override;

    std::unordered_map<std::string, Polarity> literalPolarity(
        std::string var) override {
        std::unordered_map<std::string, Polarity> polarity_map;
        return polarity_map;
    }

    std::pair<std::string, bool> unitClause() override { return {"", false}; }

    std::vector<std::shared_ptr<Expr>> clauses() override {
        return {shared_from_this()};
    }

   private:
    bool value;
};

inline std::shared_ptr<Expr> Var::guessVar(std::string var, bool val) {
    if (name == var) {
        return std::make_shared<Const>(val);
    } else {
        return std::make_shared<Var>(name);
    }
}

inline std::shared_ptr<Expr> And::guessVar(std::string var, bool val) {
    return std::make_shared<And>(left->guessVar(var, val),
                                 right->guessVar(var, val));
}

inline std::shared_ptr<Expr> Or::guessVar(std::string var, bool val) {
    return std::make_shared<Or>(left->guessVar(var, val),
                                right->guessVar(var, val));
}

inline std::shared_ptr<Expr> Not::guessVar(std::string var, bool val) {
    return std::make_shared<Not>(expr->guessVar(var, val));
}

inline std::shared_ptr<Expr> Const::guessVar(std::string var, bool val) {
    return std::make_shared<Const>(value);
}

inline std::shared_ptr<Expr> And::simplify() {
    auto simplifiedLeft = left->simplify();
    auto simplifiedRight = right->simplify();

    if (simplifiedLeft->to_string() == "True" &&
        simplifiedRight->to_string() == "True") {
        return std::make_shared<Const>(true);
    } else if (simplifiedLeft->to_string() == "False" ||
               simplifiedRight->to_string() == "False") {
        return std::make_shared<Const>(false);
    } else if (simplifiedLeft->to_string() == "True") {
        return simplifiedRight;
    } else if (simplifiedRight->to_string() == "True") {
        return simplifiedLeft;
    } else {
        return std::make_shared<And>(simplifiedLeft, simplifiedRight);
    }
}

inline std::shared_ptr<Expr> Or::simplify() {
    auto simplifiedLeft = left->simplify();
    auto simplifiedRight = right->simplify();

    if (simplifiedLeft->to_string() == "False" &&
        simplifiedRight->to_string() == "False") {
        return std::make_shared<Const>(false);
    } else if (simplifiedLeft->to_string() == "True" ||
               simplifiedRight->to_string() == "True") {
        return std::make_shared<Const>(true);
    } else if (simplifiedLeft->to_string() == "False") {
        return simplifiedRight;
    } else if (simplifiedRight->to_string() == "False") {
        return simplifiedLeft;
    } else {
        return std::make_shared<Or>(simplifiedLeft, simplifiedRight);
    }
}

inline std::shared_ptr<Expr> Not::simplify() {
    auto simplifiedExpr = expr->simplify();

    if (simplifiedExpr->to_string() == "True") {
        return std::make_shared<Const>(false);
    } else if (simplifiedExpr->to_string() == "False") {
        return std::make_shared<Const>(true);
    } else {
        return std::make_shared<Not>(simplifiedExpr);
    }
}

inline std::shared_ptr<Expr> Not::fixNegations() {
    if (expr->to_string() == "True") {
        // !True = False
        return std::make_shared<Const>(false);
    } else if (expr->to_string() == "False") {
        // !False = True
        return std::make_shared<Const>(true);
    } else if (expr->opcode == OpCode::VAR) {
        return std::make_shared<Not>(expr->fixNegations());
    } else if (expr->opcode == OpCode::NOT) {
        // !!Var = Var
        return expr->fixNegations();
    } else if (expr->opcode == OpCode::AND) {
        return std::make_shared<Or>(
            std::make_shared<Not>(expr->getLeft())->fixNegations(),
            std::make_shared<Not>(expr->getRight())->fixNegations());
    } else if (expr->opcode == OpCode::OR) {
        return std::make_shared<And>(
            std::make_shared<Not>(expr->getLeft())->fixNegations(),
            std::make_shared<Not>(expr->getRight())->fixNegations());
    }
    return std::make_shared<Not>(expr->fixNegations());
}

/**
 * @brief Convert a logical expression to conjunctive normal form (CNF).
 * @param expr Shared pointer to the logical expression.
 * @return Shared pointer to the expression in CNF.
 */
inline std::shared_ptr<Expr> cnf(std::shared_ptr<Expr> expr) {
    std::shared_ptr<Expr> new_expr = expr->fixNegations()->distribute();
    if (expr->to_string() == new_expr->to_string()) {
        return expr;
    } else {
        return cnf(new_expr);
    }
}

/**
 * @brief Eliminate literals in a logical expression based on assignments.
 * @param expr Shared pointer to the logical expression.
 * @param assignments_map Reference to an unordered map of variable assignments.
 * @return Shared pointer to the modified logical expression.
 */
inline std::shared_ptr<Expr> literalElimination(
    std::shared_ptr<Expr> expr,
    std::unordered_map<std::string, bool> &assignments_map) {
    std::unordered_set<std::string> literal_set = expr->literals();
    std::unordered_map<std::string, Polarity> polarity_map;
    std::vector<std::pair<std::string, bool>> assignments;

    for (std::string var : literal_set) {
        polarity_map = expr->literalPolarity(var);
        if (polarity_map.find(var) != polarity_map.end()) {
            if (polarity_map[var] == Polarity::Positive) {
                assignments.push_back(std::make_pair(var, true));
            } else if (polarity_map[var] == Polarity::Negative) {
                assignments.push_back(std::make_pair(var, false));
            }
        }
    }

    for (auto &assignment : assignments) {
        std::string var = assignment.first;
        bool value = assignment.second;
        expr = expr->guessVar(var, value);
        if (assignments_map.find(var) == assignments_map.end()) {
            assignments_map.emplace(var, value);
        } else {
            assignments_map[var] = value;
        }
    }

    return expr;
}

/**
 * @brief Extract all unit clauses from a logical expression.
 * @param expr Shared pointer to the logical expression.
 * @return Vector of pairs representing unit clauses: (literal, value).
 */
inline std::vector<std::pair<std::string, bool>> allUnitClauses(
    std::shared_ptr<Expr> expr) {
    std::vector<std::pair<std::string, bool>> result;
    std::vector<std::shared_ptr<Expr>> exprs = expr->clauses();
    for (auto e : exprs) {
        auto uc = e->unitClause();
        if (uc.first != "") {
            result.emplace_back(uc);
        }
    }
    return result;
}

/**
 * @brief Perform unit propagation on a logical expression based on assignments.
 * @param expr Shared pointer to the logical expression.
 * @param assignments_map Reference to an unordered map of variable assignments.
 * @return Shared pointer to the modified logical expression.
 */
inline std::shared_ptr<Expr> unitPropagation(
    std::shared_ptr<Expr> expr,
    std::unordered_map<std::string, bool> &assignments_map) {
    std::vector<std::pair<std::string, bool>> assignments =
        allUnitClauses(expr);
    for (auto &assignment : assignments) {
        std::string var = assignment.first;
        bool value = assignment.second;
        expr = expr->guessVar(var, value);
        if (assignments_map.find(var) == assignments_map.end()) {
            assignments_map.emplace(var, value);
        } else {
            assignments_map[var] = value;
        }
    }

    return expr;
}

/**
 * @brief Check satisfiability of a logical expression using the DPLL algorithm.
 * @param expr Shared pointer to the logical expression.
 * @param assignments_map Reference to an unordered map of variable assignments.
 * @return True if the expression is satisfiable, false otherwise.
 */
inline bool satisfiableDPLL(
    std::shared_ptr<Expr> expr,
    std::unordered_map<std::string, bool> &assignments_map) {
    // std::shared_ptr<Expr> expr2 = literalElimination(
    //     cnf(unitPropagation(expr, assignments_map)), assignments_map);
    std::shared_ptr<Expr> expr2 = cnf(unitPropagation(expr, assignments_map));

    std::pair<bool, std::string> freevar = expr2->freeVar();
    if (!freevar.first) {
        return expr2->simplify()->unConst();
    } else {
        std::string var = freevar.second;

        auto trueGuess = expr2->guessVar(var, true)->simplify();
        std::unordered_map<std::string, bool> true_assignments_map =
            assignments_map;
        if (true_assignments_map.find(var) == true_assignments_map.end()) {
            true_assignments_map.emplace(var, true);
        } else {
            true_assignments_map[var] = true;
        }
        if (satisfiableDPLL(trueGuess, true_assignments_map)) {
            assignments_map = true_assignments_map;
            return true;
        }

        auto falseGuess = expr2->guessVar(var, false)->simplify();
        std::unordered_map<std::string, bool> false_assignments_map =
            assignments_map;
        if (false_assignments_map.find(var) == false_assignments_map.end()) {
            false_assignments_map.emplace(var, false);
        } else {
            false_assignments_map[var] = false;
        }
        if (satisfiableDPLL(falseGuess, false_assignments_map)) {
            assignments_map = false_assignments_map;
            return true;
        }

        return false;
    }
}

/**
 * @brief Convert a symbolic expression to a logical expression.
 * @param sym Pointer to the symbolic expression.
 * @param unique_sym_map Reference to an unordered map of unique symbolic
 * expressions.
 * @return Shared pointer to the corresponding logical expression.
 */
inline std::shared_ptr<Expr> sym2expr(
    gymbo::Sym *sym,
    std::unordered_map<std::string, gymbo::Sym *> &unique_sym_map) {
    switch (sym->symtype) {
        case (gymbo::SymType::SAnd): {
            return std::make_shared<And>(sym2expr(sym->left, unique_sym_map),
                                         sym2expr(sym->right, unique_sym_map));
        }
        case (gymbo::SymType::SNot): {
            return std::make_shared<Not>(sym2expr(sym->left, unique_sym_map));
        }
        case (gymbo::SymType::SOr): {
            return std::make_shared<Or>(sym2expr(sym->left, unique_sym_map),
                                        sym2expr(sym->right, unique_sym_map));
        }
        default: {
            unique_sym_map.insert({sym->toString(true), sym});
            return std::make_shared<Var>(sym->toString(true));
        }
    }
}

/**
 * @brief Convert a vector of symbolic path constraints to a logical expression.
 * @param constraints Vector of symbolic path constraints.
 * @param unique_sym_map Reference to an unordered map of unique symbolic
 * expressions.
 * @return Shared pointer to the logical expression representing the path
 * constraints.
 */
inline std::shared_ptr<Expr> pathconstraints2expr(
    std::vector<gymbo::Sym> &constraints,
    std::unordered_map<std::string, gymbo::Sym *> &unique_sym_map) {
    if (constraints.size() == 0) {
        return std::make_shared<Const>(true);
    } else {
        std::shared_ptr<Expr> res = sym2expr(&constraints[0], unique_sym_map);
        for (int i = 1; i < constraints.size(); i++) {
            res = std::make_shared<And>(
                res, sym2expr(&constraints[i], unique_sym_map));
        }
        return res;
    }
}
}  // namespace gymbosat
