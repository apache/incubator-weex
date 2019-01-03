/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "backtrace.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
typedef uintptr_t unw_word_t;
typedef int unw_tdep_proc_info_t;
#define Debug(level, ...)

/* Error codes.  The unwind routines return the *negated* values of
   these error codes on error and a non-negative value on success.  */
typedef enum {
  UNW_ESUCCESS = 0, /* no error */
  UNW_EUNSPEC,      /* unspecified (general) error */
  UNW_ENOMEM,       /* out of memory */
  UNW_EBADREG,      /* bad register number */
  UNW_EREADONLYREG, /* attempt to write read-only register */
  UNW_ESTOPUNWIND,  /* stop unwinding */
  UNW_EINVALIDIP,   /* invalid IP */
  UNW_EBADFRAME,    /* bad frame */
  UNW_EINVAL,       /* unsupported operation or bad value */
  UNW_EBADVERSION,  /* unwind info has unsupported version */
  UNW_ENOINFO       /* no unwind info found */
} unw_error_t;

typedef enum {
  UNW_INFO_FORMAT_DYNAMIC,      /* unw_dyn_proc_info_t */
  UNW_INFO_FORMAT_TABLE,        /* unw_dyn_table_t */
  UNW_INFO_FORMAT_REMOTE_TABLE, /* unw_dyn_remote_table_t */
  UNW_INFO_FORMAT_ARM_EXIDX     /* ARM specific unwind info */
} unw_dyn_info_format_t;
#define DWARF_GET_LOC(l) ((l).val)

#define HIDDEN static
#define ARM_EXBUF_START(x) (((x) >> 4) & 0x0f)
#define ARM_EXBUF_COUNT(x) ((x)&0x0f)
#define ARM_EXBUF_END(x) (ARM_EXBUF_START (x) + ARM_EXBUF_COUNT (x))

#define ARM_EXIDX_CANT_UNWIND 0x00000001
#define ARM_EXIDX_COMPACT 0x80000000

#define ARM_EXTBL_OP_FINISH 0xb0
#define DWARF_NUM_PRESERVED_REGS 128
#define dwarf_to_unw_regnum(reg) (((reg) < 16) ? (reg) : 0)
#define DWARF_NULL_LOC DWARF_LOC (0, 0)
#define DWARF_IS_NULL_LOC(l) (DWARF_GET_LOC (l) == 0)
#define DWARF_LOC(r, t) ((dwarf_loc_t){.val = (r) })
#define DWARF_IS_REG_LOC(l) 0
#define DWARF_REG_LOC(c, r) (DWARF_LOC ((unw_word_t) (unw_word_t)&c[r], 0))
#define DWARF_MEM_LOC(c, m) DWARF_LOC ((m), 0)

typedef enum {
  UNW_ARM_R0,
  UNW_ARM_R1,
  UNW_ARM_R2,
  UNW_ARM_R3,
  UNW_ARM_R4,
  UNW_ARM_R5,
  UNW_ARM_R6,
  UNW_ARM_R7,
  UNW_ARM_R8,
  UNW_ARM_R9,
  UNW_ARM_R10,
  UNW_ARM_R11,
  UNW_ARM_R12,
  UNW_ARM_R13,
  UNW_ARM_R14,
  UNW_ARM_R15,

  /* VFPv2 s0-s31 (obsolescent numberings).  */
  UNW_ARM_S0 = 64,
  UNW_ARM_S1,
  UNW_ARM_S2,
  UNW_ARM_S3,
  UNW_ARM_S4,
  UNW_ARM_S5,
  UNW_ARM_S6,
  UNW_ARM_S7,
  UNW_ARM_S8,
  UNW_ARM_S9,
  UNW_ARM_S10,
  UNW_ARM_S11,
  UNW_ARM_S12,
  UNW_ARM_S13,
  UNW_ARM_S14,
  UNW_ARM_S15,
  UNW_ARM_S16,
  UNW_ARM_S17,
  UNW_ARM_S18,
  UNW_ARM_S19,
  UNW_ARM_S20,
  UNW_ARM_S21,
  UNW_ARM_S22,
  UNW_ARM_S23,
  UNW_ARM_S24,
  UNW_ARM_S25,
  UNW_ARM_S26,
  UNW_ARM_S27,
  UNW_ARM_S28,
  UNW_ARM_S29,
  UNW_ARM_S30,
  UNW_ARM_S31,

  /* FPA register numberings.  */
  UNW_ARM_F0 = 96,
  UNW_ARM_F1,
  UNW_ARM_F2,
  UNW_ARM_F3,
  UNW_ARM_F4,
  UNW_ARM_F5,
  UNW_ARM_F6,
  UNW_ARM_F7,

  /* iWMMXt GR register numberings.  */
  UNW_ARM_wCGR0 = 104,
  UNW_ARM_wCGR1,
  UNW_ARM_wCGR2,
  UNW_ARM_wCGR3,
  UNW_ARM_wCGR4,
  UNW_ARM_wCGR5,
  UNW_ARM_wCGR6,
  UNW_ARM_wCGR7,

  /* iWMMXt register numberings.  */
  UNW_ARM_wR0 = 112,
  UNW_ARM_wR1,
  UNW_ARM_wR2,
  UNW_ARM_wR3,
  UNW_ARM_wR4,
  UNW_ARM_wR5,
  UNW_ARM_wR6,
  UNW_ARM_wR7,
  UNW_ARM_wR8,
  UNW_ARM_wR9,
  UNW_ARM_wR10,
  UNW_ARM_wR11,
  UNW_ARM_wR12,
  UNW_ARM_wR13,
  UNW_ARM_wR14,
  UNW_ARM_wR15,

  /* Two-byte encodings from here on.  */

  /* SPSR.  */
  UNW_ARM_SPSR = 128,
  UNW_ARM_SPSR_FIQ,
  UNW_ARM_SPSR_IRQ,
  UNW_ARM_SPSR_ABT,
  UNW_ARM_SPSR_UND,
  UNW_ARM_SPSR_SVC,

  /* User mode registers.  */
  UNW_ARM_R8_USR = 144,
  UNW_ARM_R9_USR,
  UNW_ARM_R10_USR,
  UNW_ARM_R11_USR,
  UNW_ARM_R12_USR,
  UNW_ARM_R13_USR,
  UNW_ARM_R14_USR,

  /* FIQ registers.  */
  UNW_ARM_R8_FIQ = 151,
  UNW_ARM_R9_FIQ,
  UNW_ARM_R10_FIQ,
  UNW_ARM_R11_FIQ,
  UNW_ARM_R12_FIQ,
  UNW_ARM_R13_FIQ,
  UNW_ARM_R14_FIQ,

  /* IRQ registers.  */
  UNW_ARM_R13_IRQ = 158,
  UNW_ARM_R14_IRQ,

  /* ABT registers.  */
  UNW_ARM_R13_ABT = 160,
  UNW_ARM_R14_ABT,

  /* UND registers.  */
  UNW_ARM_R13_UND = 162,
  UNW_ARM_R14_UND,

  /* SVC registers.  */
  UNW_ARM_R13_SVC = 164,
  UNW_ARM_R14_SVC,

  /* iWMMXt control registers.  */
  UNW_ARM_wC0 = 192,
  UNW_ARM_wC1,
  UNW_ARM_wC2,
  UNW_ARM_wC3,
  UNW_ARM_wC4,
  UNW_ARM_wC5,
  UNW_ARM_wC6,
  UNW_ARM_wC7,

  /* VFPv3/Neon 64-bit registers.  */
  UNW_ARM_D0 = 256,
  UNW_ARM_D1,
  UNW_ARM_D2,
  UNW_ARM_D3,
  UNW_ARM_D4,
  UNW_ARM_D5,
  UNW_ARM_D6,
  UNW_ARM_D7,
  UNW_ARM_D8,
  UNW_ARM_D9,
  UNW_ARM_D10,
  UNW_ARM_D11,
  UNW_ARM_D12,
  UNW_ARM_D13,
  UNW_ARM_D14,
  UNW_ARM_D15,
  UNW_ARM_D16,
  UNW_ARM_D17,
  UNW_ARM_D18,
  UNW_ARM_D19,
  UNW_ARM_D20,
  UNW_ARM_D21,
  UNW_ARM_D22,
  UNW_ARM_D23,
  UNW_ARM_D24,
  UNW_ARM_D25,
  UNW_ARM_D26,
  UNW_ARM_D27,
  UNW_ARM_D28,
  UNW_ARM_D29,
  UNW_ARM_D30,
  UNW_ARM_D31,

  /* For ARM, the CFA is the value of SP (r13) at the call site in the
     previous frame.  */
  UNW_ARM_CFA,

  UNW_TDEP_LAST_REG = UNW_ARM_D31,

  UNW_TDEP_IP = UNW_ARM_R14, /* A little white lie.  */
  UNW_TDEP_SP = UNW_ARM_R13,
  UNW_TDEP_EH = UNW_ARM_R0 /* FIXME.  */
} arm_regnum_t;

typedef struct dwarf_loc
{
  unw_word_t val;
  unw_word_t type; /* see DWARF_LOC_TYPE_* macros.  */
} dwarf_loc_t;

typedef enum arm_exbuf_cmd {
  ARM_EXIDX_CMD_FINISH,
  ARM_EXIDX_CMD_DATA_PUSH,
  ARM_EXIDX_CMD_DATA_POP,
  ARM_EXIDX_CMD_REG_POP,
  ARM_EXIDX_CMD_REG_TO_SP,
  ARM_EXIDX_CMD_VFP_POP,
  ARM_EXIDX_CMD_WREG_POP,
  ARM_EXIDX_CMD_WCGR_POP,
  ARM_EXIDX_CMD_RESERVED,
  ARM_EXIDX_CMD_REFUSED,
} arm_exbuf_cmd_t;

enum arm_exbuf_cmd_flags
{
  ARM_EXIDX_VFP_SHIFT_16 = 1 << 16,
  ARM_EXIDX_VFP_DOUBLE = 1 << 17,
};

struct arm_exbuf_data
{
  arm_exbuf_cmd_t cmd;
  uint32_t data;
};

typedef struct unw_proc_info
{
  unw_word_t start_ip; /* first IP covered by this procedure */
  unw_word_t end_ip;   /* first IP NOT covered by this procedure */
  unw_word_t lsda;     /* address of lang.-spec. data area (if any) */
  unw_word_t handler;  /* optional personality routine */
  unw_word_t gp;       /* global-pointer value for this procedure */
  unw_word_t flags;    /* misc. flags */

  int format;                 /* unwind-info format (arch-specific) */
  int unwind_info_size;       /* size of the information (if applicable) */
  void *unwind_info;          /* unwind-info (arch-specific) */
  unw_tdep_proc_info_t extra; /* target-dependent auxiliary proc-info */
} unw_proc_info_t;

struct dwarf_cursor
{
  unw_proc_info_t pi;
  dwarf_loc_t loc[DWARF_NUM_PRESERVED_REGS];
  unw_word_t ip;  /* ip this rs is for */
  unw_word_t cfa; /* canonical frame address; aka frame-/stack-pointer */

  unsigned int pi_valid : 1; /* is proc_info valid? */
};

struct cursor
{
  struct dwarf_cursor dwarf;
};

#define prel31_to_addr(a, b, c, d) _prel31_to_addr (c, d)

static inline int
_prel31_to_addr (unw_word_t prel31, unw_word_t *val)
{
  unw_word_t offset;
  offset = *(unw_word_t *)prel31;
  offset = ((long)offset << 1) >> 1;
  *val = prel31 + offset;

  return 0;
}

static inline int
access_mem (unw_word_t addr, unw_word_t *val, int write)
{
  if (!write)
    {
      *val = *(unw_word_t *)addr;
    }
  else
    {
      *(unw_word_t *)addr = *val;
    }
  return 0;
}

static int
dwarf_get (struct dwarf_cursor *c, dwarf_loc_t loc, unw_word_t *val)
{
  *val = *(unw_word_t *)loc.val;
  return 0;
}

/**
 * Applies the given command onto the new state to the given dwarf_cursor.
 */
HIDDEN int
arm_exidx_apply_cmd (struct arm_exbuf_data *edata, struct dwarf_cursor *c)
{
  int ret = 0;
  unsigned i;

  switch (edata->cmd)
    {
    case ARM_EXIDX_CMD_FINISH:
      /* Set LR to PC if not set already.  */
      if (DWARF_IS_NULL_LOC (c->loc[UNW_ARM_R15]))
        c->loc[UNW_ARM_R15] = c->loc[UNW_ARM_R14];
      /* Set IP.  */
      dwarf_get (c, c->loc[UNW_ARM_R15], &c->ip);
      break;
    case ARM_EXIDX_CMD_DATA_PUSH:
      Debug (2, "vsp = vsp - %d\n", edata->data);
      c->cfa -= edata->data;
      break;
    case ARM_EXIDX_CMD_DATA_POP:
      Debug (2, "vsp = vsp + %d\n", edata->data);
      c->cfa += edata->data;
      break;
    case ARM_EXIDX_CMD_REG_POP:
      for (i = 0; i < 16; i++)
        if (edata->data & (1 << i))
          {
            Debug (2, "pop {r%d}\n", i);
            c->loc[UNW_ARM_R0 + i] = DWARF_LOC (c->cfa, 0);
            c->cfa += 4;
          }
      /* Set cfa in case the SP got popped. */
      if (edata->data & (1 << 13))
        dwarf_get (c, c->loc[UNW_ARM_R13], &c->cfa);
      break;
    case ARM_EXIDX_CMD_REG_TO_SP:
      assert (edata->data < 16);
      Debug (2, "vsp = r%d\n", edata->data);
      c->loc[UNW_ARM_R13] = c->loc[UNW_ARM_R0 + edata->data];
      dwarf_get (c, c->loc[UNW_ARM_R13], &c->cfa);
      break;
    case ARM_EXIDX_CMD_VFP_POP:
      /* Skip VFP registers, but be sure to adjust stack */
      for (i = ARM_EXBUF_START (edata->data); i <= ARM_EXBUF_END (edata->data);
           i++)
        c->cfa += 8;
      if (!(edata->data & ARM_EXIDX_VFP_DOUBLE))
        c->cfa += 4;
      break;
    case ARM_EXIDX_CMD_WREG_POP:
      for (i = ARM_EXBUF_START (edata->data); i <= ARM_EXBUF_END (edata->data);
           i++)
        c->cfa += 8;
      break;
    case ARM_EXIDX_CMD_WCGR_POP:
      for (i = 0; i < 4; i++)
        if (edata->data & (1 << i))
          c->cfa += 4;
      break;
    case ARM_EXIDX_CMD_REFUSED:
    case ARM_EXIDX_CMD_RESERVED:
      ret = -1;
      break;
    }
  return ret;
}

HIDDEN int
arm_exidx_decode (const uint8_t *buf, uint8_t len, struct dwarf_cursor *c)
{
#define READ_OP() *buf++
  const uint8_t *end = buf + len;
  int ret;
  struct arm_exbuf_data edata;

  assert (buf != NULL);
  assert (len > 0);

  while (buf < end)
    {
      uint8_t op = READ_OP ();
      if ((op & 0xc0) == 0x00)
        {
          edata.cmd = ARM_EXIDX_CMD_DATA_POP;
          edata.data = (((int)op & 0x3f) << 2) + 4;
        }
      else if ((op & 0xc0) == 0x40)
        {
          edata.cmd = ARM_EXIDX_CMD_DATA_PUSH;
          edata.data = (((int)op & 0x3f) << 2) + 4;
        }
      else if ((op & 0xf0) == 0x80)
        {
          uint8_t op2 = READ_OP ();
          if (op == 0x80 && op2 == 0x00)
            edata.cmd = ARM_EXIDX_CMD_REFUSED;
          else
            {
              edata.cmd = ARM_EXIDX_CMD_REG_POP;
              edata.data = ((op & 0xf) << 8) | op2;
              edata.data = edata.data << 4;
            }
        }
      else if ((op & 0xf0) == 0x90)
        {
          if (op == 0x9d || op == 0x9f)
            edata.cmd = ARM_EXIDX_CMD_RESERVED;
          else
            {
              edata.cmd = ARM_EXIDX_CMD_REG_TO_SP;
              edata.data = op & 0x0f;
            }
        }
      else if ((op & 0xf0) == 0xa0)
        {
          unsigned end = (op & 0x07);
          edata.data = (1 << (end + 1)) - 1;
          edata.data = edata.data << 4;
          if (op & 0x08)
            edata.data |= 1 << 14;
          edata.cmd = ARM_EXIDX_CMD_REG_POP;
        }
      else if (op == ARM_EXTBL_OP_FINISH)
        {
          edata.cmd = ARM_EXIDX_CMD_FINISH;
          buf = end;
        }
      else if (op == 0xb1)
        {
          uint8_t op2 = READ_OP ();
          if (op2 == 0 || (op2 & 0xf0))
            edata.cmd = ARM_EXIDX_CMD_RESERVED;
          else
            {
              edata.cmd = ARM_EXIDX_CMD_REG_POP;
              edata.data = op2 & 0x0f;
            }
        }
      else if (op == 0xb2)
        {
          uint32_t offset = 0;
          uint8_t byte, shift = 0;
          do
            {
              byte = READ_OP ();
              offset |= (byte & 0x7f) << shift;
              shift += 7;
            }
          while (byte & 0x80);
          edata.data = offset * 4 + 0x204;
          edata.cmd = ARM_EXIDX_CMD_DATA_POP;
        }
      else if (op == 0xb3 || op == 0xc8 || op == 0xc9)
        {
          edata.cmd = ARM_EXIDX_CMD_VFP_POP;
          edata.data = READ_OP ();
          if (op == 0xc8)
            edata.data |= ARM_EXIDX_VFP_SHIFT_16;
          if (op != 0xb3)
            edata.data |= ARM_EXIDX_VFP_DOUBLE;
        }
      else if ((op & 0xf8) == 0xb8 || (op & 0xf8) == 0xd0)
        {
          edata.cmd = ARM_EXIDX_CMD_VFP_POP;
          edata.data = 0x80 | (op & 0x07);
          if ((op & 0xf8) == 0xd0)
            edata.data |= ARM_EXIDX_VFP_DOUBLE;
        }
      else if (op >= 0xc0 && op <= 0xc5)
        {
          edata.cmd = ARM_EXIDX_CMD_WREG_POP;
          edata.data = 0xa0 | (op & 0x07);
        }
      else if (op == 0xc6)
        {
          edata.cmd = ARM_EXIDX_CMD_WREG_POP;
          edata.data = READ_OP ();
        }
      else if (op == 0xc7)
        {
          uint8_t op2 = READ_OP ();
          if (op2 == 0 || (op2 & 0xf0))
            edata.cmd = ARM_EXIDX_CMD_RESERVED;
          else
            {
              edata.cmd = ARM_EXIDX_CMD_WCGR_POP;
              edata.data = op2 & 0x0f;
            }
        }
      else
        edata.cmd = ARM_EXIDX_CMD_RESERVED;
      ret = arm_exidx_apply_cmd (&edata, c);
      if (ret < 0)
        return ret;
    }
  return 0;
}

/**
 * Reads the entry from the given cursor and extracts the unwind instructions
 * into buf.  Returns the number of the extracted unwind insns or
 * -UNW_ESTOPUNWIND if the special bit pattern ARM_EXIDX_CANT_UNWIND (0x1) was
 * found.
 */
HIDDEN int
arm_exidx_extract (struct dwarf_cursor *c, uint8_t *buf)
{
  int nbuf = 0;
  unw_word_t entry = (unw_word_t)c->pi.unwind_info;
  unw_word_t addr;
  uint32_t data;

  /* An ARM unwind entry consists of a prel31 offset to the start of a
     function followed by 31bits of data:
       * if set to 0x1: the function cannot be unwound (EXIDX_CANTUNWIND)
       * if bit 31 is one: this is a table entry itself (ARM_EXIDX_COMPACT)
       * if bit 31 is zero: this is a prel31 offset of the start of the
         table entry for this function  */
  if (prel31_to_addr (c->as, c->as_arg, entry, &addr) < 0)
    return -UNW_EINVAL;

  if (access_mem (entry + 4, &data, 0) < 0)
    return -UNW_EINVAL;

  if (data == ARM_EXIDX_CANT_UNWIND)
    {
      Debug (2, "0x1 [can't unwind]\n");
      nbuf = -UNW_ESTOPUNWIND;
    }
  else if (data & ARM_EXIDX_COMPACT)
    {
      Debug (2, "%p compact model %d [%8.8x]\n", (void *)addr,
             (data >> 24) & 0x7f, data);
      buf[nbuf++] = data >> 16;
      buf[nbuf++] = data >> 8;
      buf[nbuf++] = data;
    }
  else
    {
      unw_word_t extbl_data;
      unsigned int n_table_words = 0;

      if (prel31_to_addr (c->as, c->as_arg, entry + 4, &extbl_data) < 0)
        return -UNW_EINVAL;

      if (access_mem (extbl_data, &data, 0) < 0)
        return -UNW_EINVAL;

      if (data & ARM_EXIDX_COMPACT)
        {
          int pers = (data >> 24) & 0x0f;
          Debug (2, "%p compact model %d [%8.8x]\n", (void *)addr, pers, data);
          if (pers == 1 || pers == 2)
            {
              n_table_words = (data >> 16) & 0xff;
              extbl_data += 4;
            }
          else
            buf[nbuf++] = data >> 16;
          buf[nbuf++] = data >> 8;
          buf[nbuf++] = data;
        }
      else
        {
          unw_word_t pers;
          if (prel31_to_addr (c->as, c->as_arg, extbl_data, &pers) < 0)
            return -UNW_EINVAL;
          Debug (2, "%p Personality routine: %8p\n", (void *)addr,
                 (void *)pers);
          if (access_mem (extbl_data + 4, &data, 0) < 0)
            return -UNW_EINVAL;
          n_table_words = data >> 24;
          buf[nbuf++] = data >> 16;
          buf[nbuf++] = data >> 8;
          buf[nbuf++] = data;
          extbl_data += 8;
        }
      assert (n_table_words <= 5);
      unsigned j;
      for (j = 0; j < n_table_words; j++)
        {
          if (access_mem (extbl_data, &data, 0) < 0)
            return -UNW_EINVAL;
          extbl_data += 4;
          buf[nbuf++] = data >> 24;
          buf[nbuf++] = data >> 16;
          buf[nbuf++] = data >> 8;
          buf[nbuf++] = data >> 0;
        }
    }

  if (nbuf > 0 && buf[nbuf - 1] != ARM_EXTBL_OP_FINISH)
    buf[nbuf++] = ARM_EXTBL_OP_FINISH;

  return nbuf;
}

static int
tdep_search_unwind_table (unw_word_t first, unw_word_t last, unw_word_t ip,
                          unw_proc_info_t *pi)
{
  {
    /* The .ARM.exidx section contains a sorted list of key-value pairs -
       the unwind entries.  The 'key' is a prel31 offset to the start of a
       function.  We binary search this section in order to find the
       appropriate unwind entry.  */
    unw_word_t entry, val;

    if (prel31_to_addr (as, arg, first, &val) < 0 || ip < val)
      {
        return -UNW_ENOINFO;
      }

    if (prel31_to_addr (as, arg, last, &val) < 0)
      {
        return -UNW_EINVAL;
      }

    if (ip >= val)
      {
        entry = last;

        if (prel31_to_addr (as, arg, last, &pi->start_ip) < 0)
          return -UNW_EINVAL;

        pi->end_ip = 0xffffffff;
      }
    else
      {
        while (first < last - 8)
          {
            entry = first + (((last - first) / 8 + 1) >> 1) * 8;

            if (prel31_to_addr (as, arg, entry, &val) < 0)
              return -UNW_EINVAL;

            if (ip < val)
              last = entry;
            else
              first = entry;
          }

        entry = first;

        if (prel31_to_addr (as, arg, entry, &pi->start_ip) < 0)
          {
            return -UNW_EINVAL;
          }

        if (prel31_to_addr (as, arg, entry + 8, &pi->end_ip) < 0)
          {
            return -UNW_EINVAL;
          }

        pi->end_ip--;
      }

    {
      pi->unwind_info_size = 8;
      pi->unwind_info = (void *)entry;
      pi->format = UNW_INFO_FORMAT_ARM_EXIDX;
    }
    return 0;
  }

  return -UNW_ENOINFO;
}

extern unw_word_t dl_unwind_find_exidx(unw_word_t, int*);

HIDDEN int
tdep_find_proc_info (struct dwarf_cursor *c, unw_word_t ip,
                     int need_unwind_info)
{
  unw_word_t ptr;
  int count;
  unw_proc_info_t *pi;
  if (need_unwind_info == 0)
    return -UNW_ENOINFO;
  ptr = dl_unwind_find_exidx (ip, &count);
  if (!ptr)
    return -UNW_ENOINFO;
  pi = &c->pi;
  pi->start_ip = 0;
  pi->end_ip = 0xffffffff;
  return tdep_search_unwind_table (ptr, ptr + count * 8 - 8, ip, pi);
}

static inline int
arm_exidx_step (struct cursor *c)
{
  uint8_t buf[32];
  int ret;

  /* mark PC unsaved */
  c->dwarf.loc[UNW_ARM_R15] = DWARF_NULL_LOC;

  if ((ret = tdep_find_proc_info (&c->dwarf, c->dwarf.ip, 1)) < 0)
    return 0;

  if (c->dwarf.pi.format != UNW_INFO_FORMAT_ARM_EXIDX)
    return -UNW_ENOINFO;

  ret = arm_exidx_extract (&c->dwarf, buf);
  if (ret == -UNW_ESTOPUNWIND)
    return 0;
  else if (ret < 0)
    return ret;

  ret = arm_exidx_decode (buf, ret, &c->dwarf);
  if (ret < 0)
    return ret;

  c->dwarf.pi_valid = 0;

  return (c->dwarf.ip == 0) ? 0 : 1;
}

/* On ARM, we define our own unw_tdep_context instead of using ucontext_t.
   This allows us to support systems that don't support getcontext and
   therefore do not define ucontext_t.  */
typedef struct unw_tdep_context
{
  unsigned long regs[16];
} unw_tdep_context_t;

/* There is no getcontext() on ARM.  Use a stub version which only saves GP
   registers.  FIXME: Not ideal, may not be sufficient for all libunwind
   use cases.  Stores pc+8, which is only approximately correct, really.  */
#ifndef __thumb__
#define unw_tdep_getcontext(uc)                                               \
  (({                                                                         \
     unw_tdep_context_t *unw_ctx = (uc);                                      \
     register unsigned long *unw_base asm("r0") = unw_ctx->regs;              \
     __asm__ __volatile__("stmia %[base], {r0-r15}"                           \
                          :                                                   \
                          : [base] "r"(unw_base)                              \
                          : "memory");                                        \
   }),                                                                        \
   0)
#else /* __thumb__ */
#define unw_tdep_getcontext(uc)                                               \
  (({                                                                         \
     unw_tdep_context_t *unw_ctx = (uc);                                      \
     register unsigned long *unw_base asm("r0") = unw_ctx->regs;              \
     __asm__ __volatile__(".align 2\nbx pc\nnop\n.code 32\n"                  \
                          "stmia %[base], {r0-r15}\n"                         \
                          "orr %[base], pc, #1\nbx %[base]"                   \
                          : [base] "+r"(unw_base)                             \
                          :                                                   \
                          : "memory", "cc");                                  \
   }),                                                                        \
   0)
#endif

static int
init_dwarf (struct cursor *c, unw_tdep_context_t *uc, void* uc_mcontext)
{
  int ret, i;

  memcpy(uc->regs, &(((mcontext_t*)uc_mcontext)->arm_r0), sizeof(uc->regs));
  c->dwarf.loc[UNW_ARM_R0] = DWARF_REG_LOC (uc->regs, UNW_ARM_R0);
  c->dwarf.loc[UNW_ARM_R1] = DWARF_REG_LOC (uc->regs, UNW_ARM_R1);
  c->dwarf.loc[UNW_ARM_R2] = DWARF_REG_LOC (uc->regs, UNW_ARM_R2);
  c->dwarf.loc[UNW_ARM_R3] = DWARF_REG_LOC (uc->regs, UNW_ARM_R3);
  c->dwarf.loc[UNW_ARM_R4] = DWARF_REG_LOC (uc->regs, UNW_ARM_R4);
  c->dwarf.loc[UNW_ARM_R5] = DWARF_REG_LOC (uc->regs, UNW_ARM_R5);
  c->dwarf.loc[UNW_ARM_R6] = DWARF_REG_LOC (uc->regs, UNW_ARM_R6);
  c->dwarf.loc[UNW_ARM_R7] = DWARF_REG_LOC (uc->regs, UNW_ARM_R7);
  c->dwarf.loc[UNW_ARM_R8] = DWARF_REG_LOC (uc->regs, UNW_ARM_R8);
  c->dwarf.loc[UNW_ARM_R9] = DWARF_REG_LOC (uc->regs, UNW_ARM_R9);
  c->dwarf.loc[UNW_ARM_R10] = DWARF_REG_LOC (uc->regs, UNW_ARM_R10);
  c->dwarf.loc[UNW_ARM_R11] = DWARF_REG_LOC (uc->regs, UNW_ARM_R11);
  c->dwarf.loc[UNW_ARM_R12] = DWARF_REG_LOC (uc->regs, UNW_ARM_R12);
  c->dwarf.loc[UNW_ARM_R13] = DWARF_REG_LOC (uc->regs, UNW_ARM_R13);
  c->dwarf.loc[UNW_ARM_R14] = DWARF_REG_LOC (uc->regs, UNW_ARM_R14);
  c->dwarf.loc[UNW_ARM_R15] = DWARF_REG_LOC (uc->regs, UNW_ARM_R15);
  for (i = UNW_ARM_R15 + 1; i < DWARF_NUM_PRESERVED_REGS; ++i)
    c->dwarf.loc[i] = DWARF_NULL_LOC;

  ret = dwarf_get (&c->dwarf, c->dwarf.loc[UNW_ARM_R15], &c->dwarf.ip);
  if (ret < 0)
    return ret;

  /* FIXME: correct for ARM?  */
  ret = dwarf_get (&c->dwarf, c->dwarf.loc[UNW_ARM_R13], &c->dwarf.cfa);
  if (ret < 0)
    return ret;
  return ret;
}

int
mybacktrace (backtrace_callback callback, void *args, void* uc_mcontext)
{
  struct cursor c;
  unw_tdep_context_t uc;
  int ret;
  memset (&c, 0, sizeof (c));
  if (init_dwarf (&c, &uc, uc_mcontext) < 0)
    {
      return 1;
    }
  while (1)
    {
      if (arm_exidx_step (&c) == 0)
        {
          return 0;
        }
      if (BACKTRACE_CONTINUE != callback (c.dwarf.ip, args))
        {
          return 0;
        }
    }
  return 1;
}
