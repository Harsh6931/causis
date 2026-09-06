#include "ir/optimizer.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace causis::ir {
namespace {

bool is_arithmetic_opcode(Opcode opcode) {
  return opcode == Opcode::Add || opcode == Opcode::Sub || opcode == Opcode::Mul ||
         opcode == Opcode::Div;
}

bool is_comparison_opcode(Opcode opcode) {
  return opcode == Opcode::Eq || opcode == Opcode::Ne || opcode == Opcode::Lt ||
         opcode == Opcode::Le || opcode == Opcode::Gt || opcode == Opcode::Ge;
}

//helper function that calc arthematic expressions at compiler time rather than waiting fr VM (runtime)
std::optional<int> fold_arithmetic(Opcode opcode, int left, int right) {
  switch (opcode) {
  case Opcode::Add:
    return left + right;
  case Opcode::Sub:
    return left - right;
  case Opcode::Mul:
    return left * right;
  case Opcode::Div:
      // Preserve division-by-zero as a runtime error.
    if (right == 0) {
      return std::nullopt;
    }
    return left / right;
  default:
    return std::nullopt;
  }
}

std::optional<bool> fold_int_comparison(Opcode opcode, int left, int right) {
  switch (opcode) {
  case Opcode::Eq:
    return left == right;
  case Opcode::Ne:
    return left != right;
  case Opcode::Lt:
    return left < right;
  case Opcode::Le:
    return left <= right;
  case Opcode::Gt:
    return left > right;
  case Opcode::Ge:
    return left >= right;
  default:
    return std::nullopt;
  }
}

std::optional<bool> fold_bool_comparison(Opcode opcode, bool left, bool right) {
  switch (opcode) {
  case Opcode::Eq:
    return left == right;
  case Opcode::Ne:
    return left != right;
  default:
    return std::nullopt;
  }
}

bool try_fold_binary(const Instruction& left, const Instruction& right, const Instruction& op,
                     Instruction& folded) {
  if (is_arithmetic_opcode(op.opcode)) {
    if (left.opcode != Opcode::PushInt || right.opcode != Opcode::PushInt) {
      return false;
    }

    const std::optional<int> value = fold_arithmetic(op.opcode, left.int_value, right.int_value);
    if (!value.has_value()) {
      return false;
    }

    folded = left;
    folded.int_value = *value;
    folded.line = op.line;
    folded.column = op.column;
    return true;
  }

  if (is_comparison_opcode(op.opcode)) {
    if (left.opcode == Opcode::PushInt && right.opcode == Opcode::PushInt) {
      const std::optional<bool> value =
          fold_int_comparison(op.opcode, left.int_value, right.int_value);
      if (!value.has_value()) {
        return false;
      }

      folded.opcode = Opcode::PushBool;
      folded.bool_value = *value;
      folded.line = op.line;
      folded.column = op.column;
      return true;
    }

    if (left.opcode == Opcode::PushBool && right.opcode == Opcode::PushBool) {
      const std::optional<bool> value =
          fold_bool_comparison(op.opcode, left.bool_value, right.bool_value);
      if (!value.has_value()) {
        return false;
      }

      folded = left;
      folded.bool_value = *value;
      folded.line = op.line;
      folded.column = op.column;
      return true;
    }
  }

  return false;
}

bool try_fold_unary(const Instruction& operand, const Instruction& op, Instruction& folded) {
  if (op.opcode == Opcode::Neg && operand.opcode == Opcode::PushInt) {
    folded = operand;
    folded.int_value = -operand.int_value;
    folded.line = op.line;
    folded.column = op.column;
    return true;
  }

  if (op.opcode == Opcode::Not && operand.opcode == Opcode::PushBool) {
    folded = operand;
    folded.bool_value = !operand.bool_value;
    folded.line = op.line;
    folded.column = op.column;
    return true;
  }

  return false;
}

bool try_remove_dead_pop(const Instruction& push, const Instruction& pop) {
  return (push.opcode == Opcode::PushInt || push.opcode == Opcode::PushBool) &&
         pop.opcode == Opcode::Pop;
}

IrProgram constant_fold_once(const IrProgram& program) {
  const std::vector<Instruction>& instructions = program.instructions;
  std::vector<Instruction> output;
  output.reserve(instructions.size());

  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (i + 2 < instructions.size() && is_arithmetic_opcode(instructions[i + 2].opcode)) {
      Instruction folded;
      if (try_fold_binary(instructions[i], instructions[i + 1], instructions[i + 2], folded)) {
        output.push_back(folded);
        i += 2;
        continue;
      }
    }

    if (i + 2 < instructions.size() && is_comparison_opcode(instructions[i + 2].opcode)) {
      Instruction folded;
      if (try_fold_binary(instructions[i], instructions[i + 1], instructions[i + 2], folded)) {
        output.push_back(folded);
        i += 2;
        continue;
      }
    }

    if (i + 1 < instructions.size() &&
        (instructions[i + 1].opcode == Opcode::Neg || instructions[i + 1].opcode == Opcode::Not)) {
      Instruction folded;
      if (try_fold_unary(instructions[i], instructions[i + 1], folded)) {
        output.push_back(folded);
        ++i;
        continue;
      }
    }

    if (i + 1 < instructions.size() &&
        try_remove_dead_pop(instructions[i], instructions[i + 1])) {
      ++i;
      continue;
    }

    output.push_back(instructions[i]);
  }

  IrProgram result;
  result.instructions = std::move(output);
  return result;
}

bool instructions_equal(const Instruction& lhs, const Instruction& rhs) {
  return lhs.opcode == rhs.opcode && lhs.text == rhs.text && lhs.text2 == rhs.text2 &&
         lhs.int_value == rhs.int_value && lhs.int_value2 == rhs.int_value2 &&
         lhs.bool_value == rhs.bool_value;
}

IrProgram constant_fold(const IrProgram& program) {
  IrProgram current = program;
  for (int pass = 0; pass < 8; ++pass) {
    IrProgram next = constant_fold_once(current);
    if (next.instructions.size() == current.instructions.size()) {
      bool same = true;
      for (std::size_t i = 0; i < next.instructions.size(); ++i) {
        const Instruction& next_instruction = next.instructions[i];
        const Instruction& current_instruction = current.instructions[i];
        if (!instructions_equal(next_instruction, current_instruction)) {
          same = false;
          break;
        }
      }
      if (same) {
        break;
      }
    }
    current = std::move(next);
  }
  return current;
}

std::unordered_map<std::string, std::size_t>
build_label_index(const std::vector<Instruction>& instructions) {
  std::unordered_map<std::string, std::size_t> labels;
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (instructions[i].opcode == Opcode::Label) {
      labels[instructions[i].text] = i;
    }
  }
  return labels;
}

std::optional<bool> read_const_branch(const std::vector<Instruction>& instructions,
                                      std::size_t index) {
  if (index + 1 >= instructions.size()) {
    return std::nullopt;
  }

  const Instruction& push = instructions[index];
  const Instruction& branch = instructions[index + 1];
  if (push.opcode != Opcode::PushBool || branch.opcode != Opcode::JumpIfFalse) {
    return std::nullopt;
  }

  return push.bool_value;
}

IrProgram eliminate_dead_branches(const IrProgram& program) {
  const std::vector<Instruction>& instructions = program.instructions;
  const std::unordered_map<std::string, std::size_t> labels = build_label_index(instructions);
  std::vector<Instruction> output;
  output.reserve(instructions.size());

  for (std::size_t i = 0; i < instructions.size();) {
    const std::optional<bool> condition = read_const_branch(instructions, i);
    if (!condition.has_value()) {
      output.push_back(instructions[i]);
      ++i;
      continue;
    }

    const std::string& else_label = instructions[i + 1].text;
    const auto else_it = labels.find(else_label);
    if (else_it == labels.end()) {
      output.push_back(instructions[i]);
      ++i;
      continue;
    }

    const std::size_t else_index = else_it->second;

    if (!*condition) {
      i = else_index;
      continue;
    }

    // if true: skip PUSH/JUMP_IF_FALSE, keep then-branch, drop else-branch when present.
    i += 2;
    if (else_index > i && instructions[else_index - 1].opcode == Opcode::Jump) {
      const auto end_it = labels.find(instructions[else_index - 1].text);
      if (end_it != labels.end()) {
        const std::size_t end_index = end_it->second;
        while (i < else_index - 1) {
          output.push_back(instructions[i]);
          ++i;
        }
        i = end_index;
        continue;
      }
    }

    while (i < else_index) {
      output.push_back(instructions[i]);
      ++i;
    }
  }

  IrProgram result;
  result.instructions = std::move(output);
  return result;
}

} // namespace

IrProgram optimize_program(const IrProgram& program) {
  IrProgram current = constant_fold(program);
  current = eliminate_dead_branches(current);
  current = constant_fold(current);
  return current;
}

} // namespace causis::ir
