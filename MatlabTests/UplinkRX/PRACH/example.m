%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% MIT License
%
% Copyright (c) 2016 Microsoft
%
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included in
% all copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
% THE SOFTWARE.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%prachSettings = struct(...
%    'nUlRb', 50, 'frameNumber', 0, 'subFrameNumber', 1,...
%    'prachConfigurationIndex', 0, 'prachFrequencyOffset', 22,...
%    'rachRootSequence', 0, 'zeroCorrelationZoneConfig', 12, ...
%    'highSpeedFlag', 0, 'betaPrach', 1, 'nPreambles', 64);


% Preamble sequence index: between 0 and 63. Each preamble sequence is generated from its root sequence by applying a cyclic shift. 
% Sequences (indices) are divided into categories (contention based, group A, contention based, group B, non-contention based)

% rachRootSequence: logical root sequence number [0-837] defining the physical root sequence number <u> from Table 5.7.2-5
% We use rachRootSequence = 0, implying u = 129

% Preamble format [0-4]: Specifies duration and other PHY parameters, and defines range. 

% Frame structure type of random access [1 or 2] defines how many PRACH resources can we have  per sub frame. For type 1, preamble format 0-3 there can be at most one PRACH resource per subframe. 

% Prach configuration defines what preamble format is used and in which frame/subframe are PRACH resources allocated (Table 5.7.1-2)

% Frequency offset is the index of the first physical resource block allocated to PRACH, when pramble format is 0-3
% We use prachFrequencyOffset = 22 to center preamble around 0

% PRACH sequence length = 839 for formats 0-3

% highSpeedFlag - whether restricted (highSpeedFlag == 1, high mobility) or unrestricted (highSpeedFlag == 0, low mobility) set will be used. 
% We use unrestricted set (low mobility)

% Zero correlation zone config (N_CS) - determines the size of the cyclic shift and the number of preamble sequences that can be generated from each root sequence
% We use zeroCorrelationZoneConfig = 12, implying nCs = 119

% preambleNum denotes the index of the preamble. This corresponds to <v> parameter in the Zadoff Chu sequence in 5.7.2. 
% For any PRACH configuration there are always 64 preambles.  
% We use preambleNum = 0 for tests.





% We use:
% highSpeedFlag = 0                    => unrestricted
% zeroCorrelationZoneConfig = 12       => N_CS = 119 


nUlRb = 50;
preambleNum = 0;
prachConfigurationIndex = 0;            % Preamble format = 0, System frame = even, Subframe number = 1
prachFrequencyOffset = 22;              % Center (around 0)
rachRootSequence = 0;
zeroCorrelationZoneConfig = 12;
highSpeedFlag = 0;
betaPrach = 1;
nZc = 839;



% [ fdPrachSeq, tdPrachSeq ] = ulGenPrachSeqBW( bandwitdthRbs, numSlotInFrame, preambleNum, prachConfigurationIndex, prachFrequencyOffset, rachRootSequence, zeroCorrelationZoneConfig, highSpeedFlag, betaPrach)
% Generate slot long UL PRACH Sequence as per 3GPP
% 36.211 p5.7
%

nRbSc = 12;
deltaF = 15e3; %LTE carrier spacing
deltaFra = 1.25e3; %PRACH carrier spacing 3GPP 36.211 Table 5.7.3-1
phi = 7; %fixed offset 3GPP 36.211 Table 5.7.3-1

%3GPP 36.211 5.7.1 for formats 0..3
nRaPrb = prachFrequencyOffset;

%3GPP 36.211 5.7.3
K = deltaF / deltaFra;
k0 = nRaPrb * nRbSc - (nUlRb *nRbSc / 2);

%PRACH preamble format table 5.7.1-1
tCpTseqTable = [3168, 21024 6240 21024; 24576, 24576, 2*24576, 2*24576].';

%rach configuration table for formats 0-3 3GPP 5.7.1-2
%note SFN allocation is encoded 0 - even SFN, 1-any SFN,
%note allowed subframes have been encoded as a bitfield (0 not allowed, 1 -
%allowed, LSB - subframe 0, bit1 - subframe 1 and so on, Nan for not
%applicable
%[preambleFormat EvenAnySfn, allowedSubframes]
Even = 1;
Any = 0;
NA = 2^15;
prachCfgTable = [...
  0	Even	sum(2.^1);...
  0	Even	sum(2.^4);...
  0	Even	sum(2.^7);...
  0	Any	sum(2.^1);...
  0	Any	sum(2.^4);...
  0	Any	sum(2.^7);...
  0	Any	sum(2.^[1, 6]);...
  0	Any	sum(2.^[2 ,7]);...
  0	Any	sum(2.^[3, 8]);...
  0	Any	sum(2.^[1, 4, 7]);...
  0	Any	sum(2.^[2, 5, 8]);...
  0	Any	sum(2.^[3, 6, 9]);...
  0	Any	sum(2.^[0, 2, 4, 6, 8]);...
  0	Any	sum(2.^[1, 3, 5, 7, 9]);...
  0	Any	sum(2.^[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);...
  0	Even	sum(2.^9);...
  1	Even	sum(2.^1);...
  1	Even	sum(2.^4);...
  1	Even	sum(2.^7);...
  1	Any	sum(2.^1);...
  1	Any	sum(2.^4);...
  1	Any	sum(2.^7);...
  1	Any	sum(2.^[1, 6]);...
  1	Any	sum(2.^[2 ,7]);...
  1	Any	sum(2.^[3, 8]);...
  1	Any	sum(2.^[1, 4, 7]);...
  1	Any	sum(2.^[2, 5, 8]);...
  1	Any	sum(2.^[3, 6, 9]);...
  1	Any	sum(2.^[0, 2, 4, 6, 8]);...
  1	Any	sum(2.^[1, 3, 5, 7, 9]);...
  NA NA NA;...
  1	Even	sum(2.^9);...
  2	Even	sum(2.^1);...
  2	Even	sum(2.^4);...
  2	Even	sum(2.^7);...
  2	Any	sum(2.^1);...
  2	Any	sum(2.^4);...
  2	Any	sum(2.^7);...
  2	Any	sum(2.^[1, 6]);...
  2	Any	sum(2.^[2, 7]);...
  2	Any	sum(2.^[3, 8]);...
  2	Any	sum(2.^[1, 4, 7]);...
  2	Any	sum(2.^[2, 5, 8]);...
  2	Any	sum(2.^[3, 6, 9]);...
  2	Any	sum(2.^[0, 2, 4, 6, 8]);...
  2	Any	sum(2.^[1, 3, 5, 7, 9]);...
  NA NA NA;...
  2	Even	sum(2.^9);...
  3	Even	sum(2.^1);...
  3	Even	sum(2.^4);...
  3	Even	sum(2.^7);...
  3	Any	sum(2.^1);...
  3	Any	sum(2.^4);...
  3	Any	sum(2.^7);...
  3	Any	sum(2.^[1, 6]);...
  3	Any	sum(2.^[2 ,7]);...
  3	Any	sum(2.^[3, 8]);...
  3	Any	sum(2.^[1, 4, 7]);...
  3	Any	sum(2.^[2, 5, 8]);...
  3	Any	sum(2.^[3, 6, 9]);...
  NA NA NA;...
  NA NA NA;...
  NA NA NA;...
  3	Even	sum(2.^9)
  ];

%nCs tables
nCsUnrestrictedTable = [0 13 15 18 22 26 32 38 46 59 76 93 119 167 279 419];
nCsRestrictedTable = [15 18 22 26 32 38 46 55 68 82 100 128 158 202 237 NA];

%logical to physical root sequence number mapping, 3GPP table 5.7.2-4
phySeqNumTable = [...
  129, 710, 140, 699, 120, 719, 210, 629, 168, 671, 84, 755, 105, 734, 93, 746, 70, 769, 60, 779, 2, 837, 1, 838,...
  56, 783, 112, 727, 148, 691,...
  80, 759, 42, 797, 40, 799,...
  35, 804, 73, 766, 146, 693,...
  31, 808, 28, 811, 30, 809, 27, 812, 29, 810,...
  24, 815, 48, 791, 68, 771, 74, 765, 178, 661, 136, 703,...
  86, 753, 78, 761, 43, 796, 39, 800, 20, 819, 21, 818,...
  95, 744, 202, 637, 190, 649, 181, 658, 137, 702, 125, 714, 151, 688,...
  217, 622, 128, 711, 142, 697, 122, 717, 203, 636, 118, 721, 110, 729, 89, 750, 103, 736, 61, 778, 55, 784, 15, 824, 14, 825,...
  12, 827, 23, 816, 34, 805, 37, 802, 46, 793, 207, 632, 179, 660, 145, 694, 130, 709, 223, 616,...
  228, 611, 227, 612, 132, 707, 133, 706, 143, 696, 135, 704, 161, 678, 201, 638, 173, 666, 106, 733, 83, 756, 91, 748, 66, 773, 53, 786, 10, 829, 9, 830,...
  7, 832, 8, 831, 16, 823, 47, 792, 64, 775, 57, 782, 104, 735, 101, 738, 108, 731, 208, 631, 184, 655, 197, 642, 191, 648, 121, 718, 141, 698, 149, 690, 216, 623, 218, 621,...
  152, 687, 144, 695, 134, 705, 138, 701, 199, 640, 162, 677, 176, 663, 119, 720, 158, 681, 164, 675, 174, 665, 171, 668, 170, 669, 87, 752, 169, 670, 88, 751, 107, 732, 81, 758, 82, 757, 100, 739, 98, 741, 71, 768, 59, 780, 65, 774, 50, 789, 49, 790, 26, 813, 17, 822, 13, 826, 6, 833,...
  5, 834, 33, 806, 51, 788, 75, 764, 99, 740, 96, 743, 97, 742, 166, 673, 172, 667, 175, 664, 187, 652, 163, 676, 185, 654, 200, 639, 114, 725, 189, 650, 115, 724, 194, 645, 195, 644, 192, 647, 182, 657, 157, 682, 156, 683, 211, 628, 154, 685, 123, 716, 139, 700, 212, 627, 153, 686, 213, 626, 215, 624, 150, 689,...
  225, 614, 224, 615, 221, 618, 220, 619, 127, 712, 147, 692, 124, 715, 193, 646, 205, 634, 206, 633, 116, 723, 160, 679, 186, 653, 167, 672, 79, 760, 85, 754, 77, 762, 92, 747, 58, 781, 62, 777, 69, 770, 54, 785, 36, 803, 32, 807, 25, 814, 18, 821, 11, 828, 4, 835,...
  3, 836, 19, 820, 22, 817, 41, 798, 38, 801, 44, 795, 52, 787, 45, 794, 63, 776, 67, 772, 72,...
  767, 76, 763, 94, 745, 102, 737, 90, 749, 109, 730, 165, 674, 111, 728, 209, 630, 204, 635, 117, 722, 188, 651, 159, 680, 198, 641, 113, 726, 183, 656, 180, 659, 177, 662, 196, 643, 155, 684, 214, 625, 126, 713, 131, 708, 219, 620, 222, 617, 226, 613,...
  230, 609, 232, 607, 262, 577, 252, 587, 418, 421, 416, 423, 413, 426, 411, 428, 376, 463, 395, 444, 283, 556, 285, 554, 379, 460, 390, 449, 363, 476, 384, 455, 388, 451, 386, 453, 361, 478, 387, 452, 360, 479, 310, 529, 354, 485, 328, 511, 315, 524, 337, 502, 349, 490, 335, 504, 324, 515,...
  323, 516, 320, 519, 334, 505, 359, 480, 295, 544, 385, 454, 292, 547, 291, 548, 381, 458, 399, 440, 380, 459, 397, 442, 369, 470, 377, 462, 410, 429, 407, 432, 281, 558, 414, 425, 247, 592, 277, 562, 271, 568, 272, 567, 264, 575, 259, 580,...
  237, 602, 239, 600, 244, 595, 243, 596, 275, 564, 278, 561, 250, 589, 246, 593, 417, 422, 248, 591, 394, 445, 393, 446, 370, 469, 365, 474, 300, 539, 299, 540, 364, 475, 362, 477, 298, 541, 312, 527, 313, 526, 314, 525, 353, 486, 352, 487, 343, 496, 327, 512, 350, 489, 326, 513, 319, 520, 332, 507, 333, 506, 348, 491, 347, 492, 322, 517,...
  330, 509, 338, 501, 341, 498, 340, 499, 342, 497, 301, 538, 366, 473, 401, 438, 371, 468, 408, 431, 375, 464, 249, 590, 269, 570, 238, 601, 234, 605,...
  257, 582, 273, 566, 255, 584, 254, 585, 245, 594, 251, 588, 412, 427, 372, 467, 282, 557, 403, 436, 396, 443, 392, 447, 391, 448, 382, 457, 389, 450, 294, 545, 297, 542, 311, 528, 344, 495, 345, 494, 318, 521, 331, 508, 325, 514, 321, 518,...
  346, 493, 339, 500, 351, 488, 306, 533, 289, 550, 400, 439, 378, 461, 374, 465, 415, 424, 270, 569, 241, 598,...
  231, 608, 260, 579, 268, 571, 276, 563, 409, 430, 398, 441, 290, 549, 304, 535, 308, 531, 358, 481, 316, 523,...
  293, 546, 288, 551, 284, 555, 368, 471, 253, 586, 256, 583, 263, 576,...
  242, 597, 274, 565, 402, 437, 383, 456, 357, 482, 329, 510,...
  317, 522, 307, 532, 286, 553, 287, 552, 266, 573, 261, 578,...
  236, 603, 303, 536, 356, 483,...
  355, 484, 405, 434, 404, 435, 406, 433,...
  235, 604, 267, 572, 302, 537,...
  309, 530, 265, 574, 233, 606,...
  367, 472, 296, 543,...
  336, 503, 305, 534, 373, 466, 280, 559, 279, 560, 419, 420, 240, 599, 258, 581, 229, 610 ];


fdPrachSeq = [];
tdPrachSeq = [];


preambleFormat = prachCfgTable(1+prachConfigurationIndex,1);

%Generate Zadoff-Chu sequence with offset
nZc = 839; %3GPP 36.211, 5.7.2-1

%select nCs for signalled values
if (highSpeedFlag)
  nCs = nCsRestrictedTable(1+zeroCorrelationZoneConfig);
else
  nCs = nCsUnrestrictedTable(1+zeroCorrelationZoneConfig);
end;

%initial root sequence
logicalSeqNum = rachRootSequence;
u = phySeqNumTable(1+mod(logicalSeqNum,nZc-1));
v = preambleNum;

if (nCs == 0)
  cV = 0;
else
  if (highSpeedFlag) %3GPP 5.7.2
    vReadyFlag = 0;
    while (~vReadyFlag)
      %find p
      p = 0;
      while ( (p<=nZc) && ( mod(p*u,nZc) ~= 1) )
        p = p+1;
      end;
      
      if (2*p < nZc)
        dU = p;
      else
        dU = nZc - p;
      end;
      
      if ( (dU < nCs) || (2*dU > (nZc-nCs)) )
        %no cyclic shifts
        cShLimit = 1;
        cV = 0;
      else
        if ( (dU >= nCs) || (3*dU > nZc) )
          nRaShift = floor(dU/nCs);
          dStart = 2*dU + nRaShift * nCs;
          nRaGroup = floor(nZc / dStart);
          npRaShift = max(floor((nZc - 2*dU - nRaGroup*dStart)/nCs),0);
        else
          nRaShift = floor((nZc-2*dU)/nCs);
          dStart = nZc - 2*dU + nRaShift*nCs;
          nRaGroup = floor(dU/dStart);
          npRaShift = min( max( floor((dU - nRaGroup*dStart)/nCs)   , 0) ,nRaShift);
        end;
        
        %find max cyclic shift allowed
        cShLimit = max(nRaShift*nRaGroup + npRaShift, 1);
        cV = dStart*floor(v/nRaShift) + mod(v, nRaShift)*nCs;
      end;
      
      %find the correct root sequence for a given cyclic offset, trivial/skipped if
      %nCs low and only root sequence used
      if (v >= cShLimit) %proceed to next root index if cyclic shift exceeds range
        v = v - cShLimit;
        logicalSeqNum = logicalSeqNum+1;
        u = phySeqNumTable(1+mod(logicalSeqNum,nZc-1));
      else %output valid u and cV values
        vReadyFlag = 1; %effective break
      end;
    end;
  else
    %find the correct root sequence for a given cyclic offset, trivial/skipped if
    %nCs low and only root sequence used
    cShLimit = floor(nZc/nCs);
    while (v >= cShLimit)
      v = v - cShLimit;
      logicalSeqNum = logicalSeqNum+1;
      u = phySeqNumTable(1+mod(logicalSeqNum,nZc-1));
    end;
    cV = v*nCs;
  end;
end;

% x_u(n) in 5.7.2
mZadChu = 0:(nZc-1);
xuZadChu = exp( (-1i * pi * u * mZadChu .*(mZadChu+1)  ) / nZc);

% x_{u,v}(n) in 5.7.2
nSeqVec = mZadChu + cV;
prachSeq = xuZadChu(1+mod(nSeqVec, nZc));        % x_{u,v} from 36.211

%PRACH in freq domain
ifdPrachSeq = betaPrach * prachSeq;

%temporary, TODO implement time domain
%note preamble is generated at 1.92MHz, to keep fft/ifft size acceptable
fftSize = nZc;
ifftSize = 24576;          % 800us (preamble duration) * 30.72 MHz (LTE sampling rate)

%destination carrier offset, 3GPP 36.211 5.7.3
carrierOffset  =  phi + K/2 + K*k0; %13+K*k0
% first take a DFT, then distribute on correct carriers, then take an iDFT
% The goal of this is purely to shift the PRACH in frequency
% This is the inner sum in the equation in 5.7.3
fdPrachSeq = fft(ifdPrachSeq);

%apply carrier offset, modulo ifftSize
circBuf = [fdPrachSeq zeros(1, ifftSize - nZc)];
idxVec = mod((0:(ifftSize-1)) - carrierOffset, ifftSize);
ifftVec  = circBuf(1+idxVec);

%time domain sequence for formats 0 and 1, doubled for 2 and 3
tdPrachSeq2 = ifft(ifftVec);

%concatenate 2 identical preambles for formats 2 and 3
if ( (preambleFormat==2) || (preambleFormat==3) )
  tdPrachSeq2cc = [tdPrachSeq2 tdPrachSeq2];
else
  tdPrachSeq2cc = tdPrachSeq2;
end;

%add cyclic prefix and output
tdPrachSeq = [tdPrachSeq2cc( (end - tCpTseqTable(1+preambleFormat,1) + 1):end ), tdPrachSeq2cc];



% BOZIDAR: Why do we do this? Time shift by 54 samples?
fdPrachTable = zeros(nZc);
%note complex conjugate needed for derotation
fdPrachTable = exp(1i*(2*pi/nZc)* 54 *[-((nZc-1)/2):((nZc-1)/2)]) .* conj(fdPrachSeq); 


% Scale to complex16
fScaleMax = max(max(abs(fdPrachTable)));
fdPrachTable = int16(round( (32767/fScaleMax) * fdPrachTable ));



% Set up test example
preambleTable = loadPreambleTable();
%waveform = [tdPrachSeq, zeros(size(tdPrachSeq))];
longwaveform = [tdPrachSeq, tdPrachSeq2cc, tdPrachSeq2cc, tdPrachSeq2cc];
%longwaveform = [tdPrachSeq, zeros(size(tdPrachSeq2cc))];
%waveform = longwaveform((128*16):16:end);
%waveform = longwaveform(4*1024:16:end);
%waveform = longwaveform(4*1024:16:end);
waveform = longwaveform(4*1024:16:end);
%waveform = longwaveform(1024:16:end) + longwaveform(1:16:end-1023);
%waveform = longwaveform(1:16:end);

% Scale for fixed-poin ops
unsc_wave = waveform;
waveform = round(waveform * 32767 / max(abs(tdPrachSeq)));



% Detect preamble
tCp = 198;
%DEBUG: skip the whole prefix
%tCp = 198*2;
x = waveform(tCp/2 + (1:1536)); % step in half a cyclic prefix
unsc_x = unsc_wave(tCp/2 + (1:1536)); % step in half a cyclic prefix

% The fully sampled preamble has 24576 samples = 800us (preamble duration) * 30.72 MHz (LTE sampling rate)
% We sample at the smallest sampling frequency of 30.72 MHz / 16 = 1.92 MHz
% Thus, we take 800us (preamble duration) * 1.92 MHz (LTE sampling rate) = 1536 samples = 512 * 3

xF = fft3x512(x);
unsc_xF = fft(unsc_x);

xFs = [xF(1118:1536) xF(1:420)]; % fft shift and useful carrier extraction
postFftPwrScaleBits = 9; %3 bits per amplitude lost in fft, 6 bits in pwr
unsc_xFs = [unsc_xF(1118:1536) unsc_xF(1:420)]; 


%%% EXPERIMENT
%t = preambleTable(1+preambleNum,:);
shift = 1000; 
t = fdPrachSeq;
%t = t .* exp(1i*(2*pi/nZc)* (shift) *[1:nZc]); 
t(1:419) = t(1:419) .* exp(1i*(2*pi/1536)* (shift) *[(1118:1536)-1537]);
t(420:839) = t(420:839) .* exp(1i*(2*pi/1536)* (shift) *[0:1:419]); 
test = zeros(1536,1);
test(1:420) = t(end-419:end);
test(1118:1536) = t(1:419);
%test(1:420) = test(1:420) .* exp(1i*(2*pi/1536)* (shift) *[0:1:419]'); 
%test(1118:1536) = test(1118:1536) .* exp(1i*(2*pi/1536)* (shift) *[(1118:1536)-1537]'); 

% These two are the same
%figure; plot(1:1536, angle(test), 1:1536, angle(xF))

% These are the same
% % %figure; plot(1:1536, real(ifft(test)), 1:1536, real(x) / 3e4);
Y1 = ifft(test);
Y2 = 15*longwaveform( (shift+198)*16 + (1:16:(1536*16)) );
%figure; plot(1:1536, real(Y1), 1:1536, real(Y2));
%figure; plot(1:1536, angle(Y1), 1:1536, angle(Y2));
%%% END EXPERIMENT





bestDetVal = 3e5; %detection threshold, this is for ca 10dB SNR and good timing estimate
index = -inf; offset = inf;
% NOTE: zeroCorrelationZoneConfig = 12 implies N_CS = 119, which implies at most 7 preambles (0-6) 
% from one root sequence. Anything above preambleNum > 6 won't decode as it is generated with a different root sequence.
for preambleNum = 0:6
  % BOZIDAR: Here we really need an AGC for a complete implementation (instead of /2^10)
  % xD should be almost constant if xFs and preambleTable match in phase
  xD = floor((2^14 + xFs .* preambleTable(1+preambleNum,:))/(2^10)); %derotate preamble according to assumed sequence number
  % xD2 should be small if xD is almost constant
  xD2 = xD(2:end) - xD(1:(end-1)); %detection metric - channel estimate with minimum carrier-to carrier changes has smallest metric
  d = floor(2^20 * floor( xD2 * xD2' ) / max(1,floor( xD * xD' )) ); %64bit/32bit

  xDdiff = floor( (2^14 + xD(2:end) .*conj(xD(1:(end-1))))/(2^postFftPwrScaleBits) ); %work out angle change per sample
  angleTime = angle16bitBW( round(mean(xDdiff)) ); %average angle change
  o =  floor( (2^14 + 12288 * angleTime) / (2^15) ); %timing offset

  
  % Find the best angle and then correlate
  adj = (angle( round(mean(xDdiff)) ) ) / (2*pi/nZc);
  xFsP = xFs .* exp(1i*(2*pi/nZc)* (-adj) *[1:nZc]); 
  
  xDP = floor((2^14 + xFsP .* preambleTable(1+preambleNum,:))/(2^10)); %derotate preamble according to assumed sequence number
  xDP2 = xDP(2:end) - xDP(1:(end-1)); %detection metric - channel estimate with minimum carrier-to carrier changes has smallest metric
  dP = floor(2^20 * floor( xDP2 * xDP2' ) / max(1,floor( xDP * xDP' )) ); %64bit/32bit
  xDdiffP = floor( (2^14 + xDP(2:end) .*conj(xDP(1:(end-1))))/(2^postFftPwrScaleBits) ); %work out angle change per sample


  % DEBUG: accurate calc
  unsc_xDP = xFsP .* preambleTable(1+preambleNum,:);
  unsc_xDP2 = unsc_xDP(2:end) - unsc_xDP(1:(end-1)); 
  unsc_dP = 2^20 * ( unsc_xDP2 * unsc_xDP2' ) / max(1,floor( unsc_xDP * unsc_xDP' ));
  unsc_xDdiffP = unsc_xDP(2:end) .*conj(unsc_xDP(1:(end-1)));

  
  fprintf('%d %e %e %e %e %e %d %d %e %e\n', ...
          preambleNum, xD * xD', xD2 * xD2', mean(xD(2:end) .*conj(xD(1:(end-1)))), mean(xDdiff), angleTime, round(adj), o, d, dP);
  
  if (d < bestDetVal) && (abs(o) < (0.5*(30.72/1.92)*tCp))
    bestDetVal = d; offset = o; index = preambleNum;
  end
end
%assert(index >= 0); assert(offset < inf);

