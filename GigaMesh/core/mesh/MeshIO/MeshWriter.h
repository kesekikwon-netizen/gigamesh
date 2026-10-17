/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESHWRITER_H
#define MESHWRITER_H

#include <string>
#include <vector>
#include <GigaMesh/mesh/meshseedext.h>
#include <GigaMesh/mesh/MeshIO/ModelMetaData.h>

class MeshWriter
{
	public:
		MeshWriter();
		virtual ~MeshWriter() = default;
		virtual bool writeFile(const std::filesystem::path& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) = 0;

		void setModelMetaData(const ModelMetaData& metaData);
		ModelMetaData& getModelMetaDataRef();

		void setExportVertColor(bool exportVertColor);
		void setExportVertFeatureVector(bool exportVertFeatureVector);
		void setExportBinary(bool exportBinary);
		void setIsBigEndian(bool isBigEndian);
		void setExportVertNormal(bool exportVertNormal);
		void setExportVertLabel(bool exportVertLabel);
		void setExportPolyline(bool exportPolyline);
		void setExportTextureCoordinates(bool exportTextureCoordinates);
		void setExportVertFlags(bool exportVertFlags);

	private:
		ModelMetaData mModelMetaData;

	protected:
		bool mExportVertColor = false;
		bool mExportVertFeatureVector = false;
		bool mExportBinary = false;
		bool mIsBigEndian = false;
		bool mExportVertFlags = false;
		bool mExportVertNormal = false;
		bool mExportVertLabel = false;
		bool mExportPolyline = false;
		bool mExportTextureCoordinates = false;
};

#endif // MESHWRITER_H
