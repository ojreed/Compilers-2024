// Copyright (c) 2021-2024, David H. Hovemeyer <david.hovemeyer@gmail.com>
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


/*
TODO LIST:
1) BASIC CODE FUNCTIONALITY
  - Example03 fix broken loops
2) fix LEAQ
4) other GS comments
5) circle back to fix whatever the issue is with high examples
6) circle back to implement proper type casting

*/



#include <cassert>
#include <map>
#include <sstream>
#include "node.h"
#include "instruction.h"
#include "operand.h"
#include "local_storage_allocation.h"
#include "highlevel.h"
#include "lowlevel.h"
#include "highlevel_formatter.h"
#include "exceptions.h"
#include "lowlevel_codegen.h"

int get_size(HighLevelOpcode opcode);

// This map has some "obvious" translations of high-level opcodes to
// low-level opcodes.
const std::map<HighLevelOpcode, LowLevelOpcode> HL_TO_LL = {
  { HINS_nop, MINS_NOP},
  { HINS_add_b, MINS_ADDB },
  { HINS_add_w, MINS_ADDW },
  { HINS_add_l, MINS_ADDL },
  { HINS_add_q, MINS_ADDQ },
  { HINS_sub_b, MINS_SUBB },
  { HINS_sub_w, MINS_SUBW },
  { HINS_sub_l, MINS_SUBL },
  { HINS_sub_q, MINS_SUBQ },
  { HINS_mul_l, MINS_IMULL },
  { HINS_mul_q, MINS_IMULQ },
  { HINS_mov_b, MINS_MOVB },
  { HINS_mov_w, MINS_MOVW },
  { HINS_mov_l, MINS_MOVL },
  { HINS_mov_q, MINS_MOVQ },
  { HINS_sconv_bw, MINS_MOVSBW },
  { HINS_sconv_bl, MINS_MOVSBL },
  { HINS_sconv_bq, MINS_MOVSBQ },
  { HINS_sconv_wl, MINS_MOVSWL },
  { HINS_sconv_wq, MINS_MOVSWQ },
  { HINS_sconv_lq, MINS_MOVSLQ },
  { HINS_uconv_bw, MINS_MOVZBW },
  { HINS_uconv_bl, MINS_MOVZBL },
  { HINS_uconv_bq, MINS_MOVZBQ },
  { HINS_uconv_wl, MINS_MOVZWL },
  { HINS_uconv_wq, MINS_MOVZWQ },
  { HINS_uconv_lq, MINS_MOVZLQ },
  { HINS_ret, MINS_RET },
  { HINS_jmp, MINS_JMP },
  { HINS_call, MINS_CALL },

  // For comparisons, it is expected that the code generator will first
  // generate a cmpb/cmpw/cmpl/cmpq instruction to compare the operands,
  // and then generate a setXX instruction to put the result of the
  // comparison into the destination operand. These entries indicate
  // the apprpropriate setXX instruction to use.
  { HINS_cmplt_b, MINS_SETL },
  { HINS_cmplt_w, MINS_SETL },
  { HINS_cmplt_l, MINS_SETL },
  { HINS_cmplt_q, MINS_SETL },
  { HINS_cmplte_b, MINS_SETLE },
  { HINS_cmplte_w, MINS_SETLE },
  { HINS_cmplte_l, MINS_SETLE },
  { HINS_cmplte_q, MINS_SETLE },
  { HINS_cmpgt_b, MINS_SETG },
  { HINS_cmpgt_w, MINS_SETG },
  { HINS_cmpgt_l, MINS_SETG },
  { HINS_cmpgt_q, MINS_SETG },
  { HINS_cmpgte_b, MINS_SETGE },
  { HINS_cmpgte_w, MINS_SETGE },
  { HINS_cmpgte_l, MINS_SETGE },
  { HINS_cmpgte_q, MINS_SETGE },
  { HINS_cmpeq_b, MINS_SETE },
  { HINS_cmpeq_w, MINS_SETE },
  { HINS_cmpeq_l, MINS_SETE },
  { HINS_cmpeq_q, MINS_SETE },
  { HINS_cmpneq_b, MINS_SETNE },
  { HINS_cmpneq_w, MINS_SETNE },
  { HINS_cmpneq_l, MINS_SETNE },
  { HINS_cmpneq_q, MINS_SETNE },
};

LowLevelCodeGen::LowLevelCodeGen(const Options &options)
  : m_options(options)
  , m_total_memory_storage(0) {
}

LowLevelCodeGen::~LowLevelCodeGen() {
}

void LowLevelCodeGen::generate(std::shared_ptr<Function> function) {
  // Make the Function object available to member functions
  m_function = function;

  // The translation is done in the translate_hl_to_ll() member function
  std::shared_ptr<InstructionSequence> ll_iseq = translate_hl_to_ll(function->get_hl_iseq());
  m_function->set_ll_iseq(ll_iseq);
}

std::shared_ptr<InstructionSequence> LowLevelCodeGen::translate_hl_to_ll(std::shared_ptr<InstructionSequence> hl_iseq) {
  std::shared_ptr<InstructionSequence> ll_iseq(new InstructionSequence());

  // The high-level InstructionSequence will have a pointer to the Node
  // representing the function definition. Useful information could be stored
  // there (for example, about the amount of memory needed for local storage,
  // maximum number of virtual registers used, etc.)
  Node *funcdef_ast = m_function->get_funcdef_ast();
  assert(funcdef_ast != nullptr);

  // Determine the total number of bytes of memory storage
  // that the function needs. This should include both variables that
  // *must* have storage allocated in memory (e.g., arrays), and also
  // any additional memory that is needed for virtual registers,
  // spilled machine registers, etc.

  m_total_memory_storage = funcdef_ast->get_total_local_storage();

  m_data_base = m_total_memory_storage;

  if ((m_total_memory_storage) % 16 != 0)
    m_total_memory_storage += (16 - (m_total_memory_storage % 16));

  m_register_base = m_total_memory_storage;

  
  // Symbol *fn_id = funcdef_ast->get_kid(1)->get_symbol();
  // SymbolTable *l_symtab = fn_id->get_symtab_k();
  // ;
  // for (auto i = l_symtab->cbegin(); i != l_symtab->cend(); ++i) { //for all local vars
  //   Symbol *s = *i;
  //   if (s->get_type()->is_array() || s->get_type()->is_struct()){//needs mem_alloc

  //     /*do nothing already allocated*/

  //   } else { //can be stored in register
  //     ;
  //   }
  // }
  m_total_memory_storage += 8*(m_function->get_vra()->get_size());

  if ((m_total_memory_storage) % 16 != 0)
    m_total_memory_storage += (16 - (m_total_memory_storage % 16));

  // The function prologue will push %rbp, which should guarantee that the
  // stack pointer (%rsp) will contain an address that is a multiple of 16.
  // If the total memory storage required is not a multiple of 16, add to
  // it so that it is.


  // Iterate through high level instructions
  for (auto i = hl_iseq->cbegin(); i != hl_iseq->cend(); ++i) {
    Instruction *hl_ins = *i;

    // If the high-level instruction has a label, define an equivalent
    // label in the low-level instruction sequence
    if (i.has_label())
      ll_iseq->define_label(i.get_label());

    // Translate the high-level instruction into one or more low-level instructions.
    // The first generated low-level instruction is annotated with a textual
    // representation of the high-level instruction.
    unsigned ll_idx = ll_iseq->get_length();
    translate_instruction(hl_ins, ll_iseq);
    HighLevelFormatter hl_formatter;
    ll_iseq->get_instruction(ll_idx)->set_comment(hl_formatter.format_instruction(hl_ins));
  }

  return ll_iseq;
}

// These helper functions are provided to make it easier to handle
// the way that instructions and operands vary based on operand size
// ('b'=1 byte, 'w'=2 bytes, 'l'=4 bytes, 'q'=8 bytes.)

// Check whether hl_opcode matches a range of opcodes, where base
// is a _b variant opcode. Return true if the hl opcode is any variant
// of that base.
bool match_hl(int base, int hl_opcode) {
  return hl_opcode >= base && hl_opcode < (base + 4);
}

// For a low-level instruction with 4 size variants, return the correct
// variant. base_opcode should be the "b" variant, and operand_size
// should be the operand size in bytes (1, 2, 4, or 8.)
LowLevelOpcode select_ll_opcode(LowLevelOpcode base_opcode, int operand_size) {
  int off;

  switch (operand_size) {
  case 1: // 'b' variant
    off = 0; break;
  case 2: // 'w' variant
    off = 1; break;
  case 4: // 'l' variant
    off = 2; break;
  case 8: // 'q' variant
    off = 3; break;
  default:
    assert(false);
    off = 3;
  }

  return LowLevelOpcode(int(base_opcode) + off);
}

// Get the correct Operand::Kind value for a machine register
// of the specified size (1, 2, 4, or 8 bytes.)
Operand::Kind select_mreg_kind(int operand_size) {
  switch (operand_size) {
  case 1:
    return Operand::MREG8;
  case 2:
    return Operand::MREG16;
  case 4:
    return Operand::MREG32;
  case 8:
    return Operand::MREG64;
  default:
    assert(false);
    return Operand::MREG64;
  }
}

void LowLevelCodeGen::translate_instruction(Instruction *hl_ins, std::shared_ptr<InstructionSequence> ll_iseq) {
  HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());

  if (hl_opcode == HINS_enter) {
    // Function prologue: this will create an ABI-compliant stack frame.
    // The local variable area is *below* the address in %rbp, and local storage
    // can be accessed at negative offsets from %rbp. For example, the topmost
    // 4 bytes in the local storage area are at -4(%rbp).
    ll_iseq->append(new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, MREG_RBP)));
    ll_iseq->append(new Instruction(MINS_MOVQ, Operand(Operand::MREG64, MREG_RSP), Operand(Operand::MREG64, MREG_RBP)));
    if (m_total_memory_storage > 0)
      ll_iseq->append(new Instruction(MINS_SUBQ, Operand(Operand::IMM_IVAL, m_total_memory_storage), Operand(Operand::MREG64, MREG_RSP)));

    // save callee-saved registers (if any)
    //if you allocated callee-saved registers as storage for local variables,
    //emit pushq instructions to save their original values
    std::vector<MachineReg> callee_saved = {MREG_RBP, MREG_RBX, MREG_R12, MREG_R13, MREG_R14, MREG_R15};
    for (MachineReg reg : callee_saved) {
      Instruction* push_inst = new Instruction(MINS_PUSHQ, Operand(Operand::MREG64,reg));
      push_inst->set_comment("Pushing Callee saved to stack");
      ll_iseq->append(push_inst);
    }

    return;
  }


  if (hl_opcode == HINS_leave) {
    // Function epilogue: deallocate local storage area and restore original value
    // of %rbp

    //if you allocated callee-saved registers as storage for local variables,
    //emit popq instructions to save their original values
    std::vector<MachineReg> callee_saved = {MREG_RBP, MREG_RBX, MREG_R12, MREG_R13, MREG_R14, MREG_R15};
    for (int i = callee_saved.size() - 1; i >= 0; --i){//reverse iterate for popq
      Instruction* push_inst = new Instruction(MINS_POPQ, Operand(Operand::MREG64,callee_saved[i]));
      push_inst->set_comment("Popping callee saved back to proper register");
      ll_iseq->append(push_inst);
    }

    if (m_total_memory_storage > 0)
      ll_iseq->append(new Instruction(MINS_ADDQ, Operand(Operand::IMM_IVAL, m_total_memory_storage), Operand(Operand::MREG64, MREG_RSP)));
    ll_iseq->append(new Instruction(MINS_POPQ, Operand(Operand::MREG64, MREG_RBP)));

    return;
  }

  if (hl_opcode == HINS_ret) {
    ll_iseq->append(new Instruction(MINS_RET));
    return;
  }

  // TODO: handle other high-level instructions
  // Note that you can use the highlevel_opcode_get_source_operand_size() and
  // highlevel_opcode_get_dest_operand_size() functions to determine the
  // size (in bytes, 1, 2, 4, or 8) of either the source operands or
  // destination operand of a high-level instruction. This should be useful
  // for choosing the appropriate low-level instructions and
  // machine register operands.



  

  std::set<HighLevelOpcode> ARITH_OPS = {HINS_add_b,HINS_add_w,HINS_add_l,HINS_add_q,
                                         HINS_sub_b,HINS_sub_w,HINS_sub_l,HINS_sub_q,
                                         HINS_div_b,HINS_div_w,HINS_div_l,HINS_div_q,
                                         HINS_mul_b,HINS_mul_w,HINS_mul_l,HINS_mul_q,
                                         HINS_mod_b,HINS_mod_w,HINS_mod_l,HINS_mod_q};

  if (ARITH_OPS.count(hl_opcode) > 0) {//found a binary arithmatic operation
    Operand dest = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);
    Operand src1 = get_ll_operand(hl_ins->get_operand(1), get_size(hl_opcode),ll_iseq);
    Operand src2 = get_ll_operand(hl_ins->get_operand(2), get_size(hl_opcode),ll_iseq);

    //FORMAT TOP COMMENT
    Instruction* no_inst = new Instruction(HL_TO_LL.at(HINS_nop));
    std::string comment;
    if (match_hl(HINS_add_b,hl_opcode)){
      comment = "dst = src1 + src2";
    } else if (match_hl(HINS_sub_b,hl_opcode)){
      comment = "dst = src1 - src2";
    } else if (match_hl(HINS_mul_b,hl_opcode)){
      comment = "dst = src1 * src2";
    } else if (match_hl(HINS_div_b,hl_opcode)){
      comment = "dst = src1 / src2";
    } else if (match_hl(HINS_mod_b,hl_opcode)){
      comment = "dst = src1 \% src2";
    }
    no_inst->set_comment(comment);
    ll_iseq->append(no_inst);

        
    Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R11);
    Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
    clear_inst->set_comment("Clear tmp register");
    ll_iseq->append(clear_inst);

    temp = Operand(select_mreg_kind(get_size(hl_opcode)),MachineReg::MREG_R11);
    
    //MOVE SOURCE1 to TEMP
    LowLevelOpcode mov_t_opcode = select_ll_opcode(MINS_MOVB, get_size(hl_opcode));
    Instruction* mv_t_inst = new Instruction(mov_t_opcode, src1, temp);
    mv_t_inst->set_comment("Moving SRC1 to temp");
    ll_iseq->append(mv_t_inst);

    //APPLY SOURCE2 to TEMP
    LowLevelOpcode arith_opcode = HL_TO_LL.at(hl_opcode);
    Instruction* op_inst = new Instruction(arith_opcode, src2, temp);
    op_inst->set_comment("Applying operation to temp with SRC2");
    ll_iseq->append(op_inst);

    //MOVE TEMP to DST
    LowLevelOpcode mov_b_opcode = select_ll_opcode(MINS_MOVB, get_size(hl_opcode));
    Instruction* mv_b_inst = new Instruction(mov_b_opcode, temp, dest);
    mv_b_inst->set_comment("Moving temp to dest");
    ll_iseq->append(mv_b_inst);

    return;
  }

  std::set<HighLevelOpcode> CMP_OPS = {HINS_cmplt_b,HINS_cmplt_w,HINS_cmplt_l,HINS_cmplt_q,
                                       HINS_cmplte_b,HINS_cmplte_w,HINS_cmplte_l,HINS_cmplte_q,
                                       HINS_cmpgt_b,HINS_cmpgt_w,HINS_cmpgt_l,HINS_cmpgt_q,
                                       HINS_cmpgte_b,HINS_cmpgte_w,HINS_cmpgte_l,HINS_cmpgte_q,
                                       HINS_cmpeq_b,HINS_cmpeq_w,HINS_cmpeq_l,HINS_cmpeq_q,
                                       HINS_cmpneq_b,HINS_cmpneq_w,HINS_cmpneq_l,HINS_cmpneq_q};

  if (CMP_OPS.count(hl_opcode) > 0) {//found a binary comparison operation
    Operand dest = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);
    Operand src1 = get_ll_operand(hl_ins->get_operand(1), get_size(hl_opcode),ll_iseq);
    Operand src2 = get_ll_operand(hl_ins->get_operand(2), get_size(hl_opcode),ll_iseq);

    //FORMAT TOP COMMENT
    Instruction* no_inst = new Instruction(HL_TO_LL.at(HINS_nop));
    std::string comment;
    int offset = 0;
    if (match_hl(HINS_cmplt_b,hl_opcode)){
      comment = "dst = src2 < src1";
      offset = 0;
    } else if (match_hl(HINS_cmplte_b,hl_opcode)){
      comment = "dst = src2 <= src1";
      offset = 1;
    } else if (match_hl(HINS_cmpgt_b,hl_opcode)){
      comment = "dst = src2 > src1";
      offset = 2;
    } else if (match_hl(HINS_cmpgte_b,hl_opcode)){
      comment = "dst = src2 >= src1";
      offset = 3;
    } else if (match_hl(HINS_cmpeq_b,hl_opcode)){
      comment = "dst = src2 == src1";
      offset = 4;
    } else if (match_hl(HINS_cmpneq_b,hl_opcode)){
      comment = "dst = src2 != src1";
      offset = 5;
    }
    no_inst->set_comment(comment);
    ll_iseq->append(no_inst);

    Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R11);
    Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
    clear_inst->set_comment("Clear tmp register");
    ll_iseq->append(clear_inst);

    temp = Operand(select_mreg_kind(get_size(hl_opcode)),MachineReg::MREG_R11);

    //MOVE SOURCE1 to TEMP
    LowLevelOpcode mov_t_opcode = select_ll_opcode(MINS_MOVB, get_size(hl_opcode));
    Instruction* mv_t_inst = new Instruction(mov_t_opcode, src1, temp);
    mv_t_inst->set_comment("Moving SRC1 to temp");
    ll_iseq->append(mv_t_inst);

    //Compare SOURCE2 to SOURCE1
    LowLevelOpcode mov_opcode = select_ll_opcode(MINS_CMPB, get_size(hl_opcode));
    Instruction* mv_inst = new Instruction(mov_opcode, src2, temp);
    mv_inst->set_comment("Compare SRC1 and SRC2");
    ll_iseq->append(mv_inst);

    //SET DST to FLAG
    LowLevelOpcode setter = LowLevelOpcode(MINS_SETL + offset);
    Instruction* op_inst = new Instruction(setter, dest);
    op_inst->set_comment("Store Result Flag in DST");
    ll_iseq->append(op_inst);

    return;
  }

  std::set<HighLevelOpcode> UNA_OPS = {HINS_neg_b,HINS_neg_w,HINS_neg_l,HINS_neg_q,
                                       HINS_not_b,HINS_not_w,HINS_not_l,HINS_not_q};

  if (UNA_OPS.count(hl_opcode) > 0) {//found a unary operation
    
    Operand target = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);

    if (match_hl(hl_opcode,HINS_neg_b)){
      //COMMENT
      Instruction* no_inst = new Instruction(HL_TO_LL.at(HINS_nop));
      std::stringstream comment;
      comment << target.get_base_reg() << " = -" << target.get_base_reg();
      no_inst->set_comment(comment.str());
      ll_iseq->append(no_inst);

      //OPERATION
      LowLevelOpcode arith_opcode = select_ll_opcode(HL_TO_LL.at(HINS_mul_b), get_size(hl_opcode));
      LiteralValue neg = LiteralValue(-1,false,false);
      Instruction* neg_inst = new Instruction(arith_opcode,  Operand(Operand::IMM_IVAL, -1), target);
      neg_inst->set_comment("Negate target through multiplication by -1");
      ll_iseq->append(neg_inst);


    } else {
      //COMMENT
      Instruction* no_inst = new Instruction(HL_TO_LL.at(HINS_nop));
      std::stringstream comment;
      comment << target.get_base_reg() << " = !" << target.get_base_reg();
      no_inst->set_comment(comment.str());
      ll_iseq->append(no_inst);

      //OPERATION
      //Compare TARGET to 0
      Instruction* mv_inst = new Instruction(MINS_CMPB, target, Operand(Operand::IMM_IVAL, 0));
      mv_inst->set_comment("Compare Target with 0");
      ll_iseq->append(mv_inst);

      //SET DST to FLAG
      Instruction* op_inst = new Instruction(MINS_SETNE, target);
      op_inst->set_comment("Store Result Flag in DST");
      ll_iseq->append(op_inst);
    }
    return;
  }

  std::set<HighLevelOpcode> MOV_OPS = {HINS_mov_b,HINS_mov_w,HINS_mov_l,HINS_mov_q};

  if (MOV_OPS.count(hl_opcode) > 0) {//found a move operation
    Operand dest = get_ll_operand(hl_ins->get_operand(0), 8,ll_iseq);
    Operand src = get_ll_operand(hl_ins->get_operand(1), get_size(hl_opcode),ll_iseq);

    Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), dest);
    clear_inst->set_comment("Clear dest register");
    ll_iseq->append(clear_inst);


    dest = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);



    Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R11);

    clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
    clear_inst->set_comment("Clear temp register");
    ll_iseq->append(clear_inst);

    temp = Operand(select_mreg_kind(get_size(hl_opcode)),MachineReg::MREG_R11);

    //MOVE SOURCE to temp
    LowLevelOpcode mov_a_opcode = select_ll_opcode(MINS_MOVB, get_size(hl_opcode));
    Instruction* mv_a_inst = new Instruction(mov_a_opcode, src, temp);
    mv_a_inst->set_comment("Moving src to temp");
    ll_iseq->append(mv_a_inst);

    //MOVE temp to DEST
    LowLevelOpcode mov_b_opcode = select_ll_opcode(MINS_MOVB, get_size(hl_opcode));
    Instruction* mv_b_inst = new Instruction(mov_b_opcode, temp, dest);
    mv_b_inst->set_comment("Moving temp to dst");
    ll_iseq->append(mv_b_inst);



    return;
  }

  std::set<HighLevelOpcode> JMP_OPS = {HINS_jmp,HINS_cjmp_t,HINS_cjmp_f};

  if (JMP_OPS.count(hl_opcode) > 0) {//found a jmp operation


    //Unconditional JMP to DST
    if (match_hl(hl_opcode,HINS_jmp)){
      Operand label = hl_ins->get_operand(0);

      Instruction* mv_inst = new Instruction(MINS_JMP, label);
      mv_inst->set_comment("jumping to dst");
      ll_iseq->append(mv_inst);
    } else if (match_hl(hl_opcode,HINS_cjmp_t)) {
      Operand dst = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);
      Operand label = hl_ins->get_operand(1);

      Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R11);
      Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
      clear_inst->set_comment("Clear tmp register");
      ll_iseq->append(clear_inst);

      temp = Operand(select_mreg_kind(1),MachineReg::MREG_R11);

      //MOVE Literal to TEMP
      LowLevelOpcode mov_t_opcode = select_ll_opcode(MINS_MOVB, 1);
      Instruction* mv_t_inst = new Instruction(mov_t_opcode, Operand(Operand::IMM_IVAL, 0), temp);
      mv_t_inst->set_comment("Moving dst to temp");
      ll_iseq->append(mv_t_inst);
      
      Instruction* cmp_inst = new Instruction(MINS_CMPB, temp, dst);
      cmp_inst->set_comment("Compare dst with 0");
      ll_iseq->append(cmp_inst);

      Instruction* jmp_inst = new Instruction(MINS_JNE, label);
      jmp_inst->set_comment("jumping if to dst if true (dst !=0)");
      ll_iseq->append(jmp_inst);
    } else if (match_hl(hl_opcode,HINS_cjmp_f)) {
      Operand dst = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);
      Operand label = hl_ins->get_operand(1);

      Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R11);
      Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
      clear_inst->set_comment("Clear tmp register");
      ll_iseq->append(clear_inst);

      temp = Operand(select_mreg_kind(1),MachineReg::MREG_R11);

      //MOVE Literal to TEMP
      LowLevelOpcode mov_t_opcode = select_ll_opcode(MINS_MOVB, 1);
      Instruction* mv_t_inst = new Instruction(mov_t_opcode, Operand(Operand::IMM_IVAL, 0), temp);
      mv_t_inst->set_comment("Moving dst to temp");
      ll_iseq->append(mv_t_inst);

      Instruction* cmp_inst = new Instruction(MINS_CMPB, temp, dst);
      cmp_inst->set_comment("Compare dst with 0");
      ll_iseq->append(cmp_inst);

      Instruction* jmp_inst = new Instruction(MINS_JE, label);
      jmp_inst->set_comment("jumping if to dst if false (dst == 0)");
      ll_iseq->append(jmp_inst);
    }
    return;
  }

  if (hl_opcode == HINS_localaddr) {
    Operand dst = get_ll_operand(hl_ins->get_operand(0), get_size(hl_opcode),ll_iseq);
    Operand immediate = get_ll_operand(hl_ins->get_operand(1), get_size(hl_opcode),ll_iseq);
    
    int mem_offset = -1*(m_data_base - immediate.get_imm_ival());
    Operand mem_ref = Operand(Operand::MREG64_MEM_OFF,MachineReg::MREG_RBP,mem_offset);

    Operand temp = Operand(select_mreg_kind(8),MachineReg::MREG_R12);

    //DO LEA
    Instruction* mv_inst = new Instruction(MINS_LEAQ, mem_ref, temp);
    mv_inst->set_comment("Load Address");
    ll_iseq->append(mv_inst);

    //MOVE temp to DST
    LowLevelOpcode mov_b_opcode = select_ll_opcode(MINS_MOVB, 8);
    Instruction* mv_b_inst = new Instruction(mov_b_opcode, temp, dst);
    mv_b_inst->set_comment("Moving temp to dst");
    ll_iseq->append(mv_b_inst);

    return;
  } 

  if (hl_opcode == HINS_call) {
    Operand label = hl_ins->get_operand(0);
    Instruction* mv_inst = new Instruction(MINS_CALL, label);
    mv_inst->set_comment("Calling Function");
    ll_iseq->append(mv_inst);

    return;
  }

  RuntimeError::raise("high level opcode %d not handled", int(hl_opcode));
}

// TODO: implement other private member functions
Operand LowLevelCodeGen::get_ll_operand(Operand hl_opcode, int size, std::shared_ptr<InstructionSequence> ll_iseq){
  if (hl_opcode.has_base_reg()){//assert we are passed a VR 
    if (hl_opcode.get_base_reg()>=10) {//standard VR
      int reg_index = hl_opcode.get_base_reg()-10;
      int mem_offset = -1*(m_register_base + 8*(reg_index+1));
      
      if (hl_opcode.get_kind() == Operand::VREG_MEM){
        Operand temp = Operand(select_mreg_kind(8),m_spare_regs[m_spare_reg]);
        Instruction* clear_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), Operand(Operand::IMM_IVAL,0), temp);
        clear_inst->set_comment("Clear tmp register");
        ll_iseq->append(clear_inst);

        Operand addr = Operand(Operand::MREG64_MEM_OFF,MachineReg::MREG_RBP,mem_offset);
        Instruction* mv_inst = new Instruction(select_ll_opcode(MINS_MOVB,8), addr, temp);
        mv_inst->set_comment("Move Address to tmp register");
        ll_iseq->append(mv_inst);
        Operand ret_op = Operand(Operand::MREG64_MEM,m_spare_regs[m_spare_reg]);
        m_spare_reg ++;
        m_spare_reg %= 6;
        return ret_op;
      }
      return Operand(Operand::MREG64_MEM_OFF,MachineReg::MREG_RBP,mem_offset);
    } else if (hl_opcode.get_base_reg()==0) { //passed return register
      return Operand(select_mreg_kind(size),MachineReg::MREG_RAX);
    } else { //passed an input register
      std::vector<MachineReg> args = {MREG_RDI,MREG_RSI,MREG_RDX,MREG_RCX,MREG_R8,MREG_R9};
      int reg_index = hl_opcode.get_base_reg() -1;
      return Operand(select_mreg_kind(size),args[reg_index]);
    }
  } else { //passed a literal memref
    return hl_opcode;
  }
}


int get_size(HighLevelOpcode opcode) {
    switch (opcode) {
        // 1-byte (8-bit) instructions
        case HINS_add_b: case HINS_sub_b: case HINS_mul_b: case HINS_div_b:
        case HINS_mod_b: case HINS_lshift_b: case HINS_rshift_b: case HINS_cmplt_b:
        case HINS_cmplte_b: case HINS_cmpgt_b: case HINS_cmpgte_b: case HINS_cmpeq_b:
        case HINS_cmpneq_b: case HINS_and_b: case HINS_or_b: case HINS_xor_b:
        case HINS_neg_b: case HINS_not_b: case HINS_compl_b: case HINS_inc_b:
        case HINS_dec_b: case HINS_mov_b: case HINS_spill_b: case HINS_restore_b:
            return 1;

        // 2-byte (16-bit) instructions
        case HINS_add_w: case HINS_sub_w: case HINS_mul_w: case HINS_div_w:
        case HINS_mod_w: case HINS_lshift_w: case HINS_rshift_w: case HINS_cmplt_w:
        case HINS_cmplte_w: case HINS_cmpgt_w: case HINS_cmpgte_w: case HINS_cmpeq_w:
        case HINS_cmpneq_w: case HINS_and_w: case HINS_or_w: case HINS_xor_w:
        case HINS_neg_w: case HINS_not_w: case HINS_compl_w: case HINS_inc_w:
        case HINS_dec_w: case HINS_mov_w: case HINS_spill_w: case HINS_restore_w:
            return 2;

        // 4-byte (32-bit) instructions
        case HINS_add_l: case HINS_sub_l: case HINS_mul_l: case HINS_div_l:
        case HINS_mod_l: case HINS_lshift_l: case HINS_rshift_l: case HINS_cmplt_l:
        case HINS_cmplte_l: case HINS_cmpgt_l: case HINS_cmpgte_l: case HINS_cmpeq_l:
        case HINS_cmpneq_l: case HINS_and_l: case HINS_or_l: case HINS_xor_l:
        case HINS_neg_l: case HINS_not_l: case HINS_compl_l: case HINS_inc_l:
        case HINS_dec_l: case HINS_mov_l: case HINS_spill_l: case HINS_restore_l:
            return 4;

        // 8-byte (64-bit) instructions
        case HINS_add_q: case HINS_sub_q: case HINS_mul_q: case HINS_div_q:
        case HINS_mod_q: case HINS_lshift_q: case HINS_rshift_q: case HINS_cmplt_q:
        case HINS_cmplte_q: case HINS_cmpgt_q: case HINS_cmpgte_q: case HINS_cmpeq_q:
        case HINS_cmpneq_q: case HINS_and_q: case HINS_or_q: case HINS_xor_q:
        case HINS_neg_q: case HINS_not_q: case HINS_compl_q: case HINS_inc_q:
        case HINS_dec_q: case HINS_mov_q: case HINS_spill_q: case HINS_restore_q:
            return 8;

        // Instructions with no source operand
        case HINS_ret:
        case HINS_jmp:
        case HINS_call:
        case HINS_enter:
        case HINS_leave:
        case HINS_localaddr:
        case HINS_cjmp_t:
        case HINS_cjmp_f:
        case HINS_nop:
            return 0;

        // Default case for unhandled opcodes
        default:
            return 0;
    }
}