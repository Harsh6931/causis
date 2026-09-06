#include "bytecode/disassembler.h"

#include "bytecode/bytecode.h"

#include <iomanip>
#include <sstream>

namespace causis::bytecode {

std::string disassemble_program(const Program& program) {
  std::ostringstream out;

  for (std::size_t pc = 0; pc < program.code.size(); ++pc) {
    const Instruction& instruction = program.code[pc];
    out << std::setw(4) << std::setfill('0') << pc << ' ';
    out << format_instruction(program, instruction) << '\n';
  }

  return out.str();
}

} // namespace causis::bytecode
