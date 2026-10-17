fileNameIn = 'Rollsiegel_2_SM2066-HE5-S-060001_filled_orient_GM_Final_r1.5.pline';

% array for labels
label = [];
% element array for polylines:
polylines = {};

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
    % to be used to get an polyLength x 3 array for the vertices
    % coordinates:
    polylines{ end+1 } = reshape( lineVals( 3:end ), 3, polyLength );
    tline = fgetl( fid );
end
fclose( fid );
% we can remove temporary varialbles, including polyLength as 
% length( polylines{ someIdx } ) will do the trick
clear fid tline lineVals polyLength; 
fprintf( '%i Polylines parsed.\n', length(polylines) )

% color by label and plot:
cmapLabel = hsv( max( label ) ); % will cause problems, when only one line is omitted
figure; hold on; axis equal; axis vis3d; grid on;
title( 'Polylines colored by Label No.' );
for i=1:length( polylines )
    plot3( polylines{i}(1,:), polylines{i}(2,:), polylines{i}(3,:), '.-', 'Color', cmapLabel( label(i), : ) );
end
