// Copyright (c) 2021-2023, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include "highlevel_opt.h"


HighLevelOpt::HighLevelOpt(const Options &options)
  : m_options(options) {
}

HighLevelOpt::~HighLevelOpt() {
}

struct LVNKey {
    int opcode;                     // Opcode of the instruction
    std::vector<int> operands;      // Value numbers of the operands
    bool is_constant_result;        // Flag to indicate if the result is a constant

    // Constructor
    LVNKey(int op, const std::vector<int>& ops, bool is_constant = false)
        : opcode(op), operands(ops), is_constant_result(is_constant) {
        // For commutative operations, ensure operands are in canonical order
        if (is_commutative()) {
            std::sort(operands.begin(), operands.end());
        }
    }

    // Check if the key represents a constant computation
    bool is_constant() const {
        return is_constant_result; // Return whether the result is a constant
    }

    // Check if the operation is commutative (e.g., ADD, MUL)
    bool is_commutative() const {
        // Add the list of commutative operations
        return (opcode == HINS_add_b || opcode == HINS_add_w ||opcode == HINS_add_l ||opcode == HINS_add_q || 
                opcode == HINS_mul_b || opcode == HINS_mul_w ||opcode == HINS_mul_l ||opcode == HINS_mul_q);
    }

    // Equality operator for hashing and comparisons
    bool operator==(const LVNKey& other) const {
        return opcode == other.opcode && operands == other.operands;
    }

    // Hash function for LVNKey
    struct Hash {
        size_t operator()(const LVNKey& key) const {
            size_t hash_value = std::hash<int>()(key.opcode);
            for (auto operand : key.operands) {
                hash_value ^= std::hash<int>()(operand) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
            }
            return hash_value;
        }
    };

    bool operator<(const LVNKey& other) const {
        // First, compare the opcode
        if (opcode != other.opcode) {
            return opcode < other.opcode;
        }

        // Then, compare the operands (lexicographically)
        if (operands != other.operands) {
            return operands < other.operands;
        }

        // Finally, compare whether the result is a constant
        return is_constant_result < other.is_constant_result;
    }
};

class LVN : public ControlFlowGraphTransform {
  private:
    LiveVregs m_live_vregs;
    std::map<int, int> constant_to_value_number;                   // Map constant values to value numbers
    std::map<int, int> value_number_to_constant;                   // Map value numbers to constants
    std::map<int, int> vreg_to_value_number;                       // Map vregs to value numbers
    std::map<int, std::vector<int>> value_number_to_vregs;  // Map value numbers to vregs
    std::map<LVNKey, int> lvnkey_to_value_number;                  // Map LVNKey to value number
    int next_value_number = 1;                                               // Next value number to assign
  public:

    LVN(std::shared_ptr<ControlFlowGraph> cfg): ControlFlowGraphTransform(cfg), m_live_vregs(cfg) {
      m_live_vregs.execute(); // compute vreg liveness
    }


    virtual std::shared_ptr<InstructionSequence> transform_basic_block(std::shared_ptr<InstructionSequence> orig_bb) {
      //Preform Local Value Numbering
      std::map<int, int> constant_to_value_number;                   // Map constant values to value numbers
      std::map<int, int> value_number_to_constant;                   // Map value numbers to constants
      std::map<int, int> vreg_to_value_number;                       // Map vregs to value numbers
      std::map<int, std::vector<int>> value_number_to_vregs;  // Map value numbers to vregs
      std::map<LVNKey, int> lvnkey_to_value_number;                  // Map LVNKey to value number
      int next_value_number = 1;                                               // Next value number to assign
      // printf("NEW BB\n");
      // printf("SIZE OF VREG TO VN: %d \n",vreg_to_value_number.size());
      std::shared_ptr<InstructionSequence> new_bb(new InstructionSequence());
      for (auto it = orig_bb->cbegin(); it != orig_bb->cend(); ++it) {
        const auto& inst = *it;
        if (inst->get_num_operands() <= 0 || inst->get_operand(0).is_label() || (inst->get_num_operands() > 2 && inst->get_operand(1).is_label()) || inst->get_operand(0).is_imm_ival()) {
          new_bb->append(inst->duplicate());
          continue;
        }

        // Extract operands and opcode
        int num_ops = inst->get_num_operands();
        auto opcode = inst->get_opcode();

        // Track value numbers for operands
        std::vector<int> operand_value_numbers;
        bool constant_value = true;
        for (int i = 1; i < num_ops; ++i) {
            auto operand = inst->get_operand(i);
            int operand_value_number;

            if (!operand.is_imm_ival()) {
              constant_value = false;
            }

            if (operand.is_imm_ival()) {
                // Assign or retrieve value number for constant
                int i_val = operand.get_imm_ival();
                if (constant_to_value_number.count(i_val) == 0) {
                    constant_to_value_number[i_val] = next_value_number;
                    value_number_to_constant[next_value_number] = i_val;
                    ++next_value_number;
                }
                operand_value_number = constant_to_value_number[i_val];
            } else if (!operand.is_imm_label() && operand.get_kind() != Operand::LABEL){
                // Assign or retrieve value number for virtual register
                // printf("OPERAND KIND %d \n",operand.get_kind());
                auto vreg = operand.get_base_reg();
                if (vreg_to_value_number.count(vreg) == 0) {
                    vreg_to_value_number[vreg] = next_value_number;
                    value_number_to_vregs[next_value_number].push_back(vreg);
                    ++next_value_number;
                }
                operand_value_number = vreg_to_value_number[vreg];
            } 
            // printf("%d\n",operand_value_number);

            operand_value_numbers.push_back(operand_value_number);
            operand.set_val_num(operand_value_number);
        }

        // Create an LVNKey
         
        LVNKey key(opcode, operand_value_numbers, constant_value);

        // Determine if the value is a compile-time constant
        int result_value_number;
        auto operand = inst->get_operand(0);
        if (opcode == HINS_mov_b || opcode == HINS_mov_w || opcode == HINS_mov_l || opcode == HINS_mov_q) {
          result_value_number = operand_value_numbers[0];
          lvnkey_to_value_number[key] = result_value_number;
        } else {
          if (lvnkey_to_value_number.count(key) == 0) {
            lvnkey_to_value_number[key] = next_value_number;
              ++next_value_number;
          }
        }
        result_value_number = lvnkey_to_value_number[key];
        // printf("OPERAND KIND %d \n",operand.get_kind());
        auto vreg = operand.get_base_reg();
        vreg_to_value_number[vreg] = result_value_number;
        value_number_to_vregs[result_value_number].push_back(vreg);
        // printf("%d\n",result_value_number);
        operand.set_val_num(result_value_number);

        //DO COPY PROPOGATION
        auto new_inst = inst->duplicate();
        for (int i = 1; i < num_ops; ++i) {
          auto operand = new_inst->get_operand(i);
          if (!operand.is_imm_ival() && !operand.is_label()) {
            // printf("OPERAND KIND %d \n",operand.get_kind());
            int original_reg = operand.get_base_reg();
            int target_value = vreg_to_value_number[original_reg];
            int target_reg = value_number_to_vregs[target_value][0];
            new_inst->set_operand(i,Operand(operand.get_kind(),target_reg));
            // printf("%d %d \n",original_reg,target_reg);
          }
        }


        // Append the instruction to the new basic block
        new_bb->append(new_inst);
      }
    return new_bb;
    }
};


class DSE : public ControlFlowGraphTransform {
  private:
    LiveVregs m_live_vregs;
   
  public:

    DSE(std::shared_ptr<ControlFlowGraph> cfg): ControlFlowGraphTransform(cfg), m_live_vregs(cfg) {
      m_live_vregs.execute(); // compute vreg liveness
    }


    virtual std::shared_ptr<InstructionSequence> transform_basic_block(std::shared_ptr<InstructionSequence> orig_bb) {
      std::shared_ptr<InstructionSequence> new_bb(new InstructionSequence());
      for (auto it = orig_bb->cbegin(); it != orig_bb->cend(); ++it) {
        Instruction* inst = *it;

        if (inst->get_num_operands() <= 0 || inst->get_operand(0).is_label() || inst->get_operand(0).is_imm_ival() || (inst->get_num_operands() >= 2 && inst->get_operand(1).is_label())) {
          new_bb->append(inst->duplicate());
          continue;
        }

        if (inst->get_opcode() == HINS_mov_b || inst->get_opcode() == HINS_mov_w || inst->get_opcode() == HINS_mov_l || inst->get_opcode() == HINS_mov_q) {
          if (!inst->get_operand(1).is_imm_label() && !inst->get_operand(1).is_label() && !inst->get_operand(1).is_imm_ival())
          if (inst->get_operand(0).get_base_reg() == inst->get_operand(1).get_base_reg()){
            continue;
          }
        }

        // Get the destination register
        auto dst = inst->get_operand(0);

        // Retrieve liveness fact after the instruction
        LiveVregs::FactType fact = m_live_vregs.get_fact_after_instruction(orig_bb, inst);

        // Check if the destination register is live
        bool dst_live = fact.test(dst.get_base_reg()); 


        if (!dst_live && dst.get_base_reg() > 10 && dst.get_kind() != Operand::VREG_MEM && dst.get_kind() != Operand::VREG_MEM_OFF) {
          // Destination register is not live; skip this instruction
          continue;
        }

        // If dst is live, append the instruction to the new basic block
        new_bb->append(inst->duplicate());
      }
      return new_bb;
    }
};


void HighLevelOpt::optimize(std::shared_ptr<Function> function) {
  assert(m_options.has_option(Options::OPTIMIZE));

  // m_function can be used by helper functions to refer to
  // the Function
  m_function = function;

  // TODO: perform optimizations on the high-level InstructionSequence

  // Most optimizations should be implemented as objects belonging to classes
  // which derive from ControlFlowGraphTransform. Each optimization should
  // take the current control-flow graph as input and generate a transformed
  // control-flow graph. At the end, the final control-flow graph can be
  // converted back to an instruction sequence.
  //
  // E.g.:
  //
  //   std::shared_ptr<InstructionSequence> hl_iseq = m_function->get_hl_iseq();
  //   auto hl_cfg_builder = ::make_highlevel_cfg_builder(hl_iseq);
  //   std::shared_ptr<ControlFlowGraph> hl_cfg = hl_cfg_builder.build();
  //
  //   Optimization1 opt1(hl_cfg, m_options);
  //   hl_cfg = opt1.transform_cfg();
  //
  //   Optimization2 opt2(hl_cfg, m_options);
  //   hl_cfg = opt2.transform_cfg();
  //
  //   ...etc...
  //
  //   hl_iseq = hl_cfg->create_instruction_sequence();
  //   m_function->set_hl_iseq(hl_iseq);

  std::shared_ptr<InstructionSequence> hl_iseq = m_function->get_hl_iseq();
  auto hl_cfg_builder = ::make_highlevel_cfg_builder(hl_iseq);
  std::shared_ptr<ControlFlowGraph> hl_cfg = hl_cfg_builder.build();


  //Local Value Numbering (and copy propogation)
  LVN lvn(hl_cfg);
  hl_cfg = lvn.transform_cfg();

  DSE dse(hl_cfg);
  hl_cfg = dse.transform_cfg();


  hl_iseq = hl_cfg->create_instruction_sequence();
  m_function->set_hl_iseq(hl_iseq);

}


