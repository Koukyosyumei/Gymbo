#include "concretes.h"
#include "symbolic.h"
#include "type.h"

int main() {
  std::vector<Instr> prg = {Instr(InstrType::Read),     Instr(InstrType::Read),
                            Instr(InstrType::Add),      Instr(InstrType::Dup),
                            Instr(InstrType::Push, 15), Instr(InstrType::Lt),
                            Instr(InstrType::Push, 10), Instr(InstrType::Swap),
                            Instr(InstrType::JmpIf),    Instr(InstrType::Done),
                            Instr(InstrType::Done)};
}
