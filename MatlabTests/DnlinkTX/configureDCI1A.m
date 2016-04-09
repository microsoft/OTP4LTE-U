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

function dci = configureDCI1A(enb, lenRB, startRB, trblklen, rv, rnti)
  dci.DCIFormat = 'Format1A';
  dci.AllocationType = 0;
  dci.HARQNo = 0;
  dci.NewData = 0;
  dci.RV = rv;
    
  %lenRB = 12; % captured SIB2,3 of 296 bits used 12 RBs
  %startRB = 0; % TODO: scheduling of multiple codewords
  dci.Allocation.RIV = enb.NDLRB * (lenRB - 1) + startRB; assert(lenRB - 1 <= floor(enb.NDLRB/2));

  if rnti <= 10 || rnti >= 65534 % RA/P/SI-RNTI
      % NB: in this special case TPCPUCCH is not power control
      table = tbs_table();
      [ITBS, NPRB] = find(table(:,2:3) == trblklen, 1, 'first');
      assert(~isempty(ITBS) && ~isempty(NPRB));
      dci.TPCPUCCH = NPRB - 1; % find really returns NPRB + 1
      dci.ModCoding = ITBS - 1; % find really returns ITBS + 1
  else
      % Power control
      dci.TPCPUCCH = 1; % 0 dB

      % Get MCS
      % TODO: Assumes QPSK for now
      modulation = 2;
      table = tbs_table();
      ITBS = find(table(:,lenRB) == trblklen, 1, 'first');
      assert(~isempty(ITBS));
      dci.ModCoding = mcs_table(modulation, ITBS-1);
      % fprintf('%d %d %d\n', trblklen, ITBS, dci.ModCoding);
      assert(dci.ModCoding >= 0);
      
% $$$       if trblklen == 328 
% $$$         dci.ModCoding = 0; % 328 bits for 12 RBs TODO: full tbs_table
% $$$       elseif trblklen == 1224
% $$$         dci.ModCoding = 6; 
% $$$       elseif trblklen == 1864 
% $$$         dci.ModCoding = 9; 
% $$$       else
% $$$         aaa
% $$$       end
  end
end
