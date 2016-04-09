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

function toolbox
clear
warning('off','lte:defaultValue')

f = fopen('test_fixed_1.bin', 'rb');
d = fread(f, Inf, 'int16');
d = d(1:2:end) + 1i*d(2:2:end);
fclose(f);
%start = 128985;
d = resample(d, 1, 2); % captured at 30.72 MHz, but NDLRB=50 implies 15.36 MHz
framelen = 10*(2048*7+144*6+160);

enb.NDLRB = 50;
%[cellid, offset] = lteCellSearch(enb, data)
for sfn = 0:(size(d)/framelen-2)
    cellid = 11; offset = 64492 + sfn*framelen;
    enb.NCellID = cellid;
    data = d(offset+(1:framelen));

    foffset = lteFrequencyOffset(enb, data);
    data = lteFrequencyCorrect(enb, data, foffset);

    grid = lteOFDMDemodulate(enb, data);
    frame = grid(:,1:14*10);
    set(surf(abs(frame)),'edgecolor','none'); view(2);

    enb.CellRefP = 1;
    enb.NSubframe = 0;
    sf = frame(:,(enb.NSubframe*14)+(1:14));
    [hest,noiseest] = lteDLChannelEstimate(enb,sf);
    %[bits,symbols,nfmod4,trblk,cellrefp] = ltePBCHDecode(enb, grid(ltePBCHIndices(enb)));
    [bits,symbols,nfmod4,trblk,cellrefp] = ltePBCHDecode(enb, sf(ltePBCHIndices(enb)), hest(ltePBCHIndices(enb)), noiseest);
    if cellrefp == 0
        fprintf('Could not read MIB %g\n', noiseest);
        continue
    end
    enb = lteMIB(trblk);

    enb.NCellID = cellid;
    enb.CellRefP = cellrefp;
    enb.NFrame = enb.NFrame + nfmod4;

    % RAR
    enb.NSubframe = 6;
    sf = frame(:,(enb.NSubframe*14)+(1:14));
    [hest,noiseest] = lteDLChannelEstimate(enb,sf);
    cfiBits = ltePCFICHDecode(enb, sf(ltePCFICHIndices(enb)), hest, noiseest);
    enb.CFI = lteCFIDecode(cfiBits);
    ue = struct('RNTI',2);
    pdcchBits = ltePDCCHDecode(enb, sf(ltePDCCHIndices(enb)), hest(ltePDCCHIndices(enb)), noiseest);
    dcistr = ltePDCCHSearch(enb, ue, pdcchBits);
    dci = dcistr{1};
    [chs,trblklen] = hPDSCHConfiguration(enb, dci, ue.RNTI);
    sibIndices = ltePDSCHIndices(enb, chs, chs.PRBSet);
    cws = ltePDSCHDecode(enb, chs, sf(sibIndices), hest(sibIndices), noiseest);
    [trblkout, crc] = lteDLSCHDecode(enb, chs, trblklen, cws);
    fprintf('Frame %d RAR = %s %g\n', enb.NFrame, bin2hex(trblkout{1}), noiseest);

    % SIB1
    if mod(enb.NFrame, 2) == 0
        enb.NSubframe = 5;
        sf = frame(:,(enb.NSubframe*14)+(1:14));
        [hest,noiseest] = lteDLChannelEstimate(enb,sf);
        cfiBits = ltePCFICHDecode(enb, sf(ltePCFICHIndices(enb)), hest, noiseest);
        enb.CFI = lteCFIDecode(cfiBits);
        ue = struct('RNTI',65535);
        pdcchBits = ltePDCCHDecode(enb, sf(ltePDCCHIndices(enb)), hest(ltePDCCHIndices(enb)), noiseest);
        dcistr = ltePDCCHSearch(enb, ue, pdcchBits);
        dci = dcistr{1};
        [chs,trblklen] = hPDSCHConfiguration(enb, dci, ue.RNTI);
        sibIndices = ltePDSCHIndices(enb, chs, chs.PRBSet);
        cws = ltePDSCHDecode(enb, chs, sf(sibIndices), hest(sibIndices), noiseest);
        [trblkout, crc] = lteDLSCHDecode(enb, chs, trblklen, cws);
        fprintf('Frame %d SIB1 = %s\n', enb.NFrame, bin2hex(trblkout{1}));
    end

    % SIB2
    if mod(enb.NFrame, 8) == 0
        enb.NSubframe = 0;
        sf = frame(:,(enb.NSubframe*14)+(1:14));
        [hest,noiseest] = lteDLChannelEstimate(enb,sf);
        cfiBits = ltePCFICHDecode(enb, sf(ltePCFICHIndices(enb)), hest, noiseest);
        enb.CFI = lteCFIDecode(cfiBits);
        ue = struct('RNTI',65535);
        pdcchBits = ltePDCCHDecode(enb, sf(ltePDCCHIndices(enb)), hest(ltePDCCHIndices(enb)), noiseest);
        dcistr = ltePDCCHSearch(enb, ue, pdcchBits);
        dci = dcistr{1};
        [chs,trblklen] = hPDSCHConfiguration(enb, dci, ue.RNTI);
        sibIndices = ltePDSCHIndices(enb, chs, chs.PRBSet);
        cws = ltePDSCHDecode(enb, chs, sf(sibIndices), hest(sibIndices), noiseest);
        [trblkout, crc] = lteDLSCHDecode(enb, chs, trblklen, cws);
        fprintf('Frame %d SIB2 = %s\n', enb.NFrame, bin2hex(trblkout{1}));
    end
end
end

function str = bin2hex(bin)
    dec = [8 4 2 1] * reshape(double(bin), 4, []);
    str = sprintf('%x', dec);
end