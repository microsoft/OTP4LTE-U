/*********************************************************************************

MIT License

Copyright (c) 2016 Microsoft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*********************************************************************************/

#pragma once
#include "../DnlinkTX/macro_def.h"

// 36.213 Table 7.1.7.1 - 1

int mcs_table [32][3] = { 
	{0, PDSCH_MODULATION_QPSK, 0},
	{1, PDSCH_MODULATION_QPSK, 1},
	{2, PDSCH_MODULATION_QPSK, 2},
	{3, PDSCH_MODULATION_QPSK, 3},
	{4, PDSCH_MODULATION_QPSK, 4},
	{5, PDSCH_MODULATION_QPSK, 5},
	{6, PDSCH_MODULATION_QPSK, 6},
	{7, PDSCH_MODULATION_QPSK, 7},
	{8, PDSCH_MODULATION_QPSK, 8},
	{9, PDSCH_MODULATION_QPSK, 9},
	{10, PDSCH_MODULATION_16QAM, 9},
	{11, PDSCH_MODULATION_16QAM, 10},
	{12, PDSCH_MODULATION_16QAM, 11},
	{13, PDSCH_MODULATION_16QAM, 12},
	{14, PDSCH_MODULATION_16QAM, 13},
	{15, PDSCH_MODULATION_16QAM, 14},
	{16, PDSCH_MODULATION_16QAM, 15},
	{17, PDSCH_MODULATION_64QAM, 15},
	{18, PDSCH_MODULATION_64QAM, 16},
	{19, PDSCH_MODULATION_64QAM, 17},
	{20, PDSCH_MODULATION_64QAM, 18},
	{21, PDSCH_MODULATION_64QAM, 19},
	{22, PDSCH_MODULATION_64QAM, 20},
	{23, PDSCH_MODULATION_64QAM, 21},
	{24, PDSCH_MODULATION_64QAM, 22},
	{25, PDSCH_MODULATION_64QAM, 23},
	{26, PDSCH_MODULATION_64QAM, 24},
	{27, PDSCH_MODULATION_64QAM, 25},
	{28, PDSCH_MODULATION_64QAM, 26},
	{29, PDSCH_MODULATION_QPSK, -1},
	{30, PDSCH_MODULATION_16QAM, -1},
	{31, PDSCH_MODULATION_64QAM, -1}
};


// mcs_interval_table[i][0] and mcs_interval_table[i][1] hold the min and the max value of
// I_TBS from 36.213, Table 7.1.7.1 - 1 for modulation i, where 
// i == 0: QPSK
// i == 1: 16QAM
// i == 2: 64QAM
int mcs_interval_table[3][2] = { { 0, 9 }, { 9, 15 }, { 15, 26 } };
