%fileNameIn  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_SelfKR_m3.tex';
%fileNameIn  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_KKR_new_Vert_4459242_m3.tex';
%fileNameIn  = '/export/home/hmara/Blackwell_Coin_r0.5_255_KKR_Vert_220437.tex';
fileNameIn  = '/export/home/hmara/KS_Final/W_21415_18/W_21415_18_edited_GM_Final_r1.0_255_KKR_Vert_2837138.tex';

%fileNamePrefix  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_SelfKR_m3';
%fileNamePrefix  = '/export/home/hmara/KS_Final/TCH.92,G.127/TCH.92,G.127_GM_Final_KKR_new_Vert_4459242_m3';
%fileNamePrefix  = '/export/home/hmara/KS_Final/Blackwell_Coin_r0.5_255_KKR_Vert_220437';
fileNamePrefix  = '/export/home/hmara/KS_Final/W_21415_18/W_21415_18_edited_GM_Final_r1.0_255_KKR_Vert_2837138';

featureVec = dlmread( fileNameIn );
indices    = featureVec( 1:end, 1 );
invertmap  = 1.0-featureVec( 1:end, 2:end );

fileNameTex = [ fileNamePrefix '_invert.tex' ];
fid = fopen( fileNameTex, 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; invertmap' ] );
fclose( fid );


graymap    = featureVec( 1:end, 2 ); % 3 (green) and 4 (blue) are ignored.
[y,is] = sort( graymap );
cmap = hot( length( indices ) );

fileNameTex = [ fileNamePrefix '_Hot_Equal.tex' ];
fid = fopen( fileNameTex, 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(is)'; cmap' ] );
fclose( fid );

fileNameTex = [ fileNamePrefix '_Hot_Equal_Inverted.tex' ];
fid = fopen( fileNameTex, 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(is)'; fliplr(cmap') ] );
fclose( fid );

graymap = 1-graymap;

thresCI = mode( graymap )*1.0;
[ idxLow ] = find( graymap <= thresCI );
[ idxHigh ] = find( graymap > thresCI );

fileNameTex = [ fileNamePrefix '_ThresMode12.tex' ];
fid = fopen( fileNameTex, 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(idxLow)'; repmat( (graymap( idxLow, : )')/(thresCI), 3, 1 ) ] );
fprintf( fid, '%i %f %f %f\n', [ indices(idxHigh)'; ones( 3, length( idxHigh ) ) ] );
fclose( fid );
