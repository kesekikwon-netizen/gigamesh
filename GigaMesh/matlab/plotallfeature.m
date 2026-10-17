fileNameIn = 'Schriften/grabstein_textfeld.mat';
fileNameOut = 'Schriften/grabstein_textfeld_BlackWhite_12.tex';
%fileNameIn = 'Schriften/keilschrift-s_holes_filled_frac_removed_r1.00_128.mat';
%fileNameOut = 'Schriften/keilschrift-s_holes_filled_frac_removed_r1.00_128_BlackWhite_02.tex';
%fileNameIn = 'Schriften/ttt3-01.mat';
%fileNameOut = 'Schriften/ttt3-01_BlackWhite_03.tex';
featVecUse = 2:11;
invertTex = false;

featureVec = dlmread( fileNameIn );
noFigures = true;

nrElements = size( featureVec, 1 );
texMap = zeros( size( featureVec, 1 ), 4 );
featureCenter = ones( size( featureVec( :, featVecUse ) ) ) .* 0.5;
distToCenter  = sqrt( sum( ( featureVec( :, featVecUse ) - featureCenter ).^2, 2 ) );

% some threshholding to eliminate outlier
thresDistHigh = mean( distToCenter ) + 1.0*std( distToCenter );
thresDistLow  = mean( distToCenter ) - 1.0*std( distToCenter );
idxHigh = find( distToCenter > thresDistHigh );
idxLow  = find( distToCenter < thresDistLow );
distToCenter( idxHigh ) = thresDistHigh;
distToCenter( idxLow )  = thresDistLow;

% normalize
distToCenter = (distToCenter-min( distToCenter )) ./ (max( distToCenter )-min( distToCenter ));
hist( distToCenter );

if not( noFigures ) 
    lenFeatureVec = size( featureVec, 2 )-1;
    plotBase = [1:lenFeatureVec] .* 10;
    cmap = colormap( jet( size( featureVec, 1 ) ) );

    figAll = figure; hold on; grid on;
    title('All feature vectors');
    xlabel( 'radius in %' );
    ylabel( 'sphere volume in %' );

    figConcav = figure; hold on; grid on;
    title( 'Complete Concav Vertices' );
    xlabel( 'radius in %' );
    ylabel( 'sphere volume in %' );

    figConvex = figure; hold on; grid on;
    title( 'Complete Convex Vertices' );
    xlabel( 'radius in %' );
    ylabel( 'sphere volume in %' );

    figMixed = figure; hold on; grid on;
    title( 'Concav and Convex Vertices' );
    xlabel( 'radius in %' );
    ylabel( 'sphere volume in %' );
end

for idx = 1:size( featureVec, 1 )
    if not( noFigures )
        figure( figAll );
        plot( plotBase, featureVec( idx, 2:end ).*100, '.-', 'Color', cmap( idx, : ) );
    end
    %texMap( idx, : ) = [ featureVec( idx, 1 ), 1-(distToCenter(idx))^(2/1), 1-(distToCenter(idx))^(2/1), 1-(distToCenter(idx))^(2/1) ];
    scales = sum( featureVec( idx, featVecUse ) < 0.5 );
    texMap( idx, : ) = [ featureVec( idx, 1 ), scales/10, scales/10, scales/10 ];
    continue;
    if( sum( featureVec( idx, featVecUse ) < 0.5 ) == 0 )
        % CONCAV
        if not( noFigures )
            figure( figConcav );
            plot( plotBase, featureVec( idx, 1:end-1 ).*100, '.-', 'Color', cmap( idx, : ) );
        end
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0, 0.0, 0.0 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0*distToCenter(idx), 1.0*distToCenter(idx), 0.5*distToCenter(idx) ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0-distToCenter(idx)/3, 1.0-distToCenter(idx)/3, 1.0-distToCenter(idx)/3 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0-distToCenter(idx), 1.0-distToCenter(idx), 1.0-distToCenter(idx) ];
    elseif( sum( featureVec( idx, featVecUse ) > 0.5 ) == 0 )
        % CONVEX
        if not( noFigures )
            figure( figConvex );
            plot( plotBase, featureVec( idx, 1:end-1 ).*100, '.-', 'Color', cmap( idx, : ) );
        end
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 0.0, 1.0, 0.0 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0*distToCenter(idx), 0.0, 0.0 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), (1.0-distToCenter(idx))/3, (1.0-distToCenter(idx))/3, (1.0-distToCenter(idx))/3 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0-distToCenter(idx), 1.0-distToCenter(idx), 1.0-distToCenter(idx) ];
    else
        % MIX
        if not( noFigures )
            figure( figMixed );
            plot( plotBase, featureVec( idx, 1:end-1 ).*100, '.-', 'Color', cmap( idx, : ) );
        end
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 0.0, 0.0, 1.0 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 0.0, 1.0*distToCenter(idx), 0.0 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), distToCenter(idx)/3, distToCenter(idx)/3, distToCenter(idx)/3 ];
        %texMap( idx, : ) = [ featureVec( idx, 1 ), 1.0-distToCenter(idx), 1.0-distToCenter(idx), 1.0-distToCenter(idx) ];
    end
end
%texMap( idxHigh, 2:4 ) = 1.0;
%texMap( idxLow,  2:4 ) = 0.0;

dlmwrite( fileNameOut, texMap, 'delimiter', ' ', 'precision', 10 );