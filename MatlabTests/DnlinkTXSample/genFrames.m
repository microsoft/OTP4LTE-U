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

function genFrames()
global PLOT_CONST;
global PLOT_CHANNEL;
PLOT_CONST = 0;
PLOT_FRAME = 0;
PLOT_CHANNEL = 0;

    mode = 'VERIFY_FILE'; % DUMP/VERIFY/VERIFY_FILE

    switch mode
        case 'DUMP'
            f = fopen([mfilename('fullpath') '.dat'], 'w');
            for frame = 0:1023
                for subframe = 0:9
                    tx = genSubframe(frame, subframe);
                    m = max(abs([real(tx.waveform); imag(tx.waveform)]));
                    w = int16(32767 * (tx.waveform / m)).';
                    fwrite(f, [real(w); imag(w)], 'int16');
                end
            end
            fclose(f);
             
        case 'VERIFY_FILE'
            %f = fopen('out_lte_3rd_30_72_new.bin'); fs = 30.72e6; offset = 241778; period = 307200;
            %f = fopen('out_lte_3rd_30_72_new_2.bin'); fs = 30.72e6; offset = 285114; period = 307200;
            %f = fopen('out_lte_3rd_30_72_new_3.bin'); fs = 30.72e6; offset = 186486; period = 307200;

            
            % Error reports for F:\Code\SDR\Samples\rxSampleDL\out_lte_m1_2nd.bin
            % Rx 0168.00 Start
            % Rx 0169.06 (signal error)
            % Rx 0193.09 (timing error - advance timing by 12 samples)
            % Rx 0216.09 - end
            %f = fopen('F:\Code\SDR\Samples\rxSampleDL\out_lte_m1_2nd.bin'); fs = 30.72e6; offset = 39516 + 1*307200; period = 307200;
            %f = fopen('F:\Code\SDR\Samples\rxSampleDL\out_lte_m1_2nd.bin'); fs = 30.72e6; offset = 39516 + 11 + 26*307200; period = 307200;
            
            % f = fopen('F:\Code\SDR\Samples\rxSampleDL\out_lte_old_working_2nd_3.bin'); fs = 30.72e6; offset = 74052; period = 307200;
            f = fopen('F:\Code\SDR\Samples\rxSampleDL\out_lte_new_old2nd_2.bin'); fs = 30.72e6; offset = 158808 + 2*307200; period = 307200;

            %f = fopen('F:\Code\SDR\Samples\rxSampleDL\out_lte_old_old2nd_2.bin'); fs = 30.72e6; offset = 899414 + (3)*307200; period = 307200;
            % 0*, 3*
            
            b = double(fread(f, [2 30720*10*50], 'int16'));
            fclose(f);
            %cs = b(1,:) + 1j*b(2,:);
            
            % DEBUG
            cs = (b(1,:) + 1j*b(2,:)) * exp(i* pi/4);
            
            ts = downsampleBW(cs, fs, 30.72e6, 'interp');
            nFrames = floor((length(ts) - offset) / period);
            
            % DEBUG
            nFrames = 1;
            
            c = ts(offset+(1:nFrames*period));

            PLOT_FRAME = 1;
            if PLOT_FRAME
              figure(1);
              clf(1);
              lenPlotFrames = 3;
              plotFrame(ts(offset+(1:lenPlotFrames*period)));
            end

            % For plotting constelations
            PLOT_CONST = 1;
            if PLOT_CONST
              figure(2);
              clf(2);
              hold on;
            end
            
            % For plotting channel estimations
            PLOT_CHANNEL = 1;
            if PLOT_CHANNEL
              figure(3);
              clf(3);
              hold on;
            end
            
            frames = reshape(c, period, []);
            ue_state = [];
            for frame = 1:nFrames
                for subframe = 0:9
                    tx.nFFT = 2048; %tx = genSubframe(0, subframe);
                    tx.waveform = frames(30720*subframe+(1:30720), frame);
                    ue_state = receiveSubframe(tx, ue_state);
                end
            end
            
        case 'VERIFY'
            ue_state = [];
            for frame = 0:0
                for subframe = 0:9
                    tx = genSubframe(frame, subframe);
                    ue_state = receiveSubframe(tx, ue_state);
                end
            end
    end
end

function tx = genSubframe(frame, subframe)
    tx.enb = struct('CellRefP',1,'NDLRB',50,'NCellID',11,'NFrame',frame,...
    'NSubframe',subframe,'Ng',1,'PHICHDuration','Normal','CFI',3);
    tx.Nid1 = floor(tx.enb.NCellID / 3); tx.Nid2 = mod(tx.enb.NCellID, 3);

    % Scheduling. SIB1 fixed in spec, we set SIB2 period in SIB1
    tx_pss = mod(tx.enb.NSubframe, 5) == 0;
    tx_mib = tx.enb.NSubframe == 0;
    tx_sib1 = mod(tx.enb.NFrame, 2) == 0 && tx.enb.NSubframe == 5;
    tx_sib2 = mod(tx.enb.NFrame, 8) == 0 && tx.enb.NSubframe == 0;

    % CRS
    tx.grid = cellRS(tx.enb.NCellID, tx.enb.NSubframe, tx.enb.NDLRB, tx.enb.CellRefP);

    if tx_pss
        % SSS
        tx.grid(pssSubcarriers(tx.enb.NDLRB), 6) = sss(tx.Nid1, tx.Nid2, tx.enb.NSubframe);

        % PSS
        tx.grid(pssSubcarriers(tx.enb.NDLRB), 7) = pss(tx.Nid2);
    end
    
    if tx_mib
        % BCH
        tx.mib = struct('NDLRB',tx.enb.NDLRB,'NFrameDiv4',floor(tx.enb.NFrame/4),...
                        'Ng',tx.enb.Ng,'PHICHDuration',tx.enb.PHICHDuration);
        tx.mibbits = mib(tx.mib);
        tx.bch = bch(tx.mibbits, tx.enb.CellRefP);
        tx.pbch = pbch(tx.enb, tx.bch);
        tx.grid(pbchi(tx.enb)) = tx.pbch;
    end
    
    % CFI
    tx.cfibits = cfi(tx.enb.CFI);
    tx.pcfich = pcfich(tx.enb, tx.cfibits);
    tx.grid(pcfichi(tx.enb)) = tx.pcfich;

    if tx_sib1 || tx_sib2
        if tx_sib1
            tx.sib = '684c42821910f66e82119246a80000000000';
            k = mod(floor(tx.enb.NFrame/2), 4);
        else
            tx.sib = '000000fe9f848000185a7a0220040800d009656aab07b7435ce46080';
            k = 0; % TODO: 36.321 5.3.1 last para; safe to hard-code 0 for our scheduler
        end
        rv = mod(ceil(3/2 * k), 4);
        
        % DCI
        tx.dci = configureDCI(tx.enb, length(tx.sib) * 4, rv);
        tx.dcibits = dci(tx.dci);
        tx.rnti = 65535; % SI-RNTI
        tx.dcch = dcch(tx.dcibits, tx.rnti);
        tx.pdcch = pdcch(tx.enb, tx.dcch);
        tx.pdcchi = pdcchi(tx.enb);
        tx.grid(tx.pdcchi(1:36)) = tx.pdcch; % TODO: PDCCH search space placement

        % DL-SCH
        tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
        tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
        tx.dlsch = dlsch(hex2bin(tx.sib), length(tx.pdschi) * 2, tx.pdschConfig.RV); % TODO: modulation; assume QPSK
        tx.pdsch = pdsch(tx.enb, tx.rnti, tx.dlsch);
        tx.grid(tx.pdschi) = tx.pdsch;
    end
    
    % OFDM modulation
    tx.nFFT = 2048; % 1024 for normal 15.36 MHz, 2048 if providing 30.72 MHz for Sora
    tx.waveform = modulate(tx.nFFT, tx.grid);
    
    % Print frame summary.
    msg = sprintf('Tx %04d.%02d ', tx.enb.NFrame, tx.enb.NSubframe);
    if tx_pss; msg = [msg 'PSS ']; end    
    if tx_mib; msg = [msg 'MIB ']; end    
    if tx_sib1; msg = [msg 'SIB1(' num2str(tx.dci.RV) ') ']; end
    if tx_sib2; msg = [msg 'SIB2 ']; end
    fprintf([msg '\n']);
end

% TODO: test receiveFrame against LTE System Toolbox generated frames
% or captured frames; but they must be 50RB, CellRefP=1. I validated
% PSS and SSS reception against LSTgenFrame50RB.mat
function rx = receiveSubframe(tx, ue_state)
    % Cheat: assume no time or frequency offset.
    % Cheat: know bandwidth in advance so we skip 6RB demodulation pass.
    rx.enb.NDLRB = 50;
    
    % OFDM demodulation
    rx.grid = demodulate(rx.enb.NDLRB, tx.nFFT, tx.waveform);
    
    % PSS
    rx.pss = rx.grid(pssSubcarriers(rx.enb.NDLRB), 7);
    rx.Nid2 = decode_pss(rx.pss);
    if isnan(rx.Nid2)
        rx_pss = false;
        rx.enb.NCellID = ue_state.enb.NCellID;
        rx.enb.NSubframe = ue_state.enb.NSubframe + 1; % next PSS will override
    else
        rx_pss = true;
        %assert(tx.Nid2 == rx.Nid2);

        % SSS
        rx.sss = rx.grid(pssSubcarriers(rx.enb.NDLRB), 6);
        [rx.Nid1, rx.enb.NSubframe] = decode_sss(rx.sss, rx.Nid2);
        rx.enb.NCellID = 3 * rx.Nid1 + rx.Nid2;
        %assert(tx.Nid1 == rx.Nid1);
    end
    %assert(tx.enb.NCellID == rx.enb.NCellID);
    %assert(tx.enb.NSubframe == rx.enb.NSubframe);
    
    % MIB
    for cellrefp = [1 2] % don't support 4 port equalization yet
        rx.enb.CellRefP = cellrefp;
        est = estimate(rx.enb, rx.grid);
        rx.pbch = equalize(rx.enb, rx.grid, est, pbchi(rx.enb));
        [rx.mibbits, nmod4, rx.enb.CellRefP] = decode_bch(rx.enb, rx.pbch);
        if ~isnan(rx.mibbits); break; end
    end
    if isnan(rx.mibbits)
        rx_mib = false;
        rx.enb.NFrame = ue_state.enb.NFrame; % next MIB will override
        rx.enb.CellRefP = ue_state.enb.CellRefP;
        rx.enb.Ng = ue_state.enb.Ng;
        rx.enb.PHICHDuration = ue_state.enb.PHICHDuration;
    else
        rx_mib = true;
        %assert(isequal(tx.mibbits, rx.mibbits));
        rx.mib = decode_mib(rx.mibbits);
        rx.enb.Ng = rx.mib.Ng;
        rx.enb.PHICHDuration = rx.mib.PHICHDuration;
        %assert(isequal(tx.mib, rx.mib));
        rx.enb.NFrame = rx.mib.NFrameDiv4 * 4 + nmod4;
    end
    %assert(tx.enb.NFrame == rx.enb.NFrame);

    % Re-estimate if necessary (might manage to decode MIB without diversity equalization)
    if size(est, 3) < rx.enb.CellRefP
        est = estimate(rx.enb, rx.grid);
    end
    
    % CFI
    rx.pcfich = equalize(rx.enb, rx.grid, est, pcfichi(rx.enb));
    plotConstellations(rx.pcfich);
    rx.cfibits = pcfich_decode(rx.enb, rx.pcfich);
    rx.enb.CFI = cfi_decode(rx.cfibits);
    %assert(tx.enb.CFI == rx.enb.CFI);
    
    % DCI
    rx.pdcchi = pdcchi(rx.enb);
    rx.pdcch = equalize(rx.enb, rx.grid, est, rx.pdcchi(1:36)); % TODO: PDCCH search
    plotConstellations(rx.pdcch);
    rx.dcch = pdcch_decode(rx.enb, rx.pdcch);
    [rx.dcibits, rx.rnti] = dcch_decode(rx.dcch);
    if rx.rnti ~= 65535 % SI-RNTI        
        rx_sib1 = false; rx_sib2 = false;
    else
        %assert(isequal(tx.rnti, rx.rnti));
        rx.dci = dci_decode(rx.dcibits);
        %assert(isequal(tx.dci, rx.dci));
    
        % DL-SCH
        rx.pdschConfig = configurePDSCH(rx.enb, rx.dci, rx.rnti);
        rx.pdschi = pdschi(rx.enb, rx.pdschConfig);
        rx.pdsch = equalize(rx.enb, rx.grid, est, rx.pdschi);
        rx.dlsch = pdsch_decode(rx.enb, rx.rnti, rx.pdsch);
        rx.sib = bin2hex(dlsch_decode(rx.dlsch, rx.pdschConfig));
        %assert(isequal(tx.sib, rx.sib));
        disp(rx.sib);
        
        % TODO: ASN.1 decoding
        rx_sib1 = mod(rx.enb.NFrame, 2) == 0 && rx.enb.NSubframe == 5;
        rx_sib2 = mod(rx.enb.NFrame, 8) == 0 && rx.enb.NSubframe == 0;
    end
    
    % Print frame summary.
    msg = sprintf('Rx %04d.%02d ', rx.enb.NFrame, rx.enb.NSubframe);
    if rx_pss; msg = [msg 'PSS ']; end    
    if rx_mib; msg = [msg 'MIB ']; end    
    if rx_sib1; msg = [msg 'SIB1(' num2str(rx.dci.RV) ') ']; end
    if rx_sib2; msg = [msg 'SIB2 ']; end
    fprintf([msg '\n']);
end

function table = tbs_table()
    % TODO: 36.213 Table 7.1.7.2.1-1    
    table = [16 32 56;24 56 88;32 72 144;40 104 176;56 120 208;72 144 224;328 176 256;104 224 328;120 256 392;136 296 456;...
             144 328 504; 176 376 584; 208 440 680; 224 488 744; 256 552 840; 280 600 904; 328 632 968; 336 696 1064;...
             376 776 1160; 408 840 1288; 440 904 1384; 488 1000 1480; 520 1064 1608; 552 1128 1736; 584 1192 1800;...
             616 1256 1864; 712 1480 2216];
end

function dci = configureDCI(enb, trblklen, rv)
    dci.DCIFormat = 'Format1A';
    dci.AllocationType = 0;
    dci.HARQNo = 0;
    dci.NewData = 0;
    dci.RV = rv;
    
    LCRBs = 12; assert(trblklen <= 296); % captured SIB2,3 of 296 bits used 12 RBs
    RBstart = 0; % TODO: scheduling of multiple codewords
    dci.Allocation.RIV = enb.NDLRB * (LCRBs - 1) + RBstart; assert(LCRBs - 1 <= floor(enb.NDLRB/2));

    table = tbs_table();
    [ITBS, NPRB] = find(table(:,2:3) == trblklen, 1, 'first');
    assert(~isempty(ITBS) && ~isempty(NPRB));
    dci.TPCPUCCH = NPRB - 1; % find really returns NPRB - 1
    dci.ModCoding = ITBS - 1; % find really returns ITBS + 1
end

function symbols = pdsch(enb, rnti, bits)
    ci = rnti*2^14 + enb.NSubframe*2^9 + enb.NCellID;
    scrambled = xor(bits, gold(ci, length(bits)));
    symbols = qpsk(scrambled);
end

function bits = pdsch_decode(enb, rnti, symbols)
    scrambled = decode_qpsk(symbols);
    ci = rnti*2^14 + enb.NSubframe*2^9 + enb.NCellID;
    bits = xor(scrambled, gold(ci, length(scrambled)));
end

function e = dlsch(a, E, rv)
    c = [a; crc24a(a)];
    d = turboCode(c);
    e = rateMatchTurbo(d, E, rv);
end

function a = dlsch_decode(e, config)
    A = config.trblklen;
    D = A + 24 + 4; % TB + CRC + trellis termination
    d = derateMatchTurbo(e, D, config.RV);
    softbits = d' * 2 - 1; % TODO: proper soft demodulation
    [~, x, ~, ~] = decodeTurbo(softbits, A + 24, 3, 1);
    a = x(1:A)'; c = x(A+(1:24))';
    assert(isequal(crc24a(a), c));
end

function ek = rateMatchTurbo(dk, E, rv)
    vk0 = interleave(dk(:,1), 'turbo', 0);
    vk1 = interleave(dk(:,2), 'turbo', 0);
    vk2 = interleave(dk(:,3), 'turbo', 1);
    vk12 = [vk1'; vk2'];                % interlace vk1, vk2
    wk = [vk0; vk12(:)];
    C = 32;
    R = length(vk0) / C;
    Ncb = 3 * R * C;                    % TODO: have skipped all the Nsoft stuff
    k0 = R * (2 * ceil(Ncb / (8 * R)) * rv + 2);
    wk = circshift(wk, [-k0, 0]);
    ek = wk(~isnan(wk));
    ek = ek(mod(0:E-1, length(ek)) + 1);
end

function dk = derateMatchTurbo(ek, D, rv)
    % TODO: filler bits in vk2?
    % TODO: puncturing, repetition averaging
    k = (1:D)';
    wk0 = interleave(k, 'turbo', 0);    % get correct padding positions
    wk1 = interleave(k, 'turbo', 0);    % get correct padding positions
    wk2 = interleave(k, 'turbo', 1);    % get correct padding positions
    wk12 = [wk1'; wk2'];                % interlace wk1, wk2
    wk = [wk0; wk12(:)];
    C = 32;
    R = ceil(D / C);
    Ncb = 3 * R * C;                    % TODO: have skipped all the Nsoft stuff
    k0 = R * (2 * ceil(Ncb / (8 * R)) * rv + 2);
    pad = sum(isnan(wk(1:k0)));
    assert(length(ek) >= 3*D);          % assume repetition
    ek = circshift(ek(1:3*D), [k0-pad 0]);  % extract one complete cycle
    wk(~isnan(wk)) = ek;                % replace non-padding with real data
    vk = reshape(wk, [], 3);
    vk0 = vk(:,1); vk12 = vk(:,2:3);
    vk12 = reshape(vk12, 2, [])';
    vk1 = vk12(:,1); vk2 = vk12(:,2);
    dk0 = deinterleave(vk0, 'turbo', 0);
    dk1 = deinterleave(vk1, 'turbo', 0);
    dk2 = deinterleave(vk2, 'turbo', 1);
    dk = [dk0 dk1 dk2];
end

function dk = turboCode(ck)
    K = length(ck);
    table = [3 10;7 12;19 42;7 16;7 18;11 20;5 22;11 24;7 26;41 84;103 90;15 32;9 34;17 108;9 38;21 120;101 84;21 44;57 46;23 48;13 50;27 52;11 36;27 56;85 58;29 60;33 62;15 32;17 198;33 68;103 210;19 36;19 74;37 76;19 78;21 120;21 82;115 84;193 86;21 44;133 90;81 46;45 94;23 48;243 98;151 40;155 102]; % TODO: 36.212 Table 5.1.3-3
    f = table((K - 32) / 8, :);
    dk = zeros(K+4, 3);
    reg1 = zeros(3, 1);
    reg2 = zeros(3, 1);
    for k = 1:K
        cki = ck(mod(f(1) * (k-1) + f(2) * (k-1)^2, K) + 1);
        in1 = mod(ck(k) + reg1(2) + reg1(3), 2);
        in2 = mod(cki + reg2(2) + reg2(3), 2);
        dk(k,1) = ck(k);
        dk(k,2) = mod(in1 + reg1(1) + reg1(3), 2);
        dk(k,3) = mod(in2 + reg2(1) + reg2(3), 2);
        reg1 = [in1; reg1(1:end-1)];
        reg2 = [in2; reg2(1:end-1)];
    end
    % Termination
    xk=[]; xk_=[]; zk=[]; zk_=[];
    for k = 1:3
        in1 = mod(reg1(2) + reg1(3), 2);
        in2 = mod(reg2(2) + reg2(3), 2);
        xk(k)  = in1;
        xk_(k) = in2;
        zk(k)  = mod(reg1(1) + reg1(3), 2);
        zk_(k) = mod(reg2(1) + reg2(3), 2);
        reg1 = [0; reg1(1:end-1)];
        reg2 = [0; reg2(1:end-1)];
    end
    dk(K+1:end,:) = [xk(1) zk(2) xk_(1) zk_(2);...
                     zk(1) xk(3) zk_(1) xk_(3);...
                     xk(2) zk(3) xk_(2) zk_(3)]';
end

function indices = pdschi(enb, pdsch)
    nSC = enb.NDLRB * 12;
    switch enb.CellRefP
        case 1
            sym0 = (mod(enb.NCellID, 6) + 1):6:nSC; % CRS starting index
            sym4 = (mod(enb.NCellID, 3) + 1):6:nSC;
            crsi = bsxfun(@plus, nSC*[4 7 11], [sym4' sym0' sym4']);
        case 2
            crs_k = mod(enb.NCellID, 3) + 1; % CRS starting index
            crsi = bsxfun(@plus, nSC*[4 7 11], (crs_k:3:nSC)');
    end
    centre = enb.NDLRB * 6 - 36 + (1:72);
    switch enb.NSubframe
        case 0; pssi = bsxfun(@plus, nSC*(5:10), centre'); % PSS, SSS, PBCH
        case 5; pssi = bsxfun(@plus, nSC*(5:6),  centre'); % PSS, SSS
        otherwise; pssi = [];
    end
    SCs = bsxfun(@plus, 12*pdsch.PRBSet', (1:12)');
    REs = bsxfun(@plus, nSC*(enb.CFI:13), SCs(:));
    indices = setdiff(REs, crsi);
    indices = setdiff(indices, pssi);
end

function config = configurePDSCH(enb, dci, rnti)
    config.RNTI = rnti;
    len = floor(dci.Allocation.RIV / enb.NDLRB) + 1;
    start = mod(dci.Allocation.RIV, enb.NDLRB);
    config.PRBSet = (start:start+len-1)';
    config.NLayers = enb.CellRefP;
    assert(isequal(dci.DCIFormat, 'Format1A'));
    config.Modulation = {'QPSK'}; assert(rnti == 65535); % SI-RNTI
    config.RV = dci.RV;
    config.TxScheme = 'TxDiversity';
    if enb.CellRefP == 1
        config.TxScheme = 'Port0';
    end
    config.NTurboDecIts = 5; % arbitrary, from LST example code
    NPRB = 2 + mod(dci.TPCPUCCH, 2);
    table = tbs_table();
    config.trblklen = table(dci.ModCoding+1, NPRB); assert(config.trblklen ~= 0);
end

function str = bin2hex(bin)
    dec = [8 4 2 1] * reshape(double(bin), 4, []);
    str = sprintf('%x', dec);
end

function bin = hex2bin(str)
    bin = [];
    for k = 1:length(str)
        bin = [bin; bitget(hex2dec(str(k)), 4:-1:1)'];
    end
end

function symbols = pdcch(enb, bits)
    ci = enb.NSubframe * 2^9 + enb.NCellID;
    scrambled = xor(bits, gold(ci, length(bits)));
    symbols = qpsk(scrambled);
end

function bits = pdcch_decode(enb, symbols)
    scrambled = decode_qpsk(symbols);
    ci = enb.NSubframe * 2^9 + enb.NCellID;
    bits = xor(scrambled, gold(ci, length(scrambled)));
end

function e = dcch(a, rnti)
    mask = bitget(rnti, 16:-1:1)';
    c = [a; xor(crc16(a), mask)];
    d = convCode(c);
    e = rateMatchConv(d, 72); % TODO: configurable PDCCH format
end

function [a, rnti] = dcch_decode(e)
    dciFormat1ALen = 27; crcLen = 16; blockLen = dciFormat1ALen + crcLen;
    d = deinterleaveConvolutionalRateMatching(e', blockLen, 1);%derateMatchConv(e, blockLen);
    c = decodeConvolutionalCircularViterbi(d, 6)';
    a = c(1:dciFormat1ALen);
    rnti = bi2de(xor(crc16(a), c(dciFormat1ALen+1:end)));
end

function bits = dci(d)
    bits = zeros(27, 1);
    bits(1) = bitget(find(strcmp({'Format0' 'Format1A'}, d.DCIFormat))-1, 1);
    bits(2) = bitget(d.AllocationType, 1);
    bits(3:13) = bitget(d.Allocation.RIV, 11:-1:1);
    bits(14:18) = bitget(d.ModCoding, 5:-1:1);
    bits(19:21) = bitget(d.HARQNo, 3:-1:1);
    bits(22) = bitget(d.NewData, 1);
    bits(23:24) = bitget(d.RV, 2:-1:1);
    bits(25:26) = bitget(d.TPCPUCCH, 2:-1:1);
end

function dci = dci_decode(bits)
    formats = {'Format0', 'Format1A'};
    dci.DCIFormat = formats{bits(1)+1};
    dci.AllocationType = bits(2);
    dci.Allocation = struct('RIV', bi2de(bits(3:13)));
    dci.ModCoding = bi2de(bits(14:18));
    dci.HARQNo = bi2de(bits(19:21));
    dci.NewData = bits(22);
    dci.RV = bi2de(bits(23:24));
    dci.TPCPUCCH = bi2de(bits(25:26));
    assert(bits(27) == 0);
end

function pdcchREs = pdcchi(enb)
    assert(enb.NDLRB == 50);
    %assert(enb.NCellID == 11);
    %assert(enb.CFI == 3);
    % TODO: PDCCH indices depend on several other parameters; hardcoding
    % values for Redmond AT&T cell here.
    if enb.NCellID == 11 && enb.CFI == 3
        rb0 = [1 2 4 5 601:604 1201:1204 605:608 1205:1208 7 8 10 11 609:612 1209:1212];
    elseif enb.NCellID == 124 && enb.CFI == 2
        rb0 = [1 3 4 6 601:604 605:608 7 9 10 12 609:612];
    else
        fprintf('Whoa, enb is:\n'); disp(enb); freakout
    end
    indices = bsxfun(@plus, 12*(0:enb.NDLRB-1), rb0');
    indices = setdiff(indices, [pcfichi(enb); phichi(enb)], 'stable');
    pdcchREGs = reshape(indices, 4, [])';
    unshiftedREGs = circshift(pdcchREGs, [enb.NCellID, 0]);
    len = length(pdcchREGs);
    perm = derateMatchConv([1:len nan(1, 2*len)]', len);
    assert(~any(isnan(perm(:,1))));
    uninterleavedREGs = unshiftedREGs(perm(:,1), :);
    pdcchREs = reshape(uninterleavedREGs', [], 1);
end

function indices = phichi(enb)
    assert(isequal(enb.PHICHDuration, 'Normal'))
    nPHICHgroups = ceil(enb.Ng * enb.NDLRB / 8);
    regs = setdiff(0:enb.NDLRB*2-1, pcfich_regs(enb));
    n0 = length(regs);
    offsets = bsxfun(@plus, floor((0:2) .* n0 / 3), (0:nPHICHgroups-1)');
    group_k = regs(mod(enb.NCellID + offsets(:), n0) + 1);
    indices = bsxfun(@plus, 6*group_k, (0:5)');
    crs_k = mod(enb.NCellID, 3); % CRS starting index
    indices = indices(mod(indices, 3) ~= crs_k) + 1;
    % TODO: reshape into nPHICHgroups columns, ready for indexing
end

function regs = pcfich_regs(enb)
    k = mod(enb.NCellID, 2 * enb.NDLRB);
    regs = k + floor((0:3) * enb.NDLRB / 2);
end

function indices = pcfichi(enb)
    nSC = enb.NDLRB * 12;
    regs = pcfich_regs(enb);
    indices = bsxfun(@plus, 6*regs, (0:5)');
    crs_k = mod(enb.NCellID, 3); % CRS starting index
    indices = indices(mod(indices, 3) ~= crs_k);
    indices = mod(indices, nSC) + 1;
end

function bits = cfi(c)
    bits = ones(32, 1);
    bits(c:3:end) = 0;
end

function c = cfi_decode(bits)
    rr = xcorr(bits, cfi(1));
    [~, c] = max(rr(32:end));
end

function symbols = pcfich(enb, bits)
    ci = (enb.NSubframe+1)*(2*enb.NCellID+1)*2^9 + enb.NCellID;
    scrambled = xor(bits, gold(ci, 32));
    symbols = qpsk(scrambled);
end

function bits = pcfich_decode(enb, symbols)
    scrambled = decode_qpsk(symbols);
    ci = (enb.NSubframe+1)*(2*enb.NCellID+1)*2^9 + enb.NCellID;
    bits = xor(scrambled, gold(ci, 32));
end

function bits = mib(m)
    bandwidths = [6 15 25 50 75 100];
    durations = {'Normal' 'Extended'};
    ngs = [1/6 1/2 1 2];
    bits = zeros(24, 1);
    bits(1:3) = bitget(find(bandwidths == m.NDLRB)-1, 3:-1:1);
    bits(4) = bitget(find(strcmp(durations, m.PHICHDuration))-1, 1);
    bits(5:6) = bitget(find(ngs == m.Ng)-1, 2:-1:1);
    bits(7:14) = bitget(m.NFrameDiv4, 8:-1:1);
end

function d = bi2de(b)
    d = 2.^(length(b)-1:-1:0) * b;
end

function mib = decode_mib(mibbits)
    bandwidths = [6 15 25 50 75 100];
    durations = {'Normal' 'Extended'};
    ngs = [1/6 1/2 1 2];
    mib.NDLRB = bandwidths(bi2de(mibbits(1:3)) + 1);
    mib.PHICHDuration = durations{mibbits(4) + 1};
    mib.Ng = ngs(bi2de(mibbits(5:6)) + 1);
    mib.NFrameDiv4 = bi2de(mibbits(7:14));
end

function symbols = pbch(enb, bits)
    nmod4 = mod(enb.NFrame, 4);
    g = gold(enb.NCellID, 1920);
    scrambled = xor(g(nmod4*480+(1:480)), bits);
    symbols = qpsk(scrambled);
end

function e = bch(a, cellrefp)
    mask = ones(16, 1) * (cellrefp - 1); % TODO: cellrefp = 4
    c = [a; xor(crc16(a), mask)];
    d = convCode(c);
    e = rateMatchConv(d, 480);
end

function [mibbits, nmod4, cellrefp] = decode_bch(enb, symbols)
    mibbits = NaN; cellrefp = NaN;
    scrambled = decode_qpsk(symbols);
    g = gold(enb.NCellID, 1920);
    for nmod4 = 0:3
        e = xor(g(nmod4*480+(1:480)), scrambled);
        d = derateMatchConv(e, 40);
        c = decodeConvolutionalCircularViterbi(d', 6)';
        err = xor(crc16(c(1:24)), c(25:end));
        if err == zeros(16, 1)
            cellrefp = 1; break;
        elseif err == ones(16, 1)
            cellrefp = 2; break;
        elseif err == mod(0:15, 2)'
            cellrefp = 4; break;
        end
    end
    if ~isnan(cellrefp)
        mibbits = c(1:24);
    end
end

function dk = convCode(ck)
    K = length(ck);
    dk = zeros(K, 3);
    reg = ck(end:-1:end-5);
    for k = 1:K
        dk(k,1) = mod(ck(k) + sum(reg([2 3 5 6])), 2);
        dk(k,2) = mod(ck(k) + sum(reg([1 2 3 6])), 2);
        dk(k,3) = mod(ck(k) + sum(reg([1 2 4 6])), 2);
        reg = [ck(k); reg(1:end-1)];
    end
end

function r = crc16(a)
    L = 16; g = [5 12];
    r = crc(L, g, a);
end

function r = crc24a(a)
    L = 24; g = [1 3 4 5 6 7 10 11 14 17 18 23];
    r = crc(L, g, a);
end

function r = crc(L, g, a)
    r = zeros(L, 1);
    for i = 1:length(a)
        inv = xor(a(i), r(L));
        r(g) = xor(inv, r(g));
        r = [inv; r(1:L-1)];
    end
    r = flipud(r);
end

function out = interleave(in, mode, delta)
    P = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, ...
         0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30];
    if isequal(mode, 'turbo')
        P = P([17:32 1:16]);
    end
    D = length(in);
    C = 32;
    R = ceil(D / C);
    Kpi = C * R;
    k = 0:Kpi-1;
    pik = mod(P(floor(k/R)+1) + C*mod(k, R) + delta, Kpi);
    y = [nan(Kpi - D, 1); in];
    out = y(pik+1);
end

function out = deinterleave(in, mode, delta)
    D = length(in);
    assert(mod(D, 32) == 0);            % in should be padded
    k = 1:D;
    pik = interleave(k, mode, delta);
    [~, inv] = sort(pik);
    out = in(inv);
    out(isnan(out)) = [];               % remove padding
end

function ek = rateMatchConv(dk, E)
    vk0 = interleave(dk(:,1), 'conv', 0);
    vk1 = interleave(dk(:,2), 'conv', 0);
    vk2 = interleave(dk(:,3), 'conv', 0);
    wk = [vk0; vk1; vk2];
    ek = wk(~isnan(wk));
    ek = ek(mod(0:E-1, length(ek)) + 1);
end

function dk = derateMatchConv(ek, D)
    % TODO: puncturing, repetition averaging
    assert(length(ek) >= 3*D);          % assume repetition
    ek = ek(1:3*D);                     % extract one complete cycle
    k = (1:D)';
    wk = interleave(k, 'conv', 0);      % get correct padding positions
    wk = [wk; wk; wk];
    wk(~isnan(wk)) = ek;                % replace non-padding with real data
    vk = reshape(wk, [], 3);
    dk0 = deinterleave(vk(:,1), 'conv', 0);
    dk1 = deinterleave(vk(:,2), 'conv', 0);
    dk2 = deinterleave(vk(:,3), 'conv', 0);
    dk = [dk0 dk1 dk2];
end

function symbols = qpsk(bits)
    b = 1 - 2 * reshape(bits, 2, [])';
    symbols = b * [1; 1j] / sqrt(2);
end

function bits = decode_qpsk(symbols)
    % TODO: soft bits
    bits = reshape([real(symbols) imag(symbols)]', [], 1) < 0;
end

function indices = pbchi(enb)
    crs_k = mod(enb.NCellID, 3); % CRS starting index
    sym23 = enb.NDLRB * 6 - 36 + (0:71);
    sym01 = sym23(mod(sym23, 3) ~= crs_k);
    nSC = enb.NDLRB * 12;
    indices = [sym01+7*nSC sym01+8*nSC sym23+9*nSC sym23+10*nSC]' + 1;
end

function c = gold(cinit, Mpn)
    Nc=1600; len=Nc+Mpn;
    x2=zeros(len,1); x2(1:31)=bitget(cinit,1:31);
    for n=1:len-31; x2(n+31)=mod(x2(n+3)+x2(n+2)+x2(n+1)+x2(n),2); end
    x1=zeros(len,1); x1(1)=1;
    for n=1:len-31; x1(n+31)=mod(x1(n+3)+x1(n),2); end
    c=mod(x1(Nc+1:end)+x2(Nc+1:end),2);
end

function r = rss(c, m)
    r = (1-2*c(2*m+1)) + 1j*(1-2*c(2*m+2));
    r = r / sqrt(2);
end

function crs = cellRS(cellID, subframe, ndlrb, cellrefp)
    NDLRB_MAX = 110;
    assert(ismember(cellrefp, [1 2]));
    % Assume only 1 RX antenna.
    crs = zeros(ndlrb * 12, 14, cellrefp); % 12 subcarriers, 14 symbols per RB
    vshift = mod(cellID, 6);
    for port = 0:cellrefp-1
        for slot = [0 1]
            ns = subframe * 2 + slot;
            for l = [0 4]
                v = 3 * xor(port, l);
                ci = 2^10*(7*(ns+1)+l+1)*(2*cellID+1)+2*cellID+1; % normal CP
                c = gold(ci, 4*NDLRB_MAX-1);
                for m = 0:(2*ndlrb-1)
                    m_ = m + NDLRB_MAX - ndlrb;
                    r = rss(c, m_);
                    k = 6 * m + mod(v + vshift, 6);
                    assert(crs(k+1, 7*slot+l+1, port+1) == 0); % DEBUG
                    crs(k+1, 7*slot+l+1, port+1) = r;
                end
            end
        end
    end
end

function est = estimate(enb, grid)
global PLOT_CHANNEL;

% Cell reference signal interpolation per port (TX antenna).
    % TODO: account for DC, virtual pilots, average several subframes
    dims = size(grid);
    nSC = dims(1); nSym = dims(2);
    est = zeros(nSC, nSym, enb.CellRefP);
    crs = cellRS(enb.NCellID, enb.NSubframe, enb.NDLRB, enb.CellRefP);
    for port = 1:enb.CellRefP
        crsp = crs(:, :, port);
        crsip = find(crsp);
        pEst = zeros(dims); % Really so we can do nice plots
        pEst(crsip) = grid(crsip) ./ crsp(crsip);
        %subplot(2,1,1), surf(abs(pEst)); subplot(2,1,2), surf(angle(pEst));
        [sc, sym] = ind2sub(dims, crsip);
        F = scatteredInterpolant(sym, sc, pEst(crsip));
        [scq, symq] = meshgrid(1:nSym, 1:nSC);
        est(:, :, port) = F(scq, symq); % Really only need to evaluate F at desired positions
    end
    
    if PLOT_CHANNEL
      figure(3);
      hold on
      for l = 1:size(est,2)
        plot(20*log10(abs(est(:,l))));
      end
    end
end

function eq = equalize(enb, grid, est, indices)
    % Equalization and QPSK demodulation.
    if (enb.CellRefP == 1)
        eq = grid(indices) ./ est(indices);
    else
        % http://en.wikipedia.org/wiki/Space-time_block_coding_based_transmit_diversity
        % ...but with a unique channel estimate for every subcarrier.
        x1 = grid(indices(1:2:end)); x2 = grid(indices(2:2:end));
        e1 = est(:, :, 1); e2 = est(:, :, 2); % per-port channel estimates
        h1_ = conj(e1(indices)); h2 = e2(indices);
        s1 = h1_(1:2:end) .* x1 + h2(2:2:end) .* conj(x2);
        s2 = -h2(1:2:end) .* conj(x1) + h1_(2:2:end) .* x2;
        eq = reshape([s1.'; s2.'], [], 1); % interlace s1, s2
    end
end

function [s, c0, c1, z1] = sss_sequences(Nid2)
    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+2)+x(k),2); end
    s_=1-2*x;
    s=@(m) circshift(s_,[0,-m]);

    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+3)+x(k),2); end
    c_=1-2*x;
    c0=circshift(c_,[0,-Nid2]);
    c1=circshift(c0,[0,-3]);

    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+4)+x(k+2)+x(k+1)+x(k),2); end
    z_=1-2*x;
    z1=@(m) circshift(z_,[0,-mod(m,8)]);
end

function d = sss(Nid1, Nid2, subframe)
    [s, c0, c1, z1] = sss_sequences(Nid2);
    q_=floor(Nid1/30);
    q=floor((Nid1+q_*(q_+1)/2)/30);
    m_=Nid1+q*(q+1)/2;
    m0=mod(m_,31);
    m1=mod(m0+floor(m_/31)+1,31);
    
    if subframe == 0
        ma = m0; mb = m1;
    else
        ma = m1; mb = m0;
    end
    d = [s(ma).*c0 s(mb).*c1.*z1(ma)];
    d = reshape(d, [], 2)';
    d = d(:);
end

function [Nid1, subframe] = decode_sss(d, Nid2)
    [s, c0, c1, z1] = sss_sequences(Nid2);
    sref=[s(0) s(0)];
    
    % Find first m candidate.
    aS2n = d(1:2:end) ./ c0';
    rr = xcorr(aS2n, sref);
    [~, ma] = max(rr(32:62));
    ma = 31 - ma;

    % Find second m candidate.
    aS2np1 = d(2:2:end) ./ (c1 .* z1(ma))';
    rr = xcorr(aS2np1, sref);
    [~, mb] = max(rr(32:62));
    mb = 31 - mb;
    
    % Use m0 < m1 to figure out subframe index.
    if ma < mb
        subframe = 0;
        m0 = ma;
        m1 = mb;
    else
        subframe = 5;
        m0 = mb;
        m1 = ma;
    end
    
    resets = [0 cumsum(30:-1:25)];
    Nid1 = resets(m1 - m0) + m0;
end

function indices = pssSubcarriers(NDLRB)
    indices = NDLRB * 6 - 31 + (1:62);
end

function du = pss(Nid2)
    map = [25 29 34]; u = map(Nid2 + 1);
    du = exp(-1j * pi * u * [(0:30).*(1:31) (32:62).*(33:63)]' / 63);
end

function Nid2 = decode_pss(du)
    rr = colfun(@(Nid2) xcorr(du, pss(Nid2)), 0:2);
    [~, k] = max(max(abs(rr)));
    Nid2 = k - 1;
    mag = sort(abs(rr(:,k)));
    ratio = mag(end)/mag(end-1);
    if isnan(ratio) || ratio < 3 % TODO: correct threshold?
        Nid2 = NaN; % signal no PSS received
    end
end

function out = colfun(fn, m)
    out = [];
    for col = 1:size(m, 2)
        out(:, col) = fn(m(:, col));
    end
end

function waveform = modulate(nFFT, grid)
    fs = nFFT / 256 * 3.84e6;
    cpLength = [160, 144, 144, 144, 144, 144, 144] * nFFT / 2048;
    [nSC, nSymbols] = size(grid);
    waveform = [];
    for n = 1:nSymbols
        padded = zeros(nFFT, 1);
        padded([end-nSC/2+1:end, 2:nSC/2+1]) = grid(:, n);
        time = ifft(padded);
        cp = time(end + 1 - cpLength(mod(n - 1, 7) + 1) : end);
        waveform = [waveform; cp; time];
    end
    assert(length(waveform) == fs * 1e-3 * nSymbols / 14);
end

function grid = demodulate(NDLRB, nFFT, waveform)
global PLOT_CONST;
    fs = nFFT / 256 * 3.84e6;
    cpLength = [160, 144, 144, 144, 144, 144, 144] * nFFT / 2048;
    nSymbols = floor(length(waveform) / fs / 1e-3 * 14);
    nSC = NDLRB * 12;
    grid = zeros(nSC, nSymbols);
    start = 0;
    for n=0:nSymbols-1
        cp = cpLength(mod(n, 7)+1);
        cpOffset = fix(cp/2);           % skip into middle of CP
        phaseCorrection = exp(-1i*2*pi*(cp-cpOffset)/nFFT*(0:nFFT-1))';
        f = fft(waveform(start+cpOffset+(1:nFFT))) .* phaseCorrection;
        grid(:,n+1) = f([end-nSC/2+1:end, 2:nSC/2+1]);
        start = start + cp + nFFT;      % skip to end of symbol
    end
end