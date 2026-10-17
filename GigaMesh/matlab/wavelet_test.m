%fileNameIn  = '/export/home/hmara/KS_new/HOS_G10_SM2069-HE5_S060_110610_GM_Final_r2.0_255.4e.mat';
%fileNameOut = '/export/home/hmara/KS_new/HOS_G10_SM2069-HE5_S060_110610_GM_Final_r2.0_255.4e_WAVELET.mat';

%fileNameIn  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_new_r1.5_255.mat';
%fileNameOut = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_new_r1.5_255_WAVELET.mat';

%fileNameIn  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_r1.6_256_16_scales.mat';
%fileNameOut  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_r1.6_256_16_scales_WAVELET.mat';

fileNameIn  = '/export/home/hmara/Desktop/GigaMesh/Export_fuer_Hubert/100712_134332_r0.05_512.mat'
fileNameOut = '/export/home/hmara/Desktop/GigaMesh/Export_fuer_Hubert/100712_134332_r0.05_512_WAVELET.mat'

%fileNameIn  = '/export/home/hmara/3D_new/Salmanassar3_full_LessHoles_GM_Orient_r2.0_255.alt.2.mat'
%fileNameOut = '/export/home/hmara/3D_new/Salmanassar3_full_LessHoles_GM_Orient_r2.0_255.alt.2_WAVELET.mat'

dataVec    = dlmread( fileNameIn );
indices = dataVec( 1:end, 1 );
featureVec = (dataVec( 1:end, 2:end )-0.5)*2.0;
ftWave = [];

fid = fopen( fileNameOut, 'a' );

for i = 1:length( indices )
    ftWave = wavedec( featureVec(i,1:end), 3, 'haar' );
    fprintf( fid, '%i', indices(i) );
    fprintf( fid, ' %f', ftWave );
    fprintf( fid, '\n' );
    if( mod( i, 5000 ) == 0 )
        i
    end
end

fclose( fid );

