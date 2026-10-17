//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include <ctime>
#include <mutex>
#include <future>

#include <GigaMesh/mesh/compfeaturevecs.h>

// Multithreading (CPU):
#define THREADS_VERTEX_BLOCK  5000
std::mutex stdoutMutex;

//! Compute the Multi-Scale Integral Invariant feature vectors.
//! Function to be called multiple times depending on the requested
//! number of threads. Typically called by compFeatureVectorsMain.
void compFeatureVectorsThread(
                sMeshDataStruct*   rMeshData,
                const size_t       rThreadOffset,
                const size_t       rThreadVertexCount
) {

	const int threadID = rMeshData->threadID;

	{
		std::lock_guard<std::mutex> lock(stdoutMutex);
		std::cout << "[GigaMesh] Thread " << threadID  << " started, processing "
		          << rThreadVertexCount << " of vertices ("
		          << static_cast<double>(rThreadVertexCount)/
		             static_cast<double>(rMeshData->meshToAnalyze->getVertexNr())*100.0
		          << " % of total)" << std::endl;
	}

	time_t timeStampThread = time( nullptr ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )

	// initalize values to be returned via meshDataStruct:
	rMeshData->ctrIgnored   = 0;
	rMeshData->ctrProcessed = 0;

	// copy pointers from struct for easier access.
	double* tDescriptVolume     = rMeshData->descriptVolume;  //!< Volume descriptors
	double* tDescriptSurface    = rMeshData->descriptSurface; //!< Surface descriptors
	std::vector<MeshIO::grVector3ID>* tNormalSurfacePatch = rMeshData->mPatchNormal;   //!< Surface patch normal

	// setup memory for rastered surface:
	const uint   rasterSize  = rMeshData->xyzDim * rMeshData->xyzDim;
	double*      rasterArray = new double[rasterSize];

	// Processing time
	std::chrono::system_clock::time_point tStart = std::chrono::system_clock::now();

	// Allocate the bit arrays for vertices:
	uint64_t* vertBitArrayVisited{nullptr};
	const uint64_t vertNrLongs = rMeshData->meshToAnalyze->getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	uint64_t* faceBitArrayVisited{nullptr};
	const uint64_t faceNrLongs = rMeshData->meshToAnalyze->getBitArrayFaces( &faceBitArrayVisited );

	// Compute absolut radii (used to normalize the surface descriptor)
	double* absolutRadii = new double[rMeshData->multiscaleRadiiSize];
	for( uint i=0; i<rMeshData->multiscaleRadiiSize; i++ ) {
		absolutRadii[i] = static_cast<double>(rMeshData->multiscaleRadii[i]) *
		                  static_cast<double>(rMeshData->radius);
	}

	// Step thru vertices:
	Vertex*            currentVertex{nullptr};
	std::vector<Face*> facesInSphere; // temp variable to store local surface patches - returned by fetchSphereMarching

	for( size_t vertexOriIdxInProgress = rThreadOffset;
	            vertexOriIdxInProgress < (rThreadOffset + rThreadVertexCount);
	                                                    ++vertexOriIdxInProgress )
	{

		currentVertex = rMeshData->meshToAnalyze->getVertexPos( vertexOriIdxInProgress );
		if( currentVertex == nullptr ) {
			std::cout << "[GigaMesh] ERROR: Thread " << threadID  << " bad vertex id!" << std::endl;
			continue;
		}

		// Fetch faces within the largest sphere
		facesInSphere.clear();
		// slower for larger patches:
		//meshData->meshToAnalyze->fetchSphereMarching( currentVertex, &facesInSphere, meshData->radius, true );
		//meshData->meshToAnalyze->fetchSphereMarchingDualFront( currentVertex, &facesInSphere, meshData->radius, true );
		//meshData->meshToAnalyze->fetchSphereBitArray( currentVertex, &facesInSphere, meshData->radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );
		rMeshData->meshToAnalyze->fetchSphereBitArray1R( currentVertex, facesInSphere, rMeshData->radius, 
		                                                 vertNrLongs, vertBitArrayVisited, 
		                                                 faceNrLongs, faceBitArrayVisited, false );

		// Fetch and store the normal used in fetchSphereCubeVolume25D as it is a quality measure
		if( tNormalSurfacePatch ) {
			double normalXYZ[3]{0.0,0.0,0.0};
			std::vector<Face*>::iterator itFace;
			for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
				// from OLD version: (*itFace)->addNormalTo( &(tSurfacePatchNormal[vertexOriIdxInProgress*3]) );
				(*itFace)->addNormalXYZTo( normalXYZ, false );
			}
			tNormalSurfacePatch->at(vertexOriIdxInProgress) = MeshIO::grVector3ID{ static_cast<unsigned long>(vertexOriIdxInProgress),
			                                                                       normalXYZ[0], normalXYZ[1],
			                                                                       normalXYZ[2] };
		}

		// Pre-compute address offset
	    auto descriptIndexOffset = vertexOriIdxInProgress*rMeshData->multiscaleRadiiSize;

		// Get volume descriptor:
		if( tDescriptVolume ) {
			rMeshData->meshToAnalyze->fetchSphereCubeVolume25D( currentVertex, &facesInSphere,
			                                                   rMeshData->radius, rasterArray,
			                                                   rMeshData->xyzDim );
			applyVoxelFilters2D( &(tDescriptVolume[descriptIndexOffset]), rasterArray,
			                        rMeshData->sparseFilters, rMeshData->multiscaleRadiiSize, rMeshData->xyzDim );
		}

		// Get surface descriptor:
		if( tDescriptSurface ) {
			// Vector3D seedPosition = currentVertex->getCenterOfGravity();
			rMeshData->meshToAnalyze->fetchSphereArea( currentVertex, &facesInSphere,
			                                          static_cast<unsigned int>(rMeshData->multiscaleRadiiSize),
			                                          absolutRadii, &(tDescriptSurface[descriptIndexOffset]) );
		}
		// Set counters:
		rMeshData->ctrProcessed++;

		if(  !( ( vertexOriIdxInProgress - rThreadOffset ) % THREADS_VERTEX_BLOCK ) &&
		                                                        (vertexOriIdxInProgress - rThreadOffset) ) {
			// Show a time estimation:

			const double percentDone = static_cast<double>(vertexOriIdxInProgress - rThreadOffset) /
			                                                    static_cast<double>(rThreadVertexCount);

			std::chrono::system_clock::time_point tEnd = std::chrono::system_clock::now();
			//std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>( tEnd - tStart );
			double time_elapsed = ( std::chrono::duration<double>( tEnd - tStart ) ).count();
			double time_remaining =   (time_elapsed / percentDone) - time_elapsed;
			std::chrono::system_clock::time_point tFinalEst = tEnd + std::chrono::seconds( static_cast<long>( time_remaining ) );
			std::time_t ttp = std::chrono::system_clock::to_time_t(tFinalEst);

			{
				std::lock_guard<std::mutex> lock(stdoutMutex);

				std::cout << "[GigaMesh] Thread " << threadID << " | " << percentDone*100 << " percent done. Time elapsed: " << time_elapsed << " - ";
				std::cout << "remaining: " << time_remaining << " seconds. ";
				std::cout << vertexOriIdxInProgress/time_elapsed << " Vert/sec. ";
				std::cout << "ETF: " << std::ctime(&ttp);
				std::cout << std::flush;
			}
		}
	}

	// Volume descriptor
	delete[] rasterArray;
	delete vertBitArrayVisited;
	delete faceBitArrayVisited;
	// Surface descriptor
	delete[] absolutRadii;

	rMeshData->mWallTimeThread = static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampThread ); // seconds

	{
		std::lock_guard<std::mutex> lock(stdoutMutex);
		std::cout << "[GigaMesh] Thread " << threadID << " | STOP - processed: " << rMeshData->ctrProcessed
		          << " and skipped " << rMeshData->ctrIgnored << " vertices."
		          << " Walltime: " << rMeshData->mWallTimeThread << " seconds." << std::endl;
	}
} // END of compFeatureVectorsThread

//! Compute the Multi-Scale Integral Invariant feature vectors.
//! Main function to be called. Takes care about multi-threaded
//! computation.
void compFeatureVectorsMain(
                sMeshDataStruct*   rMeshData,
                const unsigned int rThreadVertexCount
) {
	// Sanity check
	if( rMeshData == nullptr ) {
		std::cout << "[GigaMesh::" << __FUNCTION__ << "] ERROR: nullptr given!" << std::endl;
		return;
	}

	// +++ Time for parallel processing
	time_t rawtime;
	struct tm* timeinfo{nullptr};
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	time_t timeStampParallel = time( nullptr ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )
	std::cout << "[GigaMesh] Time started: " << asctime( timeinfo );// << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	// --- Time for parallel processing

	int ctrIgnored{0};
	int ctrProcessed{0};

	uint64_t nrOfVerticesInMesh = rMeshData[0].meshToAnalyze->getVertexNr();

	if( rThreadVertexCount < 2 ) {
		std::cout << "[GigaMesh] SINGLE Thread started" << std::endl;
		compFeatureVectorsThread( rMeshData, 0, nrOfVerticesInMesh );
	} else {
		const uint64_t offsetPerThread{ nrOfVerticesInMesh/rThreadVertexCount };
		const uint64_t verticesPerThread{ offsetPerThread };
		const uint64_t offsetThisThread{ verticesPerThread*(rThreadVertexCount - 1) };
		const uint64_t verticesThisThread{ offsetPerThread +
			                           nrOfVerticesInMesh %
			                           rThreadVertexCount };

		std::vector<std::future<void>> threadFutureHandlesVector(rThreadVertexCount - 1);

		for( unsigned int threadCount = 0;
		     threadCount < (rThreadVertexCount - 1); threadCount++ ) {
			auto functionCall = std::bind( &compFeatureVectorsThread,
			                               &(rMeshData[threadCount]),
			                               offsetPerThread*threadCount,
			                               verticesPerThread );

			threadFutureHandlesVector.at(threadCount) =
			        std::async(std::launch::async, functionCall);
		}

		compFeatureVectorsThread( &(rMeshData[rThreadVertexCount - 1]),
		                    offsetThisThread, verticesThisThread );

		size_t threadCount{0};

		for( std::future<void>& threadFutureHandle : threadFutureHandlesVector ) {
			threadFutureHandle.get();
			ctrIgnored   += rMeshData[threadCount].ctrIgnored;
			ctrProcessed += rMeshData[threadCount].ctrProcessed;
			++threadCount;
		}
	}

	// Timing stats of threads
	int procTime = static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel );
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	for( unsigned int threadCount = 0; threadCount < rThreadVertexCount; threadCount++ ) {
		//! \todo there are certainly more elegant ways to format time into useful/readable units.
		double timeSpent = rMeshData[threadCount].mWallTimeThread;
		std::string timeSpentUnit = "sec";
		if( timeSpent > ( 24.0 * 3600.0 ) ) {
			timeSpent = round( 100.0 * timeSpent / ( 24.0 * 3600.0 ) ) / 100.0;
			timeSpentUnit = "DAYS";
		} else if( timeSpent > 3600.0 ) {
			timeSpent = round( 100.0 * timeSpent / 3600.0 ) / 100.0;
			timeSpentUnit = "hours";
		}
		std::cout << "[GigaMesh] Thread " << threadCount
		          << " | Walltime: " << timeSpent << " " << timeSpentUnit
		          << " " << (100.0*rMeshData[threadCount].mWallTimeThread)/procTime << "%"
		          << "." << std::endl;
	}
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;

	// +++ Time for parallel processing
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	std::cout << "[GigaMesh] Time finished:      " << asctime( timeinfo ); // << endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	std::cout << "[GigaMesh] Vertices processed: " << ctrProcessed << std::endl;
	std::cout << "[GigaMesh] Vertices ignored:   " << ctrIgnored << std::endl;
	std::cout << "[GigaMesh] Parallel processing took " << procTime << " seconds." << std::endl;
	std::cout << "[GigaMesh]               ... equals " << static_cast<int>( ctrProcessed ) /
	                                                       ( procTime + 1 ) << " vertices/seconds." << std::endl; // add 1 to avoid division by zero for small meshes.
	// --- Time for parallel processing
} // END of compFeatureVectorsMain
