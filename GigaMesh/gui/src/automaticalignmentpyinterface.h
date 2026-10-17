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
#ifndef AUTOMATICALIGNMENTPYINTERFACE_H
#define AUTOMATICALIGNMENTPYINTERFACE_H
#include <QTextStream>
#include <string>
#include <GigaMesh/mesh/vertex.h>
#include <QTemporaryFile>
#include <QDialog>
using namespace std;

class AutomaticAlignmentPyInterface final : public QDialog
{
    Q_OBJECT
public:
    AutomaticAlignmentPyInterface(std::vector<Vertex*> *meshVertices);
    bool startPythonScript(Matrix4D *pcaTransformationMatrix);



private:
    bool exportVerticesAsCSV(std::vector<Vertex*> *meshVertices);
    void createExportVerticesTextStream(std::vector<Vertex*> *vertices,QTextStream &outputTextStream);
    bool readPCsFromCSV(QString importPath, Matrix4D *pcaTranfMatrix);
    QTemporaryFile mVertexCoordinatesTempFile;
    const QString mVertCSVPath = "./vertExport.csv";
};



#endif // AUTOMATICALIGNMENTPYINTERFACE_H
