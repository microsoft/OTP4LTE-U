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

function r = drs(ue, chs)
    % We could do this once (assume NCellID won't change)
    fss = mod(ue.NCellID, 30); % TODO: groupAssignmentPUSCH
    cinit = floor(ue.NCellID / 30) * 32 + fss;
    c = gold(cinit, 8*7*20 + 8);
    
    ns_0 = ue.NSubframe * 2; ns_1 = ns_0 + 1;
    nPRS_0 = 0; nPRS_1 = 0;
    for k = 0:7
        nPRS_0 = nPRS_0 + c(8*7*ns_0 + k + 1) * 2^k;
        nPRS_1 = nPRS_1 + c(8*7*ns_1 + k + 1) * 2^k;
    end
    ncs_0 = mod(nPRS_0, 12); % TODO: cyclicShift, last DCI cyclic shift
    ncs_1 = mod(nPRS_1, 12); % TODO: cyclicShift, last DCI cyclic shift
    alpha_0 = 2 * pi * ncs_0 / 12;
    alpha_1 = 2 * pi * ncs_1 / 12;
    NRB = length(chs.PRBSet);
    assert(NRB >= 3, 'Not implemented <3 PUSCH PRBs yet');
    nRsZcTable = [...
          11          23          31          47          59          71          83          89         107         113,...
         131         139         151         167         179         191         199         211         227         239,...
         251         263         271         283         293         311         317         331         347         359,...
         367         383         389         401         419         431         443         449         467         479,...
         491         503         509         523         523         547         563         571         587         599,...
         607         619         631         647         659         661         683         691         701         719,...
         727         743         751         761         773         787         797         811         827         839,...
         839         863         863         887         887         911         919         929         947         953,...
         971         983         991         997        1019        1031        1039        1051        1063        1069,...
        1091        1103        1109        1123        1129        1151        1163        1171        1187        1193,...
        1201        1223        1231        1237        1259        1259        1283        1291        1307        1319
        ];
    nRsZc = nRsZcTable(NRB);
    u = fss; % TODO: groupHoppingEnabled
    q_ = nRsZc * (u + 1) / 31;
    q = floor(q_ + 0.5); % TODO: sequenceHoppingEnabled
    r = zeros(NRB*12*2, 1);
    for n = 0:NRB*12-1
        m = mod(n, nRsZc);
        angle = -pi * q * m * (m+1) / nRsZc;
        r(n+1) = pol(angle + alpha_0 * n);
        r(NRB*12+n+1) = pol(angle + alpha_1 * n);
    end
    %assert(max(abs(r-ltePUSCHDRS(ue, chs))) < 3e-12);
    %assert(max(abs(r-bartosz_drs(ue, chs))) < 1e-12);
end

function c = pol(theta)
    c = complex(cos(theta), sin(theta));
end

function drs = bartosz_drs(ue, chs)
    pucchRefSigOpts.numSlotInFrame = ue.NSubframe * 2;
	pucchRefSigOpts.mSeqLengthRbs = size(chs.PRBSet, 1);
	pucchRefSigOpts.pciPhyCellId = ue.NCellID;
	pucchRefSigOpts.groupHoppingEnabled = false;
	pucchRefSigOpts.seqHoppingEnabled = false;
	pucchRefSigOpts.deltaSsSeqOffset = 0;
	pucchRefSigOpts.alCyclicShift = 0;
	pucchRefSigOpts.dciFormat = 0;
	pucchRefSigOpts.betaPusch = 1;
    a = ulGenDrsPuschSeqBW(pucchRefSigOpts).';
    pucchRefSigOpts.numSlotInFrame = ue.NSubframe * 2 + 1;
    b = ulGenDrsPuschSeqBW(pucchRefSigOpts).';
    drs = [a; b];
end