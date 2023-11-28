#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "utils.h"

namespace gymbo {

using Word32 = uint32_t;

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

class Instr {
   public:
    InstrType instr;
    Word32 word;

    Instr(InstrType instr) : instr(instr) {}
    Instr(InstrType instr, Word32 word) : instr(instr), word(word) {}

    void print() {
        switch (instr) {
            case (InstrType::Add): {
                printf("add\n");
                return;
            }
            case (InstrType::Sub): {
                printf("sub\n");
                return;
            }
            case (InstrType::Mul): {
                printf("mul\n");
                return;
            }
            case (InstrType::And): {
                printf("and\n");
                return;
            }
            case (InstrType::Or): {
                printf("or\n");
                return;
            }
            case (InstrType::Not): {
                printf("not\n");
                return;
            }
            case (InstrType::JmpIf): {
                printf("jmpIf\n");
                return;
            }
            case (InstrType::Jmp): {
                printf("jmp\n");
                return;
            }
            case (InstrType::Lt): {
                printf("lt\n");
                return;
            }
            case (InstrType::Le): {
                printf("le\n");
                return;
            }
            case (InstrType::Load): {
                printf("load\n");
                return;
            }
            case (InstrType::Read): {
                printf("read\n");
                return;
            }
            case (InstrType::Done): {
                printf("ret\n");
                return;
            }
            case (InstrType::Nop): {
                printf("nop\n");
                return;
            }
            case (InstrType::Swap): {
                printf("swap\n");
                return;
            }
            case (InstrType::Store): {
                printf("store\n");
                return;
            }
            case (InstrType::Push): {
                printf("push %d\n", word);
                return;
            }
            default: {
                printf("unknown\n");
            }
        }
    }
};

using Prog = std::vector<Instr>;
using Mem = std::unordered_map<Word32, Word32>;
using PathConstraintsTable =
    std::unordered_map<std::string,
                       std::pair<bool, std::unordered_map<int, float>>>;

struct State {
    int pc;
    Mem mem;
    std::vector<Word32> stack;

    State(int pc, Mem mem, std::vector<Word32> stack)
        : pc(pc), mem(mem), stack(stack) {}
};

struct Grad {
    std::unordered_map<int, float> val;
    Grad(std::unordered_map<int, float> val) : val(val) {}
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
    Grad operator+(float w) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.at(r.first) += w;
        }
        return Grad(result);
    }
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
    Grad operator*(float w) {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.at(r.first) *= w;
        }
        return Grad(result);
    }
    Grad abs() {
        std::unordered_map<int, float> result = val;
        for (auto &r : result) {
            result.emplace(std::make_pair(r.first, std::abs(r.second)));
        }
        return Grad(result);
    }
};

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

struct Sym {
    SymType symtype;
    Sym *left;
    Sym *right;
    Word32 word;
    int var_idx;

    Sym() {}
    Sym(SymType symtype, Sym *left) : symtype(symtype), left(left) {}
    Sym(SymType symtype, Sym *left, Sym *right)
        : symtype(symtype), left(left), right(right) {}

    Sym(SymType symtype, Word32 val) : symtype(symtype) {
        if (symtype == SymType::SAny) {
            var_idx = val;
        } else {
            word = val;
        }
    }

    void gather_var_ids(std::unordered_set<int> &result) {
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

    float eval(const std::unordered_map<int, float> &cvals, float eps) {
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

    Grad grad(const std::unordered_map<int, float> &cvals, float eps) {
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

    std::string toString(bool convert_to_num) {
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

using SMem = std::unordered_map<Word32, Sym>;

struct SymState {
    int pc;
    int var_cnt;
    Mem mem;
    SMem smem;
    Linkedlist<Sym> symbolic_stack;
    std::vector<Sym> path_constraints;

    SymState() : pc(0), var_cnt(0){};
    SymState(int pc, int varcnt, Mem mem, SMem smem, Linkedlist<Sym> symbolic_stack,
             std::vector<Sym> path_constraints)
        : pc(pc),
          mem(mem),
          smem(smem),
          symbolic_stack(symbolic_stack),
          path_constraints(path_constraints) {}

    void print() {
        printf("Stack: [");
        LLNode<Sym> *tmp = symbolic_stack.head;
        while (tmp != NULL) {
            printf("%s, ", tmp->data.toString(false).c_str());
            tmp = tmp->next;
        }
        printf("]\n");

        printf("Memory: {");
        float tmp_word;
        for (auto t : mem) {
            tmp_word = wordToFloat(t.second);
            if (is_integer(tmp_word)) {
                printf("var_%d: %d, ", (int)t.first, (int)tmp_word);
            } else {
                printf("var_%d: %f, ", (int)t.first, tmp_word);
            }
        }
        printf("}\n");

        printf("Symbolic Memory: {");
        for (auto t: smem) {
            printf("var_%d: %s, ", (int)t.first, t.second.toString(true).c_str());
        }
        printf("}\n");

        printf("Path Constraints: ");
        for (Sym &sym : path_constraints) {
            printf("(%s) && ", sym.toString(true).c_str());
        }
        printf(" 1\n");
    }
};

struct Trace {
    SymState data;
    std::vector<Trace> children;

    Trace(SymState data, std::vector<Trace> children)
        : data(data), children(children) {}

    void print(int indent_cnt = 0) {
        printf("PC: %d\n", data.pc);
        data.print();
        for (Trace &trace : children) {
            trace.print();
        }
    }
};
}  // namespace gymbo
