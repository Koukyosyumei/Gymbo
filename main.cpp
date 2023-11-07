#include "sexpr.h"
#include <iostream>
#include <memory>

int main() {
  // Example usage:
  auto expr = std::make_shared<Or>(
      std::make_shared<Var>("A"),
      std::make_shared<Not>(std::make_shared<And>(
          std::make_shared<Var>("B"),
          std::make_shared<Or>(std::make_shared<Var>("A"),
                               std::make_shared<Var>("C")))));
  std::cout << "Expression: " << expr->to_string() << std::endl;

  auto cnf_expr = cnf(expr);
  std::cout << "CNF Expression: " << cnf_expr->to_string() << std::endl;

  auto le_expr = literalElimination(expr);
  std::cout << "EL Expression: " << le_expr->to_string() << std::endl;

  auto expr2 = std::make_shared<And>(
      std::make_shared<Var>("A"),
      std::make_shared<Or>(std::make_shared<Var>("B"),
                           std::make_shared<Not>(std::make_shared<Var>("A"))));
  auto up_expr = unitPropagation(expr2);
  std::cout << "UP Expression: " << up_expr->to_string() << std::endl;

  std::cout << satisfiableDPLL(expr) << std::endl;
  std::cout << satisfiableDPLL(le_expr) << std::endl;

  return 0;
}
