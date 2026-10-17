% grabstein:
vertexNr1 = 385752;
featVecNr1 = find( indices == vertexNr1 );

cc = zeros( length(indices), 19 );
for i=[ 1:length(indices) ]
    cc(i,:) = normxcorr2( features0( featVecNr1, : ), features0( i, : ) );
    if( mod( i, round( length( indices ) / 100 ) ) == 0 )
       100 * i / length( indices )
    end
end
dist = -sum(cc,2);

figure; hold on;
hist( dist, 1000 );
break;

distCut = dist;
%distCut( find( dist < cutoffLow ) ) = cutoffLow;
%distCut( find( dist > cutoffHigh ) ) = cutoffHigh;
distNormalized=(distCut-min(distCut))./(max(distCut)-min(distCut));

figure;
hist( distNormalized, 1000 );

[y,is] = sort( -distNormalized );
cmap = hot( length( indices ) );
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Integral_sort_inv.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(is)'; cmap' ] );
fclose( fid );

hotVals = hot(1000);
hotMap = hotVals( int16( floor(1+distNormalized*999) ), : );
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Integral_nosort.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; hotMap' ] )
fclose( fid );
hotMapInv = hotVals( int16( floor(1+(1-distNormalized)*999) ), : );
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Integral_nosort_inv.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; hotMapInv' ] )
fclose( fid );

[ maxShift, maxShiftIdx ] = max(cc,[],2);
figure;
hist(maxShift,1000);

hotMap = hotVals( int16( floor(1+maxShift*999) ), : );
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Maximum_nosort.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; hotMap' ] )
fclose( fid );

cmap = hot( length( indices ) );
[y,is] = sort( maxShift );
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Maximum_sort.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(is)'; cmap' ] )
fclose( fid );

cmapShiftIdx = hot( size(cc,2) );
shiftMap = cmapShiftIdx( maxShiftIdx,:);
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Maximum_shiftpos.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; shiftMap' ] )
fclose( fid );

bwMap = ones( length( indices ), 3 );
setBwIdx = find( dist > 4 );
bwMap( setBwIdx, 1 ) = 0;
bwMap( setBwIdx, 2 ) = 0;
bwMap( setBwIdx, 3 ) = 0;
fid = fopen( '../mesh/Warka_W20219_1_r0.5_255_KKR_4_Thres_lower_pos4.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; bwMap' ] )
fclose( fid );
