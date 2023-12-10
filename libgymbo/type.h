/**
 * @file type.h
 * @brief Implementatations of basic alias, types, and classes.
 * @author Hideaki Takahashi
 */

#pragma once
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "utils.h"

namespace gymbo {

/**
 * @brief Alias for 32-bit unsigned integer.
 */
using Word32 = uint32_t;

/**
 * @brief Enum representing different instruction types.
 */
enum class InstrType {
    Add,
    Sub,
    Mul,
    JmpIf,
    Jmp,
    And,
    Or,
    Not,
    Lt,
    Le,
    Eq,
    Push,
    Store,
    Load,
    Pop,
    Read,
    Print,
    Swap,
    Dup,
    Over,
    RotL,
    Done,
    Nop,
};

/**
 * @brief Class representing an instruction.
 */
class Instr {
   public:
    InstrType instr;
    Word32 word;

    /**
     * @brief Constructor for an instruction without additional data.
     * @param instr The type of the instruction.
     */
    Instr(InstrType instr) : instr(instr) {}

    /**
     * @brief Constructor for an instruction with additional data.
     * @param instr The type of the instruction.
     * @param word Additional data associated with the instruction.
     */
    Instr(InstrType instr, Word32 word) : instr(instr), word(word) {}

    /**
     * @brief Prints a human-readable representation of the instruction.
     */
    void print() const { printf("%s\n", toString().c_str()); }

    /**
     * @brief Converts the instruction to a string representation.
     * @return A string representation of the instruction.
     */
    std::string toString() const {
        switch (instr) {
            case (InstrType::Add): {
                return "add";
            }
            case (InstrType::Sub): {
                return "sub";
            }
            case (InstrType::Mul): {
                return "mul";
            }
            case (InstrType::And): {
                return "and";
            }
            case (InstrType::Or): {
                return "or";
            }
            case (InstrType::Not): {
                return "not";
            }
            case (InstrType::JmpIf): {
                return "jmpIf";
            }
            case (InstrType::Jmp): {
                return "jmp";
            }
            case (InstrType::Lt): {
                return "lt";
            }
            case (InstrType::Le): {
                return "le";
            }
            case (InstrType::Load): {
                return "load";
            }
            case (InstrType::Read): {
                return "read";
            }
            case (InstrType::Done): {
                return "ret";
            }
            case (InstrType::Nop): {
                return "nop";
            }
            case (InstrType::Swap): {
                return "swap";
            }
            case (InstrType::Store): {
                return "store";
            }
            case (InstrType::Push): {
                return "push " + std::to_string(word);
            }
            default: {
                return "unknown";
            }
        }
    }
};

/**
 * @brief Alias for a program, represented as a vector of instructions.
 */
using Prog = std::vector<Instr>;

/**
 * @brief Alias for memory, represented as an unordered map of 32-bit words.
 */
using Mem = std::unordered_map<Word32, Word32>;

/**
 * @brief Struct representing the gradient of a symbolic expression.
 */
struct Grad {
    std::unordered_map<int, float>
        val; /**< Map of variable indices to gradient values. */

    /**
     * @brief Constructor for the gradient.
     * @param val Map of variable indices to gradient values.
     */
    Grad(std::unordered_map<int, float> val) : val(val) {}

    /**
     * @brief Overloaded addition operator for adding two gradients.
     * @param other The gradient to add.
     * @return The sum of the two gradients.
     */
    Grad operator+(const Grad &other) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            if (other.val.find(r.first) != other.val.end()) {
                result.at(r.first) += other.val.at(r.first);
            }
        }
        for (auto &r : other.val) {
            if (result.find(r.first) == result.end()) {
                result.emplace(std::make_pair(r.first, r.second));
            }
        }
        return Grad(result);
    }

    /**
     * @brief Overloaded addition operator for adding a constant to the
     * gradient.
     * @param w The constant to add.
     * @return The sum of the gradient and the constant.
     */
    Grad operator+(float w) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.at(r.first) += w;
        }
        return Grad(result);
    }

    /**
     * @brief Overloaded subtraction operator for subtracting two gradients.
     * @param other The gradient to subtract.
     * @return The result of subtracting the second gradient from the first.
     */
    Grad operator-(const Grad &other) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            if (other.val.find(r.first) != other.val.end()) {
                result.at(r.first) -= other.val.at(r.first);
            }
        }
        for (auto &r : other.val) {
            if (result.find(r.first) == result.end()) {
                result.emplace(std::make_pair(r.first, -1.0f * r.second));
            }
        }
        return Grad(result);
    }

    /**
     * @brief Overloaded multiplication operator for multiplying the gradient by
     * a constant.
     * @param w The constant to multiply by.
     * @return The result of multiplying the gradient by the constant.
     */
    Grad operator*(float w) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.at(r.first) *= w;
        }
        return Grad(result);
    }

    /**
     * @brief Compute the absolute values of the gradient.
     * @return The gradient with absolute values.
     */
    Grad abs() {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.emplace(std::make_pair(r.first, std::abs(r.second)));
        }
        return Grad(result);
    }
};

/**
 * @brief Enum representing different symbolic expression types.
 */
enum class SymType {
    SAdd,
    SSub,
    SMul,
    SEq,
    SNot,
    SOr,
    SCon,
    SAnd,
    SLt,
    SLe,
    SAny
};

/**
 * @brief Struct representing a symbolic expression.
 */
struct Sym {
    SymType symtype; /**< The type of the symbolic expression. */
    Sym *left;       /**< Pointer to the left child of the expression. */
    Sym *right;      /**< Pointer to the right child of the expression. */
    Word32 word;     /**< Additional data associated with the expression. */
    int var_idx; /**< Index of the variable associated with the expression. */

    /**
     * @brief Default constructor for Sym.
     */
    Sym() {}

    /**
     * @brief Constructor for Sym with a specified type and left child.
     * @param symtype The type of the symbolic expression.
     * @param left Pointer to the left child.
     */
    Sym(SymType symtype, Sym *left) : symtype(symtype), left(left) {}

    /**
     * @brief Constructor for Sym with a specified type, left child, and right
     * child.
     * @param symtype The type of the symbolic expression.
     * @param left Pointer to the left child.
     * @param right Pointer to the right child.
     */
    Sym(SymType symtype, Sym *left, Sym *right)
        : symtype(symtype), left(left), right(right) {}

    /**
     * @brief Constructor for Sym with a specified type and Word32 value.
     * @param symtype The type of the symbolic expression.
     * @param val Word32 value associated with the expression.
     */
    Sym(SymType symtype, Word32 val) : symtype(symtype) {
        if (symtype == SymType::SAny) {
            var_idx = val;
        } else {
            word = val;
        }
    }

    Sym(SymType symtype, Sym *left, Sym *right, Word32 word, int var_idx)
        : symtype(symtype),
          left(left),
          right(right),
          word(word),
          var_idx(var_idx) {}

    Sym *copy() { return new Sym(symtype, left, right, word, var_idx); }

    /**
     * @brief Gathers variable indices from the symbolic expression.
     * @param result Set to store gathered variable indices.
     */
    void gather_var_ids(std::unordered_set<int> &result) const {
        switch (symtype) {
            case (SymType::SAdd): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SSub): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SMul): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SAny): {
                result.emplace(var_idx);
                return;
            }
            case (SymType::SEq): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SNot): {
                left->gather_var_ids(result);
                return;
            }
            case (SymType::SAnd): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SOr): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SLt): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            case (SymType::SLe): {
                left->gather_var_ids(result);
                right->gather_var_ids(result);
                return;
            }
            default:
                return;
        }
    }

    /**
     * @brief Simplifies the symbolic expression by evaluating constant
     * subexpressions.
     * @param cvals Map of variable indices to constant values.
     * @return Simplified symbolic expression.
     */
    Sym *psimplify(const Mem &cvals) {
        Sym *tmp_left, *tmp_right;

        switch (symtype) {
            case (SymType::SAny): {
                if (cvals.find(var_idx) != cvals.end()) {
                    return new Sym(SymType::SCon, cvals.at(var_idx));
                } else {
                    return this;
                }
            }
            case (SymType::SAdd): {
                if (left->symtype == SymType::SCon &&
                    right->symtype == SymType::SCon) {
                    return new Sym(SymType::SCon,
                                   FloatToWord(wordToFloat(left->word) +
                                               wordToFloat(right->word)));
                } else {
                    tmp_left = left->psimplify(cvals);
                    tmp_right = right->psimplify(cvals);
                    if (tmp_left->symtype == SymType::SCon &&
                        tmp_right->symtype == SymType::SCon) {
                        return new Sym(
                            SymType::SCon,
                            FloatToWord(wordToFloat(tmp_left->word) +
                                        wordToFloat(tmp_right->word)));
                    }
                    return new Sym(SymType::SAdd, tmp_left, tmp_right);
                }
            }
            case (SymType::SSub): {
                if (left->symtype == SymType::SCon &&
                    right->symtype == SymType::SCon) {
                    return new Sym(SymType::SCon,
                                   FloatToWord(wordToFloat(left->word) -
                                               wordToFloat(right->word)));
                } else {
                    tmp_left = left->psimplify(cvals);
                    tmp_right = right->psimplify(cvals);
                    if (tmp_left->symtype == SymType::SCon &&
                        tmp_right->symtype == SymType::SCon) {
                        return new Sym(
                            SymType::SCon,
                            FloatToWord(wordToFloat(tmp_left->word) -
                                        wordToFloat(tmp_right->word)));
                    }
                    return new Sym(SymType::SSub, tmp_left, tmp_right);
                }
            }
            case (SymType::SMul): {
                if (left->symtype == SymType::SCon &&
                    right->symtype == SymType::SCon) {
                    return new Sym(SymType::SCon,
                                   FloatToWord(wordToFloat(left->word) *
                                               wordToFloat(right->word)));
                } else {
                    tmp_left = left->psimplify(cvals);
                    tmp_right = right->psimplify(cvals);
                    if (tmp_left->symtype == SymType::SCon &&
                        tmp_right->symtype == SymType::SCon) {
                        return new Sym(
                            SymType::SCon,
                            FloatToWord(wordToFloat(tmp_left->word) *
                                        wordToFloat(tmp_right->word)));
                    }
                    return new Sym(SymType::SMul, tmp_left, tmp_right);
                }
            }
            case (SymType::SEq): {
                return new Sym(SymType::SEq, left->psimplify(cvals),
                               right->psimplify(cvals));
            }
            case (SymType::SAnd): {
                return new Sym(SymType::SAnd, left->psimplify(cvals),
                               right->psimplify(cvals));
            }
            case (SymType::SOr): {
                return new Sym(SymType::SOr, left->psimplify(cvals),
                               right->psimplify(cvals));
            }
            case (SymType::SLt): {
                return new Sym(SymType::SLt, left->psimplify(cvals),
                               right->psimplify(cvals));
            }
            case (SymType::SLe): {
                return new Sym(SymType::SLe, left->psimplify(cvals),
                               right->psimplify(cvals));
            }
            case (SymType::SNot): {
                return new Sym(SymType::SNot, left->psimplify(cvals));
            }
            default: {
                return this;
            }
        }
    }

    /**
     * @brief Evaluates the symbolic expression given concrete variable values.
     * @param cvals Map of variable indices to concrete values.
     * @param eps The smallest positive value of the target type.
     * @return Result of the symbolic expression evaluation.
     */
    float eval(const std::unordered_map<int, float> &cvals, float eps) const {
        switch (symtype) {
            case (SymType::SAdd): {
                return left->eval(cvals, eps) + right->eval(cvals, eps);
            }
            case (SymType::SSub): {
                return left->eval(cvals, eps) - right->eval(cvals, eps);
            }
            case (SymType::SMul): {
                return left->eval(cvals, eps) * right->eval(cvals, eps);
            }
            case (SymType::SCon): {
                return wordToFloat(word);
            }
            case (SymType::SAny): {
                return cvals.at(var_idx);
            }
            case (SymType::SEq): {
                return std::abs(left->eval(cvals, eps) -
                                right->eval(cvals, eps));
            }
            case (SymType::SNot): {
                return left->eval(cvals, eps) * (-1.0f) + eps;
            }
            case (SymType::SAnd): {
                return std::max(0.0f, left->eval(cvals, eps)) +
                       std::max(0.0f, right->eval(cvals, eps));
            }
            case (SymType::SOr): {
                return std::max(0.0f, left->eval(cvals, eps)) *
                       std::max(0.0f, right->eval(cvals, eps));
            }
            case (SymType::SLt): {
                return left->eval(cvals, eps) - right->eval(cvals, eps) + eps;
            }
            case (SymType::SLe): {
                return left->eval(cvals, eps) - right->eval(cvals, eps);
            }
            default: {
                return 0.0f;
            }
        }
    }

    /**
     * @brief Computes the gradient of the symbolic expression given concrete
     * variable values.
     * @param cvals Map of variable indices to concrete values.
     * @param eps Small value to handle numerical instability.
     * @return Gradient of the symbolic expression.
     */
    Grad grad(const std::unordered_map<int, float> &cvals, float eps) const {
        switch (symtype) {
            case (SymType::SAdd): {
                return left->grad(cvals, eps) + right->grad(cvals, eps);
            }
            case (SymType::SSub): {
                return left->grad(cvals, eps) - right->grad(cvals, eps);
            }
            case (SymType::SMul): {
                return left->grad(cvals, eps) * right->eval(cvals, eps) +
                       right->grad(cvals, eps) * left->eval(cvals, eps);
            }
            case (SymType::SCon): {
                return Grad({});
            }
            case (SymType::SAny): {
                std::unordered_map<int, float> tmp = {{var_idx, 1.0f}};
                return Grad(tmp);
            }
            case (SymType::SEq): {
                float lv = left->eval(cvals, eps);
                float rv = right->eval(cvals, eps);
                Grad lg = left->grad(cvals, eps);
                Grad rg = right->grad(cvals, eps);
                if (lv == rv) {
                    return Grad({});
                } else if (lv > rv) {
                    return lg - rg;
                } else {
                    return rg - lg;
                }
            }
            case (SymType::SNot): {
                return left->grad(cvals, eps) * (-1.0f);
            }
            case (SymType::SAnd): {
                float lv = left->eval(cvals, eps);
                float rv = right->eval(cvals, eps);
                Grad res({});
                if (lv > 0.0f) {
                    res = res + left->grad(cvals, eps);
                }
                if (rv > 0.0f) {
                    res = res + right->grad(cvals, eps);
                }
                return res;
            }
            case (SymType::SOr): {
                float lv = left->eval(cvals, eps);
                float rv = right->eval(cvals, eps);
                if (lv > 0.0f && rv > 0.0f) {
                    return left->grad(cvals, eps) * rv +
                           right->grad(cvals, eps) * lv;
                }
                return Grad({});
            }
            case (SymType::SLt): {
                return left->grad(cvals, eps) - right->grad(cvals, eps);
            }
            case (SymType::SLe): {
                return left->grad(cvals, eps) - right->grad(cvals, eps);
            }
            default: {
                return Grad({});
            }
        }
    }

    /**
     * @brief Converts the symbolic expression to a string representation.
     * @param convert_to_num Whether to convert constants to numeric
     * representations.
     * @return String representation of the symbolic expression.
     */
    std::string toString(bool convert_to_num) const {
        std::string result = "";
        float tmp_word;

        switch (symtype) {
            case (SymType::SAdd): {
                result = "(" + left->toString(convert_to_num) + "+" +
                         right->toString(convert_to_num) + ")";
                break;
            }
            case (SymType::SSub): {
                result = "(" + left->toString(convert_to_num) + "-" +
                         right->toString(convert_to_num) + ")";
                break;
            }
            case (SymType::SMul): {
                result = "(" + left->toString(convert_to_num) + "*" +
                         right->toString(convert_to_num) + ")";
                break;
            }
            case (SymType::SCon): {
                if (convert_to_num) {
                    tmp_word = wordToFloat(word);
                    if (is_integer(tmp_word)) {
                        result += std::to_string((int)tmp_word);
                    } else {
                        result += std::to_string(tmp_word);
                    }
                } else {
                    result += std::to_string(word);
                }
                break;
            }
            case (SymType::SAny): {
                result += "var_" + std::to_string(var_idx);
                break;
            }
            case (SymType::SEq): {
                result += left->toString(convert_to_num) +
                          " == " + right->toString(convert_to_num);
                break;
            }
            case (SymType::SNot): {
                result += "!(" + left->toString(convert_to_num) + ")";
                break;
            }
            case (SymType::SAnd): {
                result += left->toString(convert_to_num) + " && " +
                          right->toString(convert_to_num);
                break;
            }
            case (SymType::SOr): {
                result += left->toString(convert_to_num) + " || " +
                          right->toString(convert_to_num);
                break;
            }
            case (SymType::SLt): {
                result += left->toString(convert_to_num) + " < " +
                          right->toString(convert_to_num);
                break;
            }
            case (SymType::SLe): {
                result += left->toString(convert_to_num) +
                          " <= " + right->toString(convert_to_num);
                break;
            }
        }
        return result;
    }
};

/**
 * @brief Alias for symbolic memory, represented as an unordered map of symbolic
 * expressions.
 */
using SMem = std::unordered_map<Word32, Sym>;

struct SymProb {
    Sym numerator;
    Sym denominator;

    SymProb()
        : numerator(Sym(SymType::SCon, FloatToWord(1.0f))),
          denominator(Sym(SymType::SCon, FloatToWord(1.0f))) {}

    SymProb(Sym numerator, Sym denominator)
        : numerator(numerator), denominator(denominator) {}

    SymProb operator*(SymProb &other) {
        if (denominator.toString(false) == other.numerator.toString(false)) {
            return SymProb(numerator, other.denominator);
        } else {
            return SymProb(
                Sym(SymType::SMul, &numerator, &(other.numerator)),
                Sym(SymType::SMul, &denominator, &(other.denominator)));
        }
    }

    Sym query(SymType &symtype, Sym &other) {
        return Sym(symtype, &numerator,
                   new Sym(SymType::SMul, &denominator, &other));
    }
};

/**
 * @brief Struct representing the symbolic state of the symbolic execution.
 */
struct SymState {
    int pc;                         /**< Program counter. */
    int var_cnt;                    /**< Variable count. */
    Mem mem;                        /**< Concrete memory. */
    SMem smem;                      /**< Symbolic memory. */
    Linkedlist<Sym> symbolic_stack; /**< Symbolic stack. */
    std::vector<Sym>
        path_constraints; /**< Vector of symbolic path constraints. */
    SymProb p;            /**< Symbolic probability of the state being reached*/

    /**
     * @brief Default constructor for symbolic state.
     */
    SymState() : pc(0), var_cnt(0), p(SymProb()){};

    /**
     * @brief Constructor for symbolic state with specified values.
     * @param pc Program counter.
     * @param var_cnt Variable count.
     * @param mem Concrete memory.
     * @param smem Symbolic memory.
     * @param symbolic_stack Symbolic stack.
     * @param path_constraints Vector of symbolic path constraints.
     */
    SymState(int pc, int var_cnt, Mem &mem, SMem &smem,
             Linkedlist<Sym> &symbolic_stack,
             std::vector<Sym> &path_constraints)
        : pc(pc),
          var_cnt(var_cnt),
          mem(mem),
          smem(smem),
          symbolic_stack(symbolic_stack),
          path_constraints(path_constraints),
          p(SymProb()) {}


    /**
     * @brief Constructor for symbolic state with specified values.
     * @param pc Program counter.
     * @param var_cnt Variable count.
     * @param mem Concrete memory.
     * @param smem Symbolic memory.
     * @param symbolic_stack Symbolic stack.
     * @param path_constraints Vector of symbolic path constraints.
     */
    SymState(int pc, int var_cnt, Mem &mem, SMem &smem,
             Linkedlist<Sym> &symbolic_stack,
             std::vector<Sym> &path_constraints, SymProb &p)
        : pc(pc),
          var_cnt(var_cnt),
          mem(mem),
          smem(smem),
          symbolic_stack(symbolic_stack),
          path_constraints(path_constraints),
          p(p) {}

    /**
     * @brief Create a copy object.
     */
    SymState *copy() {
        return new SymState(pc, var_cnt, mem, smem, symbolic_stack,
                            path_constraints, p);
    }

    /**
     * @brief Sets a concrete value for a variable in the symbolic state.
     * @param var_id Index of the variable.
     * @param val Concrete value to set.
     */
    void set_concrete_val(int var_id, float val) {
        mem.emplace(var_id, FloatToWord(val));
    }

    /**
     * @brief Returns the human-redable string representation of concrete
     * memory, symbolic memory and path constraints.
     *
     * @param include_memory If set to true, add the concrete and symbolic
     * memories (default true).
     */
    std::string toString(bool include_memory = true) const {
        std::string expr = "";

        if (include_memory) {
            expr += "Concrete Memory: {";
            float tmp_word;
            for (auto &t : mem) {
                tmp_word = wordToFloat(t.second);
                if (is_integer(tmp_word)) {
                    expr += ("var_" + std::to_string(((int)t.first)) + ": " +
                             std::to_string((int)tmp_word)) +
                            ", ";
                } else {
                    expr += ("var_" + std::to_string(((int)t.first)) + ": " +
                             std::to_string(tmp_word)) +
                            ", ";
                }
            }
            expr += "}\n";

            expr += "Symbolic Memory: {";
            for (auto &t : smem) {
                expr += "var_" + std::to_string((int)t.first) + ": " +
                        t.second.toString(true) + ", ";
            }
            expr += "}\n";
        }

        expr += "Path Constraints: ";
        for (const Sym &sym : path_constraints) {
            expr += "(" + sym.toString(true) + ") && ";
        }
        expr += " 1\n";

        return expr;
    }

    /**
     * @brief Prints a human-readable representation of the symbolic state.
     */
    void print() const {
        printf("Stack: [");
        LLNode<Sym> *tmp = symbolic_stack.head;
        while (tmp != NULL) {
            printf("%s, ", tmp->data.toString(false).c_str());
            tmp = tmp->next;
        }
        printf("]\n");

        printf("%s", toString().c_str());
    }
};

/**
 * @brief Alias for a table of path constraints.
 *
 * This table uses the string representation of the constraint as the key.
 * The corresponding value is a pair consisting of:
 *  - The satisfiability of the constraint.
 *  - A map where the key is the variable ID, and the value is the concrete
 * value that makes the constraint true.
 *
 * The overall structure of the table is as follows:
 *   {str_of_constraints: {is_sat: {var_id: var_val}}}
 */
using PathConstraintsTable =
    std::unordered_map<std::string,
                       std::pair<bool, std::unordered_map<int, float>>>;

/**
 * @brief Alias for a table of probabilistic path constraints.
 *
 * This table uses the program counter as the key, and the corresponding values
 * are vectors of tuples. Each tuple contains three elements:
 *  - The first element is the path constraint.
 *  - The second element is the concrete memory.
 *  - The third element is the probability of reachability under satisfying
 * universal variables.
 *
 * The overall structure of the table is as follows:
 *   {pc: {(constraints, memory, probability)}}
 */
using ProbPathConstraintsTable =
    std::unordered_map<int, std::vector<std::tuple<Sym, Mem, float>>>;

/**
 * @brief Struct representing a trace in symbolic execution.
 */
struct Trace {
    SymState data;               /**< Symbolic state of the trace. */
    std::vector<Trace> children; /**< Children traces. */

    /**
     * @brief Constructor for a trace.
     * @param data Symbolic state of the trace.
     * @param children Children traces.
     */
    Trace(SymState data, std::vector<Trace> children)
        : data(data), children(children) {}

    /**
     * @brief Prints a human-readable representation of the trace.
     * @param indent_cnt Number of spaces to indent the output.
     */
    void print(int indent_cnt = 0) const {
        printf("PC: %d\n", data.pc);
        data.print();
        for (const Trace &trace : children) {
            trace.print();
        }
    }
};

/**
 * @struct DiscreteDist
 * @brief Represents a discrete probability distribution.
 */
struct DiscreteDist {
    std::vector<int> vals; /**< Vector to store possible discrete values. */

    /**
     * @brief Default constructor for DiscreteDist.
     */
    DiscreteDist() {}
};

/**
 * @struct DiscreteUniformDist
 * @brief Represents a discrete uniform probability distribution derived from
 * DiscreteDist.
 */
struct DiscreteUniformDist : public DiscreteDist {
    int low;  /**< The lower bound of the uniform distribution. */
    int high; /**< The upper bound of the uniform distribution. */

    /**
     * @brief Constructor for DiscreteUniformDist.
     * @param low The lower bound of the uniform distribution.
     * @param high The upper bound of the uniform distribution.
     * @details Initializes the distribution by populating vals with values from
     * low to high (inclusive).
     */
    DiscreteUniformDist(int low, int high) : low(low), high(high) {
        for (int i = low; i <= high; i++) {
            vals.emplace_back(i);
        }
    }
};

}  // namespace gymbo
