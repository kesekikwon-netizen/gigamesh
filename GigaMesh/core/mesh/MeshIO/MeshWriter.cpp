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

#include "MeshWriter.h"

MeshWriter::MeshWriter()
{

}

void MeshWriter::setModelMetaData(const ModelMetaData& metaData)
{
	mModelMetaData = metaData;
}

ModelMetaData& MeshWriter::getModelMetaDataRef()
{
	return mModelMetaData;
}

void MeshWriter::setExportVertColor(bool exportVertColor)
{
	mExportVertColor = exportVertColor;
}

void MeshWriter::setExportVertFeatureVector(bool exportVertFeatureVector)
{
	mExportVertFeatureVector = exportVertFeatureVector;
}

void MeshWriter::setExportBinary(bool exportBinary)
{
	mExportBinary = exportBinary;
}

void MeshWriter::setIsBigEndian(bool isBigEndian)
{
	mIsBigEndian = isBigEndian;
}

void MeshWriter::setExportVertNormal(bool exportVertNormal)
{
	mExportVertNormal = exportVertNormal;
}

void MeshWriter::setExportVertLabel(bool exportVertLabel)
{
	mExportVertLabel = exportVertLabel;
}

void MeshWriter::setExportPolyline(bool exportPolyline)
{
	mExportPolyline = exportPolyline;
}

void MeshWriter::setExportTextureCoordinates(bool exportTextureCoordinates)
{
	mExportTextureCoordinates = exportTextureCoordinates;
}

void MeshWriter::setExportVertFlags(bool exportVertFlags)
{
	mExportVertFlags = exportVertFlags;
}
