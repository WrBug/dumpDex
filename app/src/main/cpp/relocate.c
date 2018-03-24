/*
relocate instruction
author: ele7enxxh
mail: ele7enxxh@qq.com
website: ele7enxxh.com
modified time: 2016-10-17
created time: 2015-01-17
*/

#include "relocate.h"

#define ALIGN_PC(pc)	(pc & 0xFFFFFFFC)

enum INSTRUCTION_TYPE {
	// B <label>
	B1_THUMB16,
	// B <label>
	B2_THUMB16,
	// BX PC
	BX_THUMB16,
	// ADD <Rdn>, PC (Rd != PC, Rn != PC) 在对ADD进行修正时，采用了替换PC为Rr的方法，当Rd也为PC时，由于之前更改了Rr的值，可能会影响跳转后的正常功能。
	ADD_THUMB16,
	// MOV Rd, PC
	MOV_THUMB16,
	// ADR Rd, <label>
	ADR_THUMB16,
	// LDR Rt, <label>
	LDR_THUMB16,

	// CB{N}Z <Rn>, <label>
	CB_THUMB16,


	// BLX <label>
	BLX_THUMB32,
	// BL <label>
	BL_THUMB32,
	// B.W <label>
	B1_THUMB32,
	// B.W <label>
	B2_THUMB32,
	// ADR.W Rd, <label>
	ADR1_THUMB32,
	// ADR.W Rd, <label>
	ADR2_THUMB32,
	// LDR.W Rt, <label>
	LDR_THUMB32,
	// TBB [PC, Rm]
	TBB_THUMB32,
	// TBH [PC, Rm, LSL #1]
	TBH_THUMB32,

	// BLX <label>
	BLX_ARM,
	// BL <label>
	BL_ARM,
	// B <label>
	B_ARM,
	// BX PC
	BX_ARM,
	// ADD Rd, PC, Rm (Rd != PC, Rm != PC) 在对ADD进行修正时，采用了替换PC为Rr的方法，当Rd也为PC时，由于之前更改了Rr的值，可能会影响跳转后的正常功能;实际汇编中没有发现Rm也为PC的情况，故未做处理。
	ADD_ARM,
	// ADR Rd, <label>
	ADR1_ARM,
	// ADR Rd, <label>
	ADR2_ARM,
	// MOV Rd, PC
	MOV_ARM,
	// LDR Rt, <label>
	LDR_ARM,

	UNDEFINE,
};

static int getTypeInThumb16(uint16_t instruction)
{
	if ((instruction & 0xF000) == 0xD000) {
		return B1_THUMB16;
	}
	if ((instruction & 0xF800) == 0xE000) {
		return B2_THUMB16;
	}
	if ((instruction & 0xFFF8) == 0x4778) {
		return BX_THUMB16;
	}
	if ((instruction & 0xFF78) == 0x4478) {
		return ADD_THUMB16;
	}
	if ((instruction & 0xFF78) == 0x4678) {
		return MOV_THUMB16;
	}
	if ((instruction & 0xF800) == 0xA000) {
		return ADR_THUMB16;
	}
	if ((instruction & 0xF800) == 0x4800) {
		return LDR_THUMB16;
	}
	if ((instruction & 0xF500) == 0xB100) {
		return CB_THUMB16;
	}
	return UNDEFINE;
}

static int getTypeInThumb32(uint32_t instruction)
{
	if ((instruction & 0xF800D000) == 0xF000C000) {
		return BLX_THUMB32;
	}
	if ((instruction & 0xF800D000) == 0xF000D000) {
		return BL_THUMB32;
	}
	if ((instruction & 0xF800D000) == 0xF0008000) {
		return B1_THUMB32;
	}
	if ((instruction & 0xF800D000) == 0xF0009000) {
		return B2_THUMB32;
	}
	if ((instruction & 0xFBFF8000) == 0xF2AF0000) {
		return ADR1_THUMB32;
	}
	if ((instruction & 0xFBFF8000) == 0xF20F0000) {
		return ADR2_THUMB32;		
	}
	if ((instruction & 0xFF7F0000) == 0xF85F0000) {
		return LDR_THUMB32;
	}
	if ((instruction & 0xFFFF00F0) == 0xE8DF0000) {
		return TBB_THUMB32;
	}
	if ((instruction & 0xFFFF00F0) == 0xE8DF0010) {
		return TBH_THUMB32;
	}
	return UNDEFINE;
}

static int getTypeInArm(uint32_t instruction)
{
	if ((instruction & 0xFE000000) == 0xFA000000) {
		return BLX_ARM;
	}
	if ((instruction & 0xF000000) == 0xB000000) {
		return BL_ARM;
	}
	if ((instruction & 0xF000000) == 0xA000000) {
		return B_ARM;
	}
	if ((instruction & 0xFF000FF) == 0x120001F) {
		return BX_ARM;
	}
	if ((instruction & 0xFEF0010) == 0x8F0000) {
		return ADD_ARM;
	}
	if ((instruction & 0xFFF0000) == 0x28F0000) {
		return ADR1_ARM;
	}
	if ((instruction & 0xFFF0000) == 0x24F0000) {
		return ADR2_ARM;		
	}
	if ((instruction & 0xE5F0000) == 0x41F0000) {
		return LDR_ARM;
	}
	if ((instruction & 0xFE00FFF) == 0x1A0000F) {
		return MOV_ARM;
	}
	return UNDEFINE;
}

static int relocateInstructionInThumb16(uint32_t pc, uint16_t instruction, uint16_t *trampoline_instructions)
{
	int type;
	int offset;
	
	type = getTypeInThumb16(instruction);
	if (type == B1_THUMB16 || type == B2_THUMB16 || type == BX_THUMB16) {
		uint32_t x;
		int top_bit;
		uint32_t imm32;
		uint32_t value;
		int idx;
		
		idx = 0;
		if (type == B1_THUMB16) {
			x = (instruction & 0xFF) << 1;
			top_bit = x >> 8;
			imm32 = top_bit ? (x | (0xFFFFFFFF << 8)) : x;
			value = pc + imm32;
			trampoline_instructions[idx++] = instruction & 0xFF00;
			trampoline_instructions[idx++] = 0xE003;	// B PC, #6
		}
		else if (type == B2_THUMB16) {
			x = (instruction & 0x7FF) << 1;
			top_bit = x >> 11;
			imm32 = top_bit ? (x | (0xFFFFFFFF << 11)) : x;
			value = pc + imm32;
		}
		else if (type == BX_THUMB16) {
			value = pc;
		}
		
		trampoline_instructions[idx++] = 0xF8DF;
		trampoline_instructions[idx++] = 0xF000;	// LDR.W PC, [PC]
		trampoline_instructions[idx++] = value & 0xFFFF;
		trampoline_instructions[idx++] = value >> 16;
		offset = idx;
	}
	else if (type == ADD_THUMB16) {
		int rdn;
		int rm;
		int r;
		
		rdn = ((instruction & 0x80) >> 4) | (instruction & 0x7);
		
		for (r = 7; ; --r) {
			if (r != rdn) {
				break;
			}
		}
		
		trampoline_instructions[0] = 0xB400 | (1 << r);	// PUSH {Rr}
		trampoline_instructions[1] = 0x4802 | (r << 8);	// LDR Rr, [PC, #8]
		trampoline_instructions[2] = (instruction & 0xFF87) | (r << 3);
		trampoline_instructions[3] = 0xBC00 | (1 << r);	// POP {Rr}
		trampoline_instructions[4] = 0xE002;	// B PC, #4
		trampoline_instructions[5] = 0xBF00;
		trampoline_instructions[6] = pc & 0xFFFF;
		trampoline_instructions[7] = pc >> 16;
		offset = 8;
	}
	else if (type == MOV_THUMB16 || type == ADR_THUMB16 || type == LDR_THUMB16) {
		int r;
		uint32_t value;
		
		if (type == MOV_THUMB16) {
			r = instruction & 0x7;
			value = pc;
		}
		else if (type == ADR_THUMB16) {
			r = (instruction & 0x700) >> 8;
			value = ALIGN_PC(pc) + (instruction & 0xFF) << 2;
		}
		else {
			r = (instruction & 0x700) >> 8;
			value = ((uint32_t *) (ALIGN_PC(pc) + ((instruction & 0xFF) << 2)))[0];
		}

		trampoline_instructions[0] = 0x4800 | (r << 8);	// LDR Rd, [PC]
		trampoline_instructions[1] = 0xE001;	// B PC, #2
		trampoline_instructions[2] = value & 0xFFFF;
		trampoline_instructions[3] = value >> 16;
		offset = 4;
	}
	else if (type == CB_THUMB16) {
		int nonzero;
		uint32_t imm32;
		uint32_t value;

		nonzero = (instruction & 0x800) >> 11;
		imm32 = ((instruction & 0x200) >> 3) | ((instruction & 0xF8) >> 2);
		value = pc + imm32 + 1;

		trampoline_instructions[0] = instruction & 0xFD07;
		trampoline_instructions[1] = 0xE003;	// B PC, #6
		trampoline_instructions[2] = 0xF8DF;
		trampoline_instructions[3] = 0xF000;	// LDR.W PC, [PC]
		trampoline_instructions[4] = value & 0xFFFF;
		trampoline_instructions[5] = value >> 16;
		offset = 6;
	}
	else {
		trampoline_instructions[0] = instruction;
		trampoline_instructions[1] = 0xBF00;  // NOP
		offset = 2;
	}
	
	return offset;
}

static int relocateInstructionInThumb32(uint32_t pc, uint16_t high_instruction, uint16_t low_instruction, uint16_t *trampoline_instructions)
{
	uint32_t instruction;
	int type;
	int idx;
	int offset;
	
	instruction = (high_instruction << 16) | low_instruction;
	type = getTypeInThumb32(instruction);
	idx = 0;
	if (type == BLX_THUMB32 || type == BL_THUMB32 || type == B1_THUMB32 || type == B2_THUMB32) {
		uint32_t j1;
		uint32_t j2;
		uint32_t s;
		uint32_t i1;
		uint32_t i2;
		uint32_t x;
		uint32_t imm32;
		uint32_t value;

		j1 = (low_instruction & 0x2000) >> 13;
		j2 = (low_instruction & 0x800) >> 11;
		s = (high_instruction & 0x400) >> 10;
		i1 = !(j1 ^ s);
		i2 = !(j2 ^ s);

		if (type == BLX_THUMB32 || type == BL_THUMB32) {
			trampoline_instructions[idx++] = 0xF20F;
			trampoline_instructions[idx++] = 0x0E09;	// ADD.W LR, PC, #9
		}
		else if (type == B1_THUMB32) {
			trampoline_instructions[idx++] = 0xD000 | ((high_instruction & 0x3C0) << 2);
			trampoline_instructions[idx++] = 0xE003;	// B PC, #6
		}
		trampoline_instructions[idx++] = 0xF8DF;
		trampoline_instructions[idx++] = 0xF000;	// LDR.W PC, [PC]
		if (type == BLX_THUMB32) {
			x = (s << 24) | (i1 << 23) | (i2 << 22) | ((high_instruction & 0x3FF) << 12) | ((low_instruction & 0x7FE) << 1);
			imm32 = s ? (x | (0xFFFFFFFF << 25)) : x;
			value = pc + imm32;
		}
		else if (type == BL_THUMB32) {
			x = (s << 24) | (i1 << 23) | (i2 << 22) | ((high_instruction & 0x3FF) << 12) | ((low_instruction & 0x7FF) << 1);
			imm32 = s ? (x | (0xFFFFFFFF << 25)) : x;
			value = pc + imm32 + 1;
		}
		else if (type == B1_THUMB32) {
			x = (s << 20) | (j2 << 19) | (j1 << 18) | ((high_instruction & 0x3F) << 12) | ((low_instruction & 0x7FF) << 1);
			imm32 = s ? (x | (0xFFFFFFFF << 21)) : x;
			value = pc + imm32 + 1;
		}
		else if (type == B2_THUMB32) {
			x = (s << 24) | (i1 << 23) | (i2 << 22) | ((high_instruction & 0x3FF) << 12) | ((low_instruction & 0x7FF) << 1);
			imm32 = s ? (x | (0xFFFFFFFF << 25)) : x;
			value = pc + imm32 + 1;
		}
		trampoline_instructions[idx++] = value & 0xFFFF;
		trampoline_instructions[idx++] = value >> 16;
		offset = idx;
	}
	else if (type == ADR1_THUMB32 || type == ADR2_THUMB32 || type == LDR_THUMB32) {
		int r;
		uint32_t imm32;
		uint32_t value;
		
		if (type == ADR1_THUMB32 || type == ADR2_THUMB32) {
			uint32_t i;
			uint32_t imm3;
			uint32_t imm8;
		
			r = (low_instruction & 0xF00) >> 8;
			i = (high_instruction & 0x400) >> 10;
			imm3 = (low_instruction & 0x7000) >> 12;
			imm8 = instruction & 0xFF;
			
			imm32 = (i << 31) | (imm3 << 30) | (imm8 << 27);
			
			if (type == ADR1_THUMB32) {
				value = ALIGN_PC(pc) + imm32;
			}
			else {
				value = ALIGN_PC(pc) - imm32;
			}
		}
		else {
			int is_add;
			uint32_t *addr;
			
			is_add = (high_instruction & 0x80) >> 7;
			r = low_instruction >> 12;
			imm32 = low_instruction & 0xFFF;
			
			if (is_add) {
				addr = (uint32_t *) (ALIGN_PC(pc) + imm32);
			}
			else {
				addr = (uint32_t *) (ALIGN_PC(pc) - imm32);
			}
			
			value = addr[0];
		}
		
		trampoline_instructions[0] = 0x4800 | (r << 8);	// LDR Rr, [PC]
		trampoline_instructions[1] = 0xE001;	// B PC, #2
		trampoline_instructions[2] = value & 0xFFFF;
		trampoline_instructions[3] = value >> 16;
		offset = 4;
	}

	else if (type == TBB_THUMB32 || type == TBH_THUMB32) {
		int rm;
		int r;
		int rx;
		
		rm = low_instruction & 0xF;
		
		for (r = 7;; --r) {
			if (r != rm) {
				break;
			}
		}
		
		for (rx = 7; ; --rx) {
			if (rx != rm && rx != r) {
				break;
			}
		}
		
		trampoline_instructions[0] = 0xB400 | (1 << rx);	// PUSH {Rx}
		trampoline_instructions[1] = 0x4805 | (r << 8);	// LDR Rr, [PC, #20]
		trampoline_instructions[2] = 0x4600 | (rm << 3) | rx;	// MOV Rx, Rm
		if (type == TBB_THUMB32) {
			trampoline_instructions[3] = 0xEB00 | r;
			trampoline_instructions[4] = 0x0000 | (rx << 8) | rx;	// ADD.W Rx, Rr, Rx
			trampoline_instructions[5] = 0x7800 | (rx << 3) | rx; 	// LDRB Rx, [Rx]
		}
		else if (type == TBH_THUMB32) {
			trampoline_instructions[3] = 0xEB00 | r;
			trampoline_instructions[4] = 0x0040 | (rx << 8) | rx;	// ADD.W Rx, Rr, Rx, LSL #1
			trampoline_instructions[5] = 0x8800 | (rx << 3) | rx; 	// LDRH Rx, [Rx]
		}
		trampoline_instructions[6] = 0xEB00 | r;
		trampoline_instructions[7] = 0x0040 | (r << 8) | rx;	// ADD Rr, Rr, Rx, LSL #1
		trampoline_instructions[8] = 0x3001 | (r << 8);	// ADD Rr, #1
		trampoline_instructions[9] = 0xBC00 | (1 << rx);	// POP {Rx}
		trampoline_instructions[10] = 0x4700 | (r << 3);	// BX Rr
		trampoline_instructions[11] = 0xBF00;
		trampoline_instructions[12] = pc & 0xFFFF;
		trampoline_instructions[13] = pc >> 16;
		offset = 14;
	}
	else {
		trampoline_instructions[0] = high_instruction;
		trampoline_instructions[1] = low_instruction;
		offset = 2;
	}

	return offset;
}

static void relocateInstructionInThumb(uint32_t target_addr, uint16_t *orig_instructions, int length, uint16_t *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count)
{
	int orig_pos;
	int trampoline_pos;
	uint32_t pc;
	uint32_t lr;

	orig_pos = 0;
	trampoline_pos = 0;
	pc = target_addr + 4;
	while (1) {
		int offset;

		orig_boundaries[*count] = orig_pos * sizeof(uint16_t);
		trampoline_boundaries[*count] = trampoline_pos * sizeof(uint16_t);
		++(*count);
		
		if ((orig_instructions[orig_pos] >> 11) >= 0x1D && (orig_instructions[orig_pos] >> 11) <= 0x1F) {
			if (orig_pos + 2 > length / sizeof(uint16_t)) {
				break;
			}
			offset = relocateInstructionInThumb32(pc, orig_instructions[orig_pos], orig_instructions[orig_pos + 1], &trampoline_instructions[trampoline_pos]);
			pc += sizeof(uint32_t);
			trampoline_pos += offset;
			orig_pos += 2;
		}
		else {
			offset = relocateInstructionInThumb16(pc, orig_instructions[orig_pos], &trampoline_instructions[trampoline_pos]);
			pc += sizeof(uint16_t);
			trampoline_pos += offset;
			++orig_pos;
		}
		
		if (orig_pos >= length / sizeof(uint16_t)) {
			break;
		}
	}


	
	lr = target_addr + orig_pos * sizeof(uint16_t) + 1;
	trampoline_instructions[trampoline_pos] = 0xF8DF;
	trampoline_instructions[trampoline_pos + 1] = 0xF000;	// LDR.W PC, [PC]
	trampoline_instructions[trampoline_pos + 2] = lr & 0xFFFF;
	trampoline_instructions[trampoline_pos + 3] = lr >> 16;
}

static void relocateInstructionInArm(uint32_t target_addr, uint32_t *orig_instructions, int length, uint32_t *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count)
{
	uint32_t pc;
	uint32_t lr;
	int orig_pos;
	int trampoline_pos;

	pc = target_addr + 8;
	lr = target_addr + length;

	trampoline_pos = 0;
	for (orig_pos = 0; orig_pos < length / sizeof(uint32_t); ++orig_pos) {
		uint32_t instruction;
		int type;

		orig_boundaries[*count] = orig_pos * sizeof(uint32_t);
		trampoline_boundaries[*count] = trampoline_pos * sizeof(uint32_t);
		++(*count);

		instruction = orig_instructions[orig_pos];
		type = getTypeInArm(instruction);
		if (type == BLX_ARM || type == BL_ARM || type == B_ARM || type == BX_ARM) {
			uint32_t x;
			int top_bit;
			uint32_t imm32;
			uint32_t value;

			if (type == BLX_ARM || type == BL_ARM) {
				trampoline_instructions[trampoline_pos++] = 0xE28FE004;	// ADD LR, PC, #4
			}
			trampoline_instructions[trampoline_pos++] = 0xE51FF004;  	// LDR PC, [PC, #-4]
			if (type == BLX_ARM) {
				x = ((instruction & 0xFFFFFF) << 2) | ((instruction & 0x1000000) >> 23);
			}
			else if (type == BL_ARM || type == B_ARM) {
				x = (instruction & 0xFFFFFF) << 2;
			}
			else {
				x = 0;
			}
			
			top_bit = x >> 25;
			imm32 = top_bit ? (x | (0xFFFFFFFF << 26)) : x;
			if (type == BLX_ARM) {
				value = pc + imm32 + 1;
			}
			else {
				value = pc + imm32;
			}
			trampoline_instructions[trampoline_pos++] = value;
			
		}
		else if (type == ADD_ARM) {
			int rd;
			int rm;
			int r;
			
			rd = (instruction & 0xF000) >> 12;
			rm = instruction & 0xF;
			
			for (r = 12; ; --r) {
				if (r != rd && r != rm) {
					break;
				}
			}
			
			trampoline_instructions[trampoline_pos++] = 0xE52D0004 | (r << 12);	// PUSH {Rr}
			trampoline_instructions[trampoline_pos++] = 0xE59F0008 | (r << 12);	// LDR Rr, [PC, #8]
			trampoline_instructions[trampoline_pos++] = (instruction & 0xFFF0FFFF) | (r << 16);
			trampoline_instructions[trampoline_pos++] = 0xE49D0004 | (r << 12);	// POP {Rr}
			trampoline_instructions[trampoline_pos++] = 0xE28FF000;	// ADD PC, PC
			trampoline_instructions[trampoline_pos++] = pc;
		}
		else if (type == ADR1_ARM || type == ADR2_ARM || type == LDR_ARM || type == MOV_ARM) {
			int r;
			uint32_t value;
			
			r = (instruction & 0xF000) >> 12;
			
			if (type == ADR1_ARM || type == ADR2_ARM || type == LDR_ARM) {
				uint32_t imm32;
				
				imm32 = instruction & 0xFFF;
				if (type == ADR1_ARM) {
					value = pc + imm32;
				}
				else if (type == ADR2_ARM) {
					value = pc - imm32;
				}
				else if (type == LDR_ARM) {
					int is_add;
					
					is_add = (instruction & 0x800000) >> 23;
					if (is_add) {
						value = ((uint32_t *) (pc + imm32))[0];
					}
					else {
						value = ((uint32_t *) (pc - imm32))[0];
					}
				}
			}
			else {
				value = pc;
			}
				
			trampoline_instructions[trampoline_pos++] = 0xE51F0000 | (r << 12);	// LDR Rr, [PC]
			trampoline_instructions[trampoline_pos++] = 0xE28FF000;	// ADD PC, PC
			trampoline_instructions[trampoline_pos++] = value;
		}
		else {
			trampoline_instructions[trampoline_pos++] = instruction;
		}
		pc += sizeof(uint32_t);
	}
	
	trampoline_instructions[trampoline_pos++] = 0xe51ff004;	// LDR PC, [PC, #-4]
	trampoline_instructions[trampoline_pos++] = lr;
}

void relocateInstruction(uint32_t target_addr, void *orig_instructions, int length, void *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count)
{
	if (target_addr & 1 == 1) {
		relocateInstructionInThumb(target_addr - 1, (uint16_t *) orig_instructions, length, (uint16_t *) trampoline_instructions, orig_boundaries, trampoline_boundaries, count);
	}
	else {
		relocateInstructionInArm(target_addr, (uint32_t *) orig_instructions, length, (uint32_t *) trampoline_instructions, orig_boundaries, trampoline_boundaries, count);
	}
}
