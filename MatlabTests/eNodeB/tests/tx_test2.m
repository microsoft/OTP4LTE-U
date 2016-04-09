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

frame = 0;

O = [];

for i = 1:2000
  for subframe = 0:9
    tx_sib1 = mod(frame, 2) == 0 && subframe == 5;
    tx_sib2 = mod(frame, 8) == 0 && subframe == 0;
    tx_rar = subframe == 6; 
    tx_dci0 = subframe == 0; 
    tx_data = (frame < 8) && (subframe == 4);
    
    if (tx_sib1)
      O = [O, [frame, subframe, 0, 0]];
    end
  
    if (tx_sib2)
      O = [O, [frame, subframe, 0, 1]];
    end

    if (tx_rar)
      O = [O, [frame, subframe, 0, 2]];
    end
    
    if (tx_dci0)
      O = [O, [frame, subframe, 5, 5]];
    end
    
    if (tx_data)
      O = [O, [frame, subframe, 0, 3]];
    end
  
  end

  frame = mod(frame + 1, 1024);
end
   

f = fopen('tx_test2.outfile.ground', 'w');
fprintf(f,'%d,', O);
fclose(f);

   
   
