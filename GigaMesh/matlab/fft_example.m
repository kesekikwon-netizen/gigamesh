clear all; close all;

someFunctions{ 1 } = [ 0.0  0.0  1.0  1.0  1.0  1.0  1.0  1.0  1.0 ...
                                 0.0  0.0  0.0  0.0  0.0  0.0  0.0 ...
                                -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 ...
                                 0.0  0.0  0.0  0.0  0.0  0.0  0.0 ] + 0.3;
someFunctions{ 2 } = [ 0.0  0.5  1.0  0.5 ...
                       0.0 -0.5 -1.0 -0.5 ...
                       0.0  0.5  1.0  0.5 ...
                       0.0 -0.5 -1.0 -0.5 ...
                       0.0  0.0  0.0  0.0  0.0  0.0  0.0 ] - 0.2;

selectFuncNr =  2;
NFFT         = 32;
NFFT_USE     = 10; % hast to be >1 and <NFFT

dec  = someFunctions{ selectFuncNr };
clear someFunctions;

step = 360/(length(dec)-1);
asc  = 0:step:360;

% fft returns the frequency spectrum doublesided:
y_timedomain_doublesided = fft( dec, NFFT*2 );
m = length( y_timedomain_doublesided ); 
% we wan't only Y_TimeDomain_Singlesided:
y_tds = y_timedomain_doublesided( 1:NFFT );
M     = length( y_tds );

% sampling for the new function:
stepFD = 2*pi/(10*(length(dec)-1));
xFD    = 0:stepFD:2*pi;
% use only low frequencies:
n      = 1:(NFFT_USE-1);
scale  = (length(dec)-1)/(NFFT*2);
% reconstruct the original function 
a0 =  y_tds( 1 )/m;  % Frequency Zero - euals mean( dec )
an =  2*real( y_tds(2:NFFT_USE) )/m;
bn = -2*imag( y_tds(2:NFFT_USE) )/m;
yFD = a0 + an*cos(n'*xFD*scale) + bn*sin(n'*xFD*scale);
   
figure; 
subplot( 1, 3, 1 );
hold on; grid on;
plot( asc, dec, 'ro-', 'Linewidth', 2 );
plot( 180*xFD/pi, yFD, 'Linewidth', 2 );
xlim( [0 360] ); axis square;
ylabel('Function Value');
title( [ '{\bf Function No ' int2str( selectFuncNr ) ' - NFFT single sided ' int2str( NFFT ) ' used all and ' int2str( NFFT_USE ) '}' ] );

all_fq_n  =  1:NFFT-1;
all_fq_an =  2*real( y_tds(2:NFFT) )/m;
all_fq_bn = -2*imag( y_tds(2:NFFT) )/m;
all_fq_yFD = a0 + all_fq_an*cos(all_fq_n'*xFD*scale) + all_fq_bn*sin(all_fq_n'*xFD*scale);
plot( 180*xFD/pi, all_fq_yFD, '-.', 'Linewidth', 2, 'Markersize', 8, 'Color', [ 0.5 0.5 0.5 ] );
legend( 'Some Function',[ 'DFT Interpolant using ' int2str( NFFT_USE ) ], [ 'DFT Interpolant using ' int2str( NFFT ) ],'Location','NE' );

subplot( 1, 3, 2 );
hold on; grid on;
title( [ '{\bf Frequencies used (' int2str( NFFT_USE ) ')}' ] );
frequencies_used = repmat( an', 1, length(xFD) ) .* cos(xFD'*n*scale)' +  repmat( bn', 1, length(xFD) ) .* sin(xFD'*n*scale)';
plot( 180*xFD/pi, frequencies_used' );
xlim( [0 360] ); axis square;


subplot( 1, 3, 3 );
hold on; grid on;
title( [ '{\bf All Frequencies (' int2str( NFFT ) ')}' ] );
frequencies_all = repmat( all_fq_an', 1, length(xFD) ) .* cos(xFD'*all_fq_n*scale)' +  repmat( all_fq_bn', 1, length(xFD) ) .* sin(xFD'*all_fq_n*scale)';
plot( 180*xFD/pi, frequencies_all' );
xlim( [0 360] ); axis square;
