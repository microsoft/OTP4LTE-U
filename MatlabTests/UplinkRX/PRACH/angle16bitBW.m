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

function [angle16] = angle16bitBW(x)
%Usage: [angle] = angle16bitBW(x)
%performs 4 - quarter angle approximation from a 16b+16b complex phasor x
%uses 2x 32-element,16bit lookup tables, 1/2 quarter complex plane split
%and 1st derivative Taylor expansion
% cost of lookup is 1 division, 2 table lookups, 4 conditionals, bitshifts and multiplications
%Accuracy to 0.012071deg over entire 16-bit complex phasor range

scaleBits = 15;
scale = (2^scaleBits) - 1;
halfScale = (scale+1)/2;

tableBits = 5;
interpBits = scaleBits - tableBits;
halfInterp = (2^(interpBits-1));

nTable = 2^tableBits;
tableVec = ( (0:(nTable)) )/ nTable;
angleTable0 = round( (scale/pi) * atan(tableVec) );%main atan lookup table, 16-bit
angleTable2 = round((scale/pi)./(1+tableVec.^2));%first derivative lookup table, 16-bit

angle16 = zeros(size(x));
for count = 1:length(x)
  
  re = real(x(count));
  im = imag(x(count));
  reA = abs(re);
  imA = abs(im);
  
  if ( (re ~= 0) || (im ~= 0) )
    
    if (reA > imA)
      %      tn = floor( (floor(reA/2) + tableScale*imA) / reA );
      tnx = floor( (floor(reA/2) + scale*imA) / reA );
      
      if (re > 0)
        offsetVal = 0;
        if im >= 0
          signVal = 1;
        else
          signVal = -1;
        end;
      else
        if im > 0
          offsetVal = scale;
          signVal = -1;
        else
          offsetVal = -scale;
          signVal = 1;
        end;
      end;
      
    else %(reA < imA)
      tnx = floor( (floor(imA/2) + scale*reA) / imA );
      if (re > 0)
        if im >= 0
          offsetVal = halfScale;
          signVal = -1;
        else
          offsetVal = -halfScale;
          signVal = 1;
        end;
      else
        if im >= 0
          offsetVal = halfScale;
          signVal = 1;
        else
          offsetVal = -halfScale;
          signVal = -1;
        end;
      end;
    end;
    
  else %zero angle
    offsetVal = 0;
    signVal = 0;
    tn = 0;
    tnx = 0;
  end;
  
  tn = floor( (halfInterp+tnx) / (2^interpBits) ); %lookup scaled atan() value
  tnDiff = tnx - (2^interpBits)*tn; %residual
  interpVal = angleTable0(1+tn) + floor( ( halfScale + tnDiff * angleTable2(1+tn) ) / (2^scaleBits) ); %Taylor series
  %produce final value
  angle16(count) = offsetVal + signVal*interpVal;
end;

end
