clear all; close all;

% Grabstein Worms 1061
refVertices = [ 964491 2000944 194691 1177123 ]; % char, char, char, dot

clear all; close all;

% Warka 20219_1 - small ridge seems to best_
refVertices = [ 228725 188026 461346 514219 ]; % ridge, corner center, small ridge, corner out
fileNamePrefix = '../mesh/Warka_W20219_1_r0.5_255';
fileNameIn     = '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255.mat';

clear all; close all;
  
% TCH.92,G.127 (from Stefan Jakob) 
refVertices = [ 4459242 1824502 2407400 583968 ]; % long ridge, cunei center, cunei end point, cunei center II
fileNamePrefix = '~/KS_Final/TCH.92,G.127/TCH.92,G.127_r0.5_255';
fileNameIn     = '~/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_r1.0_256.mat';

clear all; close all;
  
% INV4279_SM1282-450 (from Stephan Karl) 
refVertices = [ 323301 ]; % long ridge
fileNamePrefix = '~/Desktop/3d_data/Graz/INV4279_SM1282-450';
fileNameIn     = '~/Desktop/3d_data/Graz/INV4279_SM1282-450_GM_Final_r2.5_255.mat';

clear all; close all;

% Ephesos Taeuber
refVertices = [ 188907 88506 187444 ]; % char, char endpoint noisy, char O nice
fileNamePrefix = '~/Ephesos_Inschrift_Taeuber_GM_Final_r2.5';
fileNameIn     = '~/Ephesos_Inschrift_Taeuber_GM_Final_r2.5_225.mat';

featureVec = dlmread( fileNameIn );
indices = featureVec( 1:end, 1 );
features = featureVec( 1:end, 2:end );
featNonZeroIdx = find( sum( features == 0, 2 ) < 10 );
featuresNonZero = features( featNonZeroIdx, : );

featAvg = mean( featuresNonZero, 2 );
features0 = featuresNonZero - 0.5;

% cross-correlation between selected vertices:
ccRefRefIntegral = zeros( length( refVertices ) );
ccRefRefMaximum  = zeros( length( refVertices ) );
ccRefCtr2 = 1;
for refVertex1 = refVertices
    ccRefCtr1 = 1;
    for refVertex2 = refVertices
        featVecNr1 = find( indices == refVertex1 );
        featVecNr2 = find( indices == refVertex2 );
        ccRefRefIntegral( ccRefCtr1, ccRefCtr2 ) = sum( normxcorr2( features0( featVecNr1, : ), features0( featVecNr2, : ) ) );
        ccRefRefMaximum( ccRefCtr1, ccRefCtr2 ) =  max( normxcorr2( features0( featVecNr1, : ), features0( featVecNr2, : ) ) );
        ccRefCtr1 = ccRefCtr1 + 1;
    end
	ccRefCtr2 = ccRefCtr2 + 1;
end
    
ccRefs = {};
ccRefCtr = 1;

for refVertex = refVertices

    featVecNr = find( indices == refVertex );

    ccRefs{ ccRefCtr } = zeros( length(indices), 19 );
    for i=[ 1:length(indices) ]
        ccRefs{ ccRefCtr }(i,:) = normxcorr2( features0( featVecNr, : ), features0( i, : ) );
        if( mod( i, round( length( indices ) / 100 ) ) == 0 )
            fprintf( '%i %i %03.0f\n', ccRefCtr, featVecNr, 100 * i / length( indices ) );
        end
    end
    
    dist = -sum(ccRefs{ ccRefCtr },2);
    figure; hold on;
    hist( dist, 1000 );
    
    ccRefCtr = ccRefCtr + 1;
end

for refVertexIdx = 1:length(refVertices)
    fileNameKKR = [ fileNamePrefix  '_KKR_' int2str( refVertices(refVertexIdx) ) '.mat' ];
    ccRef = ccRefs{ refVertexIdx };
    save( fileNameKKR, 'ccRef' );
end

cmap = hot( length( indices ) );
for refVertexIdx = 1:length(refVertices)
    [ maxShift, maxShiftIdx ] = max( ccRefs{refVertexIdx}, [], 2 );
    [y,is] = sort( maxShift );

    fileNameTex = [ fileNamePrefix '_KKR_Vertex_' int2str( refVertices(refVertexIdx) ) '_Maximum_Sort.tex' ];
    fid = fopen( fileNameTex, 'w');
    fprintf( fid, '%i %f %f %f\n', [ indices(is)'; cmap' ] );
    fclose( fid );

    fileNameTex = [ fileNamePrefix '_KKR_Vertex_' int2str( refVertices(refVertexIdx) ) '_Maximum_Sort_Invert.tex' ];
    fid = fopen( fileNameTex, 'w');
    fprintf( fid, '%i %f %f %f\n', [ indices(is)'; fliplr( cmap' ) ] );
    fclose( fid );

    distCut = -sum( ccRef, 2 );
    cutoffLow = -4;
    cutoffHigh = 0;
    distCut( find( dist < cutoffLow ) ) = cutoffLow;
    distCut( find( dist > cutoffHigh ) ) = cutoffHigh;
    distNormalized=(distCut-min(distCut))./(max(distCut)-min(distCut));
    hotVals = hot(1000);
    hotMap = hotVals( int16( floor(1+distNormalized*999) ), : );
    fileNameTex = [ fileNamePrefix '_KKR_Vertex_' int2str( refVertices(refVertexIdx) ) '_Integral_Nosort_Cutoff_4_0.tex' ];
    fid = fopen( fileNameTex, 'w');
    fprintf( fid, '%i %f %f %f\n', [ indices'; hotMap' ] )
    fclose( fid );
end
