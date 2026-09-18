#include "ir/optimizer.h"

#include <optional>
#include <sstream>
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

bool is_turn_opcode(Opcode opcode) {
  return opcode == Opcode::TurnLeft || opcode == Opcode::TurnRight;
}

// After stop()+jump, drop straight-line instructions until the next label.
IrProgram eliminate_unreachable_after_stop(const IrProgram& program) {
  const std::vector<Instruction>& instructions = program.instructions;
  std::vector<Instruction> output;
  output.reserve(instructions.size());

  bool skipping = false;
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    const Instruction& instruction = instructions[i];

    if (skipping) {
      if (instruction.opcode == Opcode::Label) {
        skipping = false;
        output.push_back(instruction);
      }
      continue;
    }

    output.push_back(instruction);

    if (instruction.opcode == Opcode::Stop && i + 1 < instructions.size() &&
        instructions[i + 1].opcode == Opcode::Jump) {
      output.push_back(instructions[i + 1]);
      ++i;
      skipping = true;
    }
  }

  IrProgram result;
  result.instructions = std::move(output);
  return result;
}

// Remove adjacent opposite turns with only optional Pop noise between them.
IrProgram eliminate_redundant_turn_pairs(const IrProgram& program) {
  const std::vector<Instruction>& instructions = program.instructions;
  std::vector<Instruction> output;
  output.reserve(instructions.size());

  const std::size_t n = instructions.size();
  for (std::size_t i = 0; i < n; ++i) {
    const Opcode first = instructions[i].opcode;
    if (!is_turn_opcode(first)) {
      output.push_back(instructions[i]);
      continue;
    }

    std::size_t j = i + 1;
    if (j < n && instructions[j].opcode == Opcode::Pop) {
      ++j;
    }

    if (j < n && is_turn_opcode(instructions[j].opcode) && instructions[j].opcode != first) {
      if (j + 1 < n && instructions[j + 1].opcode == Opcode::Pop) {
        i = j + 1;
      } else {
        i = j;
      }
      continue;
    }

    output.push_back(instructions[i]);
  }

  IrProgram result;
  result.instructions = std::move(output);
  return result;
}

} // namespace

OptimizationResult optimize_program_with_stats(const IrProgram& program) {
  OptimizationResult result;
  result.stats.ir_instructions_before = program.instructions.size();
  result.stats.passes_applied = {"constant-folding", "branch-pruning", "stop-unreachable",
                                 "redundant-turn-elimination"};

  IrProgram current = constant_fold(program);
  current = eliminate_dead_branches(current);
  current = constant_fold(current);
  current = eliminate_unreachable_after_stop(current);
  current = eliminate_redundant_turn_pairs(current);
  current = constant_fold(current);

  result.program = std::move(current);
  result.stats.ir_instructions_after = result.program.instructions.size();
  finalize_optimization_stats(result.stats);
  return result;
}

void finalize_optimization_stats(OptimizationStats& stats) {
  if (stats.ir_instructions_before >= stats.ir_instructions_after) {
    stats.ir_instructions_removed = stats.ir_instructions_before - stats.ir_instructions_after;
  } else {
    stats.ir_instructions_removed = 0;
  }

  if (stats.bytecode_instructions_before >= stats.bytecode_instructions_after) {
    stats.bytecode_instructions_removed =
        stats.bytecode_instructions_before - stats.bytecode_instructions_after;
  } else {
    stats.bytecode_instructions_removed = 0;
  }
}

IrProgram optimize_program(const IrProgram& program) {
  return optimize_program_with_stats(program).program;
}

namespace {

std::string format_reduction_percent(std::size_t before, std::size_t removed) {
  if (before == 0) {
    return "n/a";
  }

  const double reduction =
      100.0 * static_cast<double>(removed) / static_cast<double>(before);
  std::ostringstream out;
  out.setf(std::ios::fixed);
  out.precision(1);
  out << reduction << '%';
  return out.str();
}

} // namespace

std::string format_optimization_stats(const OptimizationStats& stats,
                                      const std::string& program_path) {
  OptimizationStats display = stats;
  finalize_optimization_stats(display);

  std::ostringstream out;
  if (!program_path.empty()) {
    out << "Program: " << program_path << "\n\n";
  }

  out << "Optimization Statistics\n";
  out << "-----------------------\n";
  out << "IR instructions:\n";
  out << "  Before       : " << display.ir_instructions_before << '\n';
  out << "  After        : " << display.ir_instructions_after << '\n';
  out << "  Removed      : " << display.ir_instructions_removed << '\n';
  out << "  Reduction    : "
      << format_reduction_percent(display.ir_instructions_before,
                                  display.ir_instructions_removed)
      << '\n';
  out << '\n';
  out << "Bytecode instructions:\n";
  out << "  Before       : " << display.bytecode_instructions_before << '\n';
  out << "  After        : " << display.bytecode_instructions_after << '\n';
  out << "  Removed      : " << display.bytecode_instructions_removed << '\n';
  out << "  Reduction    : "
      << format_reduction_percent(display.bytecode_instructions_before,
                                  display.bytecode_instructions_removed)
      << '\n';
  out << '\n';
  out << "Passes applied:\n";
  for (const std::string& pass : display.passes_applied) {
    out << "  - " << pass << '\n';
  }
  return out.str();
}

} // namespace causis::ir
