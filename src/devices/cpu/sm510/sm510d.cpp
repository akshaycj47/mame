// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510/SM500 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "sm510.h"


// common lookup tables

enum e_mnemonics
{
	// SM510 common
	mILL /* 0! */, mEXT,
	mLB, mLBL, mSBM, mEXBLA, mINCB, mDECB,
	mATPL, mRTN0, mRTN1, mTL, mTML, mTM, mT,
	mEXC, mBDC, mEXCI, mEXCD, mLDA, mLAX, mPTW, mWR, mWS,
	mKTA, mATBP, mATX, mATL, mATFC, mATR,
	mADD, mADD11, mADX, mCOMA, mROT, mRC, mSC,
	mTB, mTC, mTAM, mTMI, mTA0, mTABL, mTIS, mTAL, mTF1, mTF4,
	mRM, mSM,
	mPRE, mSME, mRME, mTMEL,
	mSKIP, mCEND, mIDIV, mDR, mDTA,

	// SM500-specific
	mCOMCB, mRTN, mRTNS, mSSR, mTR, mTRS,
	mADDC, mPDTW, mTW, mDTW,
	mATS, mEXKSA, mEXKFA,
	mRMF, mSMF, mCOMCN,
	mTA, mTM2, mTG,

	// KB1013VK1-2 aliases
	mLC, mLM, mLE, mLAF, mLAS, mLDF, mBS0, mBS1, mXL, mXM, mXI, mXEI, mXD, mXED, mXE, mBM0, mBM1, mSM1,
	mAM, mAC, mA10, mAS, mCLL, mCOM, mCLC, mSTC, mSCO, mSAO, mINC, mDEC, mSAM, mSAL, mNOP,
	mICD, mOAR, mOA0, mOA1, mDAF, mDAS, mABS, mABF, mCTB, mLD0, mEN,
	mBR, mLP, mCBR, mCMS, mRT, mRTS, mSI1, mSI0, mSYN, mTIM, mHLT
};

static const char *const s_mnemonics[] =
{
	"?", "",
	"LB", "LBL", "SBM", "EXBLA", "INCB", "DECB",
	"ATPL", "RTN0", "RTN1", "TL", "TML", "TM", "T",
	"EXC", "BDC", "EXCI", "EXCD", "LDA", "LAX", "PTW", "WR", "WS",
	"KTA", "ATBP", "ATX", "ATL", "ATFC", "ATR",
	"ADD", "ADD11", "ADX", "COMA", "ROT", "RC", "SC",
	"TB", "TC", "TAM", "TMI", "TA0", "TABL", "TIS", "TAL", "TF1", "TF4",
	"RM", "SM",
	"PRE", "SME", "RME", "TMEL",
	"SKIP", "CEND", "IDIV", "DR", "DTA",

	//
	"COMCB", "RTN", "RTNS", "SSR", "TR", "TRS",
	"ADDC", "PDTW", "TW", "DTW",
	"ATS", "EXKSA", "EXKFA",
	"RMF", "SMF", "COMCN",
	"TA", "TM", "TG"

	//
	"LC", "LM", "LE", "LAF", "LAS", "LDF", "BS0", "BS1", "XL", "XM", "XI", "XEI", "XD", "XED", "XE", "BM0", "BM1", "SM1",
	"AM", "AC", "A10", "AS", "CLL", "COM", "CLC", "STC", "SCO", "SAO", "INC", "DEC", "SAM", "SAL", "NOP",
	"ICD", "OAR", "OA0", "OA1", "DAF", "DAS", "ABS", "ABF", "CTB", "LD0", "EN",
	"BR", "LP", "CBR", "CMS", "RT", "RTS", "SI1", "SI0", "SYN", "TIM", "HLT"
};

// number of bits per opcode parameter, 8 or larger means 2-byte opcode
static const uint8_t s_bits[] =
{
	0, 8,
	4, 8, 0, 0, 0, 0,
	0, 0, 0, 4+8, 2+8, 6, 6,
	2, 0, 2, 2, 2, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	2, 2,
	8, 0, 0, 0,
	0, 0, 0, 0, 0,

	//
	0, 0, 0, 4, 6, 6,
	0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 2, 0,

	//
	4, 0, 2, 8, 4, 0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2,
	0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const uint32_t s_flags[] =
{
	0, 0,
	0, 0, 0, 0, 0, 0,
	0, _OUT, _OUT, 0, _OVER, _OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	0, 0, 0, 0,
	0, _OVER, 0, 0, 0,

	//
	0, _OUT, _OUT, 0, 0, _OVER,
	0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,

	//
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, _OVER, 0, _OUT, _OUT, 0, 0, 0, 0, _OVER
};

// next program counter in sequence (relative)
static const int8_t s_next_pc[0x40] =
{
	32, -1 /* rollback */, -1, 30, 30, -3, -3, 28, 28, -5, -5, 26, 26, -7, -7, 24,
	24, -9, -9, 22, 22, -11, -11, 20, 20, -13, -13, 18, 18, -15, -15, 16,
	16, -17, -17, 14, 14, -19, -19, 12, 12, -21, -21, 10, 10, -23, -23, 8,
	8, -25, -25, 6, 6, -27, -27, 4, 4, -29, -29, 2, 2, -31, -31, 0 /* gets stuck here */
};



// common disasm

static offs_t sm510_common_disasm(const uint8_t *lut_mnemonic, const uint8_t *lut_extended, std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram)
{
	// get raw opcode
	uint8_t op = oprom[0];
	uint8_t instr = lut_mnemonic[op];
	int len = 1;

	int bits = s_bits[instr];
	uint8_t mask = op & ((1 << (bits & 7)) - 1);
	uint16_t param = mask;
	if (bits >= 8)
	{
		// note: disasm view shows correct parameter, but raw view does not
		// note2: oprom array negative index doesn't work either :(
		param = oprom[s_next_pc[pc & 0x3f]];
		len++;
	}

	// extended opcode
	bool is_extended = (instr == mEXT);
	if (is_extended)
		instr = lut_extended[param];

	// disassemble it
	util::stream_format(stream, "%-6s ", s_mnemonics[instr]);
	if (bits > 0)
	{
		if (bits <= 4)
		{
			if (param < 10)
				util::stream_format(stream, "%d", param);
			else
				util::stream_format(stream, "$%X", param);
		}
		else if (bits <= 8)
		{
			if (!is_extended)
				util::stream_format(stream, "$%02X", param);
		}
		else
		{
			uint16_t address = (param << 4 & 0xc00) | (mask << 6 & 0x3c0) | (param & 0x03f);
			util::stream_format(stream, "$%03X", address);
		}

		// show param offset
		if (bits >= 8)
			util::stream_format(stream, " [$%03X]", pc + s_next_pc[pc & 0x3f]);
	}

	return len | s_flags[instr] | DASMFLAG_SUPPORTED;
}


// SM510 disasm

static const uint8_t sm510_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mSKIP, mATBP, mSBM,  mATPL, mRM,   mRM,   mRM,   mRM,   mADD,  mADD11,mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3 - note: $3A has synonym DC(decimal correct)

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	0,     mTB,   mTC,   mTAM,  mTMI,  mTMI,  mTMI,  mTMI,  mTIS,  mATL,  mTA0,  mTABL, 0,     mCEND, mTAL,  mLBL,  // 5
	mATFC, mATR,  mWR,   mWS,   mINCB, mIDIV, mRC,   mSC,   mTF1,  mTF4,  mKTA,  mROT,  mDECB, mBDC,  mRTN0, mRTN1, // 6
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTML,  mTML,  mTML,  mTML,  // 7

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 8
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 9
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // A
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // B

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // C
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // D
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // E
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM    // F
};

CPU_DISASSEMBLE(sm510)
{
	return sm510_common_disasm(sm510_mnemonic, nullptr, stream, pc, oprom, opram);
}


// SM511 disasm

static const uint8_t sm511_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mROT,  mDTA,  mSBM,  mATPL, mRM,   mRM,   mRM,   mRM,   mADD,  mADD11,mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	mKTA,  mTB,   mTC,   mTAM,  mTMI,  mTMI,  mTMI,  mTMI,  mTIS,  mATL,  mTA0,  mTABL, mATX,  mCEND, mTAL,  mLBL,  // 5
	mEXT,  mPRE,  mWR,   mWS,   mINCB, mDR,   mRC,   mSC,   mTML,  mTML,  mTML,  mTML,  mDECB, mPTW,  mRTN0, mRTN1, // 6
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   // 7

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 8
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 9
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // A
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // B

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // C
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // D
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // E
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM    // F
};

static const uint8_t sm511_extended[0x10] =
{
	mRME,  mSME,  mTMEL, mATFC, mBDC,  mATBP, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0      // 60 3
};

CPU_DISASSEMBLE(sm511)
{
	// create extended opcode table
	uint8_t ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x30, sm511_extended, 0x10);

	return sm510_common_disasm(sm511_mnemonic, ext, stream, pc, oprom, opram);
}


// SM500 disasm

static const uint8_t sm500_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mSKIP, mATR,  mEXKSA,mATBP, mRM,   mRM,   mRM,   mRM,   mADD,  mADDC, mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mATS,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	mTA,   mTB,   mTC,   mTAM,  mTM2,  mTM2,  mTM2,  mTM2,  mTG,   mPTW,  mTA0,  mTABL, mTW,   mDTW,  mEXT,  mLBL,  // 5
	mCOMCN,mPDTW, mWR,   mWS,   mINCB, mIDIV, mRC,   mSC,   mRMF,  mSMF,  mKTA,  mEXKFA,mDECB, mCOMCB,mRTN,  mRTNS, // 6
	mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  // 7

	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 8
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 9
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // A
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // B

	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // C
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // D
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // E
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS   // F
};

static const uint8_t sm500_extended[0x10] =
{
	mCEND, 0,     0,     0,     mDTA,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0      // 5E 0
};

CPU_DISASSEMBLE(sm500)
{
	// create extended opcode table
	uint8_t ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x00, sm500_extended, 0x10);

	return sm510_common_disasm(sm500_mnemonic, ext, stream, pc, oprom, opram);
}


// KB1013VK1-2 disasm

static const uint8_t kb1013vk12_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mNOP,  mOAR,  mBS1,  mEN,   mBM0,  mBM0,  mBM0,  mBM0,  mAM,   mAC,   mCOM,  mXL,   mBM1,  mBM1,  mBM1,  mBM1,  // 0
	mXM,   mXE,   mXE,   mXE,   mXI,   mXEI,  mXEI,  mXEI,  mLE,   mLE,   mLE,   mLDA,  mXD,   mXED,  mXED,  mXED,  // 1
	mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   mLC,   // 2
	mAS,   mAS,   mAS,   mAS,   mAS,   mAS,   mAS,   mAS,   mAS,   mAS,   mA10,  mAS,   mAS,   mAS,   mAS,   mAS,   // 3

	mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  mLAS,  // 4
	mSI1,  mSI0,  mSCO,  mSAM,  mSM1,  mSM1,  mSM1,  mSM1,  mTIM,  mABS,  mSAO,  mSAL,  mABF,  mDAF,  mEXT,  mLAF,  // 5
	mCTB,  mDAS,  mOA1,  mOA0,  mINC,  mSYN,  mCLC,  mSTC,  mCLL,  mLD0,  mICD,  mBS0,  mDEC,  mCMS,  mRT,   mRTS,  // 6
	mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   mLP,   // 7

	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   // 8
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   // 9
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   // A
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   // B

	mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  // C
	mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  // D
	mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  // E
	mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR,  mCBR   // F
};

static const uint8_t kb1013vk12_extended[0x10] =
{
	mHLT,  0,     0,     0,     mLDF,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0      // 5E 0
};

CPU_DISASSEMBLE(kb1013vk12)
{
	// create extended opcode table
	uint8_t ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x00, kb1013vk12_extended, 0x10);

	return sm510_common_disasm(kb1013vk12_mnemonic, ext, stream, pc, oprom, opram);
}
