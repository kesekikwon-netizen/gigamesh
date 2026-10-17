%fileNameIn = '../HOS-G10_w_FTVecs_FuncVal_Polylines.txt';
%fileNameIn = '~/Documents/diss/figs/chapter_4/polyline_curvature/HOS-G10_w_FTVecs_FuncVal_Polylines_smooth_3_variants.txt';
fileNameIn = '~/Documents/diss/figs/chapter_4/polyline_intinv/HOS-G10_w_FTVecs_FuncVal_Polylines_PART_II_Radius_Zero_0.25_0.050.txt';

fileNameIn = '../polyline_test_subdiv_10k_verts.txt';
fileNameIn = '../polyline_test_subdiv_10k_verts_displaced.txt';

fileNameIn = '~/Documents/diss/figs/chapter_4/polyline_intinv_runlen/HOS-G10_w_FTVecs_FuncVal_Polylines_PART_IntInvRunLen_COMBINED.txt';

fileNameIn = '~/Documents/diss/figs/chapter_4/compare_curv_intinv_large_scale/HOS-G10_w_FTVecs_FuncVal_Polylines_Compare.txt';
fileNameIn = '~/Documents/diss/figs/chapter_4/compare_curv_intinv_large_scale/HOS-G10_w_FTVecs_FuncVal_Polylines_Compare_1.5mm.txt';

fileNameIn = '~/Documents/diss/figs/chapter_4/poly_intinv_noise/polyline_test_subdiv_10k_verts_displaced_shifted_gauss_w1.4.txt';
fileNameIn = '~/Documents/diss/figs/chapter_4/poly_intinv_noise/polyline_test_subdiv_10k_verts_displaced_shifted_angle_r1.4.txt';
fileNameIn = '~/Documents/diss/figs/chapter_4/poly_intinv_noise/polyline_test_subdiv_10k_verts_displaced_shifted_runlen_r1.4.txt';


% array for labels:
label = [];
% element array for the run-length of the edges:
polyrunlen = {};
polyrunlenACC = {};
% element array for the function value i.e. curvature
polyfuncval = {};
% array for the tag determining a closed/open polyline:
polyclose = [];

fid = fopen( fileNameIn, 'r' );

% fetch first line
tline = fgetl( fid );
while ischar( tline )
    % lines with comments begin with # and can be ignored.
    if( tline(1) == '#' )
        disp( tline );
        tline = fgetl( fid );
        continue;
    end
    % parse the rest:
    lineVals = sscanf( tline, '%f' );
    % first element is the label no. 
    % labels from gigamesh start at 0 - so we have to add 1
    label( end+1 ) = lineVals( 1 ) +1;
    % second element is the length of the polyline
    polyLength = lineVals( 2 );
    % third element is the open/close tag of the polyline
    polyclose( end+1 ) = lineVals( 3 );
    % to be used to get an polyLength x 3 array for the vertices
    % coordinates:
    polyrunlen{ end+1 }  = lineVals( 4:3+polyLength );
    polyfuncval{ end+1 } = lineVals( 4+polyLength:3+polyLength*2 );
    polyrunlenACC{ end+1 } = [ ];
    runLenAcc = 0;
    for i=1:length( polyrunlen{ end } )
        runLenADD = polyrunlen{ end }(i);
        runLenAcc = runLenAcc + runLenADD;
        polyrunlenACC{ end }( end+1 ) = runLenAcc;
    end
    tline = fgetl( fid );
end
fclose( fid );
% we can remove temporary varialbles, including polyLength as 
% length( polylines{ someIdx } ) will do the trick
clear fid tline lineVals polyLength; 
fprintf( '%i Polylines parsed.\n', length(polyrunlen) )

% plot:
for i=1:length( polyrunlen )
    figure; hold on;
    plot( polyrunlenACC{i}, polyfuncval{i}, 'k.-' );
    % for fileNameIn = '~/Documents/diss/figs/chapter_4/polyline_intinv/HOS-G10_w_FTVecs_FuncVal_Polylines_PART_II_Radius_Zero_0.25_0.050.txt';
    %axis( [ 0 9 -pi 0 ] );
    %plot( [ 0 9 ], [ -pi -pi ]./4, 'k-' );
    %plot( [ 0 9 ], [ -pi -pi ]./2, 'k-' );
    %plot( [ 0 9 ], [ -pi -pi ].*3/4, 'k-' );
end

% for fileNameIn = '~/Documents/diss/figs/chapter_4/poly_intinv_noise/*'
polyFuncValCorr = [ polyfuncval{2}( 296:end ); polyfuncval{2}( 1:295 ) ];
figure; hold on;
plot( polyrunlenACC{2}, polyFuncValCorr/2, 'k-' );
%plot( polyrunlenACC{3}, polyfuncval{3}/2, 'k-' );
%scale=max(polyrunlenACC{3})/max(polyrunlenACC{2});
%plot( polyrunlenACC{2}*scale, polyFuncValCorr*scale/2, 'b-' );
%scale=2/min(polyfuncval{2});
%plot( polyrunlenACC{2}*scale, polyFuncValCorr*scale/2, 'r-' );
plot( polyrunlenACC{2}/2, polyFuncValCorr/4, 'g-' );
axis( [ 0 14.5 0.85 1.75 ] ); % approx. axis tight

% angle:
%plot( polyrunlenACC{3}, polyfuncval{3}, 'k-' );
%plot( polyrunlenACC{2}/2, polyFuncValCorr, 'g-' );
%axis( [  0 14.5 -pi 0 ] );
%plot( [ 0 14.5 ], -[ pi/2 pi/2 ], 'k-' );
%plot( [ 0 14.5 ], -[ pi/4 pi/4 ], 'k-' );
%plot( [ 0 14.5 ], -3*[ pi/4 pi/4 ], 'k-' );