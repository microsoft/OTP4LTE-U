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

% Transmit
drs_indices = pucch1i(ue, chs, true);
drs = [pucch1(2*ue.NSubframe, ue, chs, [], true); pucch1(2*ue.NSubframe+1, ue, chs, [], true)];

% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
drs_indices(36+(1:36)) = drs_indices(36+(1:36))-1;
pilots = data(drs_indices);

% Remove skew from pilots for easier debugging
% But not needed for processing data bits
% $$$ skew = angle(pilots(1:12) ./ drs(1:12));
% $$$ a = polyfit((1:12)', skew, 1);
% $$$ for j=1:6
% $$$   pilots((j-1)*12+(1:12)) = pilots((j-1)*12+(1:12)) .* exp(-i*a(1)*(1:12)');
% $$$ end
% $$$ pilots(1:36) = pilots(1:36) * exp(i*(angle(drs(13))-angle(pilots(13))));
% $$$ pilots(36+(1:36)) = pilots(36+(1:36)) * exp(i*(angle(drs(11+36))-angle(pilots(11+36))));
% $$$ figure(1); 
% $$$ clf(1);
% $$$ subplot(1,3,1); 
% $$$ plot((angle(pilots)));
% $$$ ylim([-pi pi]);
% $$$ title('RX pilots');
% $$$ subplot(1,3,2);
% $$$ plot((angle(drs)));
% $$$ ylim([-pi pi]);
% $$$ title('Expected pilots');
% $$$ subplot(1,3,3);
% $$$ plot(angle(pilots ./ drs));
% $$$ ylim([-pi pi]);
% $$$ title('Corrected pilots');


indices = pucch1i(ue, chs, false);
% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
indices(48+(1:48)) = indices(48+(1:48))-1;

data_eq = zeros(size(data));
for j=1:4
  data_eq(indices((j-1)*12+(1:12))) = data(indices((j-1)*12+(1:12))) .* drs(1:12) ./ pilots(1:12);
  data_eq(indices(48+(j-1)*12+(1:12))) = data(indices(48+(j-1)*12+(1:12))) .* drs(36+(1:12)) ./ pilots(36+(1:12));
end

symbols1a0_0 = [pucch1(2*ue.NSubframe, ue, chs, 0, false); pucch1(2*ue.NSubframe+1, ue, chs, 0, false)];
symbols1a1_0 = [pucch1(2*ue.NSubframe, ue, chs, 1, false); pucch1(2*ue.NSubframe+1, ue, chs, 1, false)];

%transpose(data_eq(indices(1:96)))
%transpose(symbols1a1_0(1:96))

corr = data_eq(indices(1:96))' * symbols1a1_0(1:96);




% $$$ figure; 
% $$$ X = 1:191;
% $$$ plot(X, real(xcorr(data_eq(indices(1:96)), symbols1a0_0(1:96))), 'k');
% $$$ hold on;
% $$$ plot(X, real(xcorr(data_eq(indices(1:96)), symbols1a1_0(1:96))), 'b');
% $$$ plot(X, real(xcorr(data_eq(indices(1:96)), symbols4a1_0(1:96))), 'r');
% $$$ 
% $$$ 
% $$$ if license('test', 'lte_toolbox')
% $$$     warning('off','lte:defaultValue')
% $$$     drs_indices = ltePUCCH1DRSIndices(ue, chs);
% $$$     drs = ltePUCCH1DRS(ue, chs, []);
% $$$     pilots = data(drs_indices);
% $$$     figure(2); 
% $$$     clf(2);
% $$$     subplot(1,3,1); 
% $$$     plot(angle(pilots));
% $$$     title('RX pilots');
% $$$     subplot(1,3,2);
% $$$     plot(angle(drs));
% $$$     title('Toolbox: expected pilots');
% $$$     subplot(1,3,3);
% $$$     plot(angle(pilots ./ drs));
% $$$     title('Corrected pilots');
% $$$ end
% $$$ 
% $$$ aaaaa
% $$$ 
% $$$ %function pucch
% $$$     warning('off', 'lte:defaultValue');
% $$$     ue.NULRB = 50;
% $$$     ue.NSubframe = 8;
% $$$     ue.NCellID = 11;
% $$$     chs.ResourceIdx = 0; % sr-PUCCH-ResourceIndex (RRC_CS) for SR, f(n1Pucch-AN in SIB2, DCI) for ACK/NACK
% $$$     chs.DeltaShift = 2; % deltaPUCCH-Shift in SIB2
% $$$     chs.ResourceSize = 0; % nRB-CQI in SIB2
% $$$     
% $$$     % Transmit
% $$$     indices = pucch1i(ue, chs, false);
% $$$     drs_indices = pucch1i(ue, chs, true);
% $$$     drs = pucch1(ue, chs, [], true);
% $$$     symbols1_0 = pucch1(ue, chs, [], false);
% $$$     symbols1a0_0 = pucch1(ue, chs, 0, false);
% $$$     symbols1a1_0 = pucch1(ue, chs, 1, false);
% $$$     symbols1b00_0 = pucch1(ue, chs, [0 0], false);
% $$$     symbols1b01_0 = pucch1(ue, chs, [0 1], false);
% $$$     symbols1b10_0 = pucch1(ue, chs, [1 0], false);
% $$$     symbols1b11_0 = pucch1(ue, chs, [1 1], false);
% $$$     
% $$$     chs.ResourceIdx = 4;
% $$$     symbols1_4 = pucch1(ue, chs, [], false);
% $$$     symbols1a0_4 = pucch1(ue, chs, 0, false);
% $$$     symbols1a1_4 = pucch1(ue, chs, 1, false);
% $$$     
% $$$     % Receive
% $$$     % A full receiver might check for all resource indices, formats and
% $$$     % ACK/NACK values. We are only interested in SISO and one UE, therefore:
% $$$     % - SR resource index with ack = 0
% $$$     % - SR resource index with ack = 1
% $$$     % - HARQ resource index with ack = 0
% $$$     % - HARQ resource index with ack = 1
% $$$     % Suggestion for decoding:
% $$$     % - estimate channel using pilots, equalize PUCCH symbols
% $$$     % - correlate received symbols against generated symbols1_0
% $$$     % - if you get a peak (threshold say 48)
% $$$     %   - there was an SR
% $$$     %   - look at sign of symbols ./ symbols1_0
% $$$     %   - +ve => NACK, -ve => ACK
% $$$     % - correlate received symbols against generated symbols1_4
% $$$     % - if you get a peak (threshold say 48)
% $$$     %   - look at sign of symbols ./ symbols1_4
% $$$     %   - +ve => NACK, -ve => ACK
% $$$     plot(abs(xcorr(symbols1_4, symbols1_0))); hold on
% $$$     plot(abs(xcorr(symbols1_0))); hold off
% $$$     
% $$$     if license('test', 'lte_toolbox')
% $$$         chs.ResourceIdx = 0;
% $$$         assert(isequal(indices, ltePUCCH1Indices(ue, chs)));
% $$$         assert(max(abs(drs - ltePUCCH1DRS(ue, chs))) < 1e-15);
% $$$         assert(isequal(drs_indices, ltePUCCH1DRSIndices(ue, chs)));
% $$$         assert(max(abs(symbols1_0 - ltePUCCH1(ue, chs, []))) < 1e-15);
% $$$         assert(max(abs(symbols1a0_0 - ltePUCCH1(ue, chs, 0))) < 1e-15);
% $$$         assert(max(abs(symbols1a1_0 - ltePUCCH1(ue, chs, 1))) < 1e-15);
% $$$         assert(max(abs(symbols1b00_0 - ltePUCCH1(ue, chs, [0 0]))) < 1e-15);
% $$$         assert(max(abs(symbols1b01_0 - ltePUCCH1(ue, chs, [0 1]))) < 1e-15);
% $$$         assert(max(abs(symbols1b10_0 - ltePUCCH1(ue, chs, [1 0]))) < 1e-15);
% $$$         assert(max(abs(symbols1b11_0 - ltePUCCH1(ue, chs, [1 1]))) < 1e-15);
% $$$         chs.ResourceIdx = 4;
% $$$         assert(max(abs(symbols1_4 - ltePUCCH1(ue, chs, []))) < 1e-15);
% $$$         assert(max(abs(symbols1a0_4 - ltePUCCH1(ue, chs, 0))) < 1e-15);
% $$$         assert(max(abs(symbols1a1_4 - ltePUCCH1(ue, chs, 1))) < 1e-15);
% $$$     end
% $$$     %end
% $$$ 
% $$$ 
