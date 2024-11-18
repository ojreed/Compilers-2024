#include <cassert>
#include "vreg_allocator.h"
#include <cstdio>
#include <tuple>

VregAllocator::VregAllocator()
  : m_num_params(0)
  , m_top(0)
  , m_first_temp(0)
  , m_params_ended(false)
  , m_temps_active(false) {
}

VregAllocator::~VregAllocator() {
}

void VregAllocator::reset() {
  m_num_params = 0;
  m_top = 0;
  m_first_temp = 0;
  m_params_ended = false;
  m_temps_active = false;
}


int VregAllocator::alloc_param() {
  assert(!m_params_ended);
  int vreg = m_top;
  m_top++;
  return vreg;
}

void VregAllocator::end_params() {
  assert(!m_params_ended);
  m_params_ended = true;
}

std::tuple<int, int> VregAllocator::enter_block() {
  m_first_temp = m_top;
  auto x = std::make_tuple(m_top, m_reg_count);
  m_reg_count = 0;
  return x;
}

void VregAllocator::leave_block(int mark, int reg_count) {
  m_top = mark;
  m_reg_count = reg_count;
}

int VregAllocator::alloc_local() {
  int vreg = m_top;
  m_top++;
  m_first_temp = m_top;
  m_reg_count += 1;
  return vreg;
}

void VregAllocator::begin_statement() {
  m_top = m_first_temp;
}

int VregAllocator::alloc_temp() {
  int vreg = m_top;
  m_top++;
  return vreg;
}
