/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _OPCODES_H
#define _OPCODES_H

typedef unsigned char OPCODE;

enum chip16_opcodes {
	NOP =	0x00, CLS, VBLNK, BGC, SPR, DRW_I, DRW_R, RND, FLIP, SND0, SND1, SND2, SND3,
	JMP_I = 0x10, JMC, Jx, JME, CALL_I, RET, JMP_R, Cx, CALL_R,
	LDI_R = 0x20, LDI_SP, LDM_I, LDM_R, MOV,
	STM_I = 0x30, STM_R,
	ADDI =	0x40, ADD_R2, ADD_R3,
	SUBI =	0x50, SUB_R2, SUB_R3, CMPI, CMP,
	ANDI =	0x60, AND_R2, AND_R3, TSTI, TST,
	ORI =	0x70, OR_R2, OR_R3,
	XORI =	0x80, XOR_R2, XOR_R3,
	MULI =	0x90, MUL_R2, MUL_R3,
	DIVI =	0xA0, DIV_R2, DIV_R3,
	SHL_N = 0xB0, SAL_N = 0xB0, SHR_N, SAR_N, SHL_R, SAL_R=0xB3, SHR_R, SAR_R,
	PUSH =	0xC0, POP, PUSHALL, POPALL,
	// Pseudo-opcodes
	DB =	0xFA, DB_STR
};

#endif
