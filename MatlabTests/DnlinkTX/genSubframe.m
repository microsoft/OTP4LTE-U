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

function tx = genSubframe(frame, subframe, tx, includeMIB, includeSIB, varargin)
    includeRAR = 0;
    includeDCI0 = 0;
    includeDATA = 0;
    if length(varargin) > 0
        includeRAR = varargin{1};
        fprintf('With includeRAR=%d\n', includeRAR);
    end
    if length(varargin) > 1
        includeDCI0 = varargin{2};
        fprintf('With includeDCI0=%d\n', includeDCI0);
    end
    if length(varargin) > 2
        includeDATA = varargin{3};
        fprintf('With includeDATA=%d\n', includeDATA);
    end

    % RB schedule
    lenRB = 12;
    startRB = 0;
    msgLen = 65;
    if length(varargin) > 3
        lenRB = varargin{4};
        fprintf('With lenRB=%d\n', lenRB);
    end
    if length(varargin) > 4
        startRB = varargin{5};
        fprintf('With startRB=%d\n', startRB);
    end
    if length(varargin) > 5
        msgLen = varargin{6};
        fprintf('With msgLen=%d\n', msgLen);
    end
    
    
    %tx.enb = struct('CellRefP',1,'NDLRB',50,'NCellID',11,'NFrame',frame,...
    %    'NSubframe',subframe,'Ng',1,'PHICHDuration','Normal','CFI',3);
      
    tx.Nid1 = floor(tx.enb.NCellID / 3); tx.Nid2 = mod(tx.enb.NCellID, 3);

    % Scheduling. SIB1 fixed in spec, we set SIB2 period in SIB1
    tx_pss = mod(tx.enb.NSubframe, 5) == 0;
    tx_mib = tx.enb.NSubframe == 0;
    tx_sib1 = mod(tx.enb.NFrame, 2) == 0 && tx.enb.NSubframe == 5;
    tx_sib2 = mod(tx.enb.NFrame, 8) == 0 && tx.enb.NSubframe == 0;
    tx_rar = tx.enb.NSubframe == 6; 
    % tx_dci0 = tx.enb.NSubframe ~= 7; 
    tx_dci0 = tx.enb.NSubframe == 0; 
    tx_data = tx.enb.NSubframe == 4;

    
    
    if (includeDCI0 > 1)
      tx_dci0_ndi_val = mod(floor(((tx.enb.NFrame * 10) + tx.enb.NSubframe) / 8), 2);
      %fprintf('DCI0: %d %d %d\n', frame, subframe, tx_dci0_ndi_val);
    else 
      tx_dci0_ndi_val = 0;
    end

    
    % CRS
    tx.grid = cellRS(tx.enb.NCellID, tx.enb.NSubframe, tx.enb.NDLRB, tx.enb.CellRefP);

    if tx_pss
        % SSS
        tx.grid(pssSubcarriers(tx.enb.NDLRB), 6) = sss(tx.Nid1, tx.Nid2, tx.enb.NSubframe);

        % PSS
        tx.grid(pssSubcarriers(tx.enb.NDLRB), 7) = pss(tx.Nid2);
    end
    
    if includeMIB && tx_mib
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
    
    % PHICH - always ACK an allocation corresponding to PRBstart=1, nDMRS=0...
    tx.phich = phich(tx.enb, 1, 0, 1); % ... which is PHICH group 1, seq 0, ACK see 36.213 9.1.2
    tx.grid(phichi(tx.enb)) = tx.phich;

    
    
    if includeDCI0 && tx_dci0
      % DCI0 for uplink (currently hardcoded)
      if tx_dci0_ndi_val == 0
        tx.dcibits = [0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]';
      else
        tx.dcibits = [0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0]';
      end
      tx.dcch = dcch(tx.dcibits, 64);
      % DCI0 uses CCE0 now
      %tx.pdcch = pdcch(tx.enb, tx.dcch, 4);
      tx.pdcch = pdcch(tx.enb, tx.dcch, 0);
      tx.pdcchi = pdcchi(tx.enb);
      % DCI0 uses CCE0 now
      % tx.grid(tx.pdcchi(144+(1:36))) = tx.pdcch; % TODO: PDCCH search space placement
      tx.grid(tx.pdcchi(1:36)) = tx.pdcch; % TODO: PDCCH search space placement
    end

    
    
    if includeSIB && (tx_sib1 || tx_sib2)
        if tx_sib1
            tx.sib = '684c42821910f66e82119246a80000000000';
            k = mod(floor(tx.enb.NFrame/2), 4);
        else
          % with group hopping
            tx.sib = '000000fe9f848000185a7a0220040800d009656aab07b7435ce46080';
            % without group hopping
            tx.sib = '000000fe9f848000185a7a0200040800c2cad5560f6e86b9c8c10000'; 
            k = 0; % TODO: 36.321 5.3.1 last para; safe to hard-code 0 for our scheduler
        end
        rv = mod(ceil(3/2 * k), 4);
        tx.rnti = 65535; % SI-RNTI

        % DCI
        tx.dci = configureDCI1A(tx.enb, lenRB, startRB, length(tx.sib) * 4, rv, tx.rnti);
        tx.dcibits = dci(tx.dci);
        tx.dcch = dcch(tx.dcibits, tx.rnti);
        % SIB/PDSCH uses CCE4 now
        % tx.pdcch = pdcch(tx.enb, tx.dcch, 0);
        tx.pdcch = pdcch(tx.enb, tx.dcch, 4);
        tx.pdcchi = pdcchi(tx.enb);

        %transpose(tx.pdcch(1:36))
        %transpose(tx.pdcchi(1:36))

        % SIB/PDSCH uses CCE4 now
        % tx.grid(tx.pdcchi(1:36)) = tx.pdcch; % TODO: PDCCH search space placement
        tx.grid(tx.pdcchi(144+(1:36))) = tx.pdcch; % TODO: PDCCH search space placement

        % DL-SCH
        tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
        tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
        tx.dlsch = dlsch(hex2bin(tx.sib), length(tx.pdschi) * 2, tx.pdschConfig.RV); % TODO: modulation; assume QPSK
        tx.pdsch = pdsch(tx.enb, tx.rnti, tx.dlsch);
        tx.grid(tx.pdschi) = tx.pdsch;
    end
    
    if includeRAR && tx_rar
        % RB assignment; 351 = 8 PRBs starting at 1; 208 bits for our MCS
        %         E,T,      RAPID,R,       Timing Advance,H,      RB assignment,    MCS,  TPC,D,C,Temp C-RNTI
        tx.rar = [0,1,0 0 0 0 0 0,0,0 0 0 0 0 0 0 0 0 0 0,0,0 1 0 1 0 1 1 1 1 1,0 0 0 0,0 0 0,0,0,0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0]';
        rv = 0;
        tx.rnti = 2; % RA-RNTI for PRACH in subframe 1
        
        % DCI
        tx.dci = configureDCI1A(tx.enb, lenRB, startRB, length(tx.rar), rv, tx.rnti);
        tx.dcibits = dci(tx.dci);
        tx.dcch = dcch(tx.dcibits, tx.rnti);
        % SIB/PDSCH uses CCE4 now
        % tx.pdcch = pdcch(tx.enb, tx.dcch, 0);
        tx.pdcch = pdcch(tx.enb, tx.dcch, 4);
        tx.pdcchi = pdcchi(tx.enb);
        
        % SIB/PDSCH uses CCE4 now
        % tx.grid(tx.pdcchi(1:36)) = tx.pdcch; % TODO: PDCCH search space placement
        tx.grid(tx.pdcchi(144+(1:36))) = tx.pdcch; % TODO: PDCCH search space placement

        % DL-SCH
        tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
        tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
        tx.dlsch = dlsch(tx.rar, length(tx.pdschi) * 2, tx.pdschConfig.RV); % TODO: modulation; assume QPSK
        tx.pdsch = pdsch(tx.enb, tx.rnti, tx.dlsch);
        tx.grid(tx.pdschi) = tx.pdsch;
    end

    
    if includeDATA && tx_data
        tx.sib = repmat('00', 1, msgLen);
        rv = 0;
        tx.rnti = 64; % SI-RNTI

        % DCI
        tx.dci = configureDCI1A(tx.enb, lenRB, startRB, length(tx.sib) * 4, rv, tx.rnti);
        % Toggle NewData appropriately. Simple way - one packet per frame, so calculate from frame no
        tx.dci.NewData = mod(tx.enb.NFrame, 2);
        
        tx.dcibits = dci(tx.dci);
        tx.dcch = dcch(tx.dcibits, tx.rnti);
        % SIB/PDSCH uses CCE4 now
        % tx.pdcch = pdcch(tx.enb, tx.dcch, 0);
        tx.pdcch = pdcch(tx.enb, tx.dcch, 4);
        tx.pdcchi = pdcchi(tx.enb);

        %transpose(tx.pdcch(1:36))
        %transpose(tx.pdcchi(1:36))

        % SIB/PDSCH uses CCE4 now
        % tx.grid(tx.pdcchi(1:36)) = tx.pdcch; % TODO: PDCCH search space placement
        tx.grid(tx.pdcchi(144+(1:36))) = tx.pdcch; % TODO: PDCCH search space placement

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
