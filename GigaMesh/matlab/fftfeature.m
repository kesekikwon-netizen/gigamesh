%fileNameIn = 'Schriften/grabstein_textfeld.mat';
%fileNameOut = 'Schriften/grabstein_textfeld_BlackWhite_12.tex';
%fileNameIn = '../mesh/Schriften/Worms_Grabstein/grabstein_textfeld_holes_filled_orientated_r2.5_256.mat';
%fileNameOut = '../mesh/Schriften/Worms_Grabstein/grabstein_textfeld_holes_filled_orientated_r2.5_256_FFT.mat';
%fileNameOut2 = '../mesh/Schriften/Worms_Grabstein/grabstein_textfeld_holes_filled_orientated_r2.5_256_FFT_Weight.mat';
%fileNameIn = '../mesh/Schriften/keilschrift-s_holes_filled_frac_removed_r1.00_128.mat';
%fileNameOut = '../mesh/Schriften/keilschrift-s_holes_filled_frac_removed_r1.00_128_FFT.mat';
%fileNameIn = 'Schriften/ttt3-01.mat';
%fileNameOut = 'Schriften/ttt3-01_BlackWhite_03.tex';

fileNameIn  = '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255.mat';
fileNameOut  = '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255.fft';
fileNameOut2 = '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255.xyz';

featureVec = dlmread( fileNameIn );
indices = featureVec( 1:end, 1 );
features = featureVec( 1:end, 2:end );
featNonZeroIdx = find( sum( features == 0, 2 ) < 10 );
featuresNonZero = features( featNonZeroIdx, : );

featAvg = mean( featuresNonZero, 2 );
features0 = featuresNonZero - 0.5;
%features0 = featuresNonZero - repmat( featAvg, 1, 10 );
break;

%fm=fft(features0);
%xBase = repmat( [ 0:9 ], size( fm, 1 ),1 );

L=10; Fs=1;
NFFT = 2^nextpow2(L);
Y = fft(features0,NFFT,2)/L;
%Y = fft(features0(1:100:end,:),NFFT,2)/L;
%Y = fft(features0(1:100,:),NFFT,2)/L;
f = Fs/2*linspace(0,1,NFFT/2+1);

% Basis for plot
fBase = repmat( [0:NFFT-1], size( Y, 1 ),1 );

% Plot single-sided amplitude spectrum.
figure;
plot( fBase, abs(Y),'.' );
title('Single-Sided Amplitude Spectrum of y(t)');
xlabel('Frequency (Hz)');
ylabel('|Y(f)|');
axis( [ 0 NFFT, -0.5 0.5 ] );

figure;
plot( fBase, real(Y),'.' );
title('Single-Sided Real Amplitude Spectrum of y(t)');
xlabel('Frequency (Hz)');
ylabel('|Y(f)|');
axis( [ 0 NFFT, -0.5 0.5 ] );

figure;
plot( fBase, imag(Y),'.' );
title('Single-Sided Imag Amplitude Spectrum of y(t)');
xlabel('Frequency (Hz)');
ylabel('|Y(f)|');
axis( [ 0 NFFT, -0.5 0.5 ] );

newMat = [indices(featNonZeroIdx)';abs(Y')]';

dlmwrite( fileNameOut, newMat, 'delimiter', ' ', 'precision', 10 );

weightVec = ([NFFT:-1:1]/NFFT).^2;
newMat = [indices(featNonZeroIdx)';abs((Y.*weightArr)')]';
dlmwrite( fileNameOut2, newMat, 'delimiter', ' ', 'precision', 10 );
