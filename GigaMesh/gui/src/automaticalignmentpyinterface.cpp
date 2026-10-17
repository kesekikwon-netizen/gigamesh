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
#include "automaticalignmentpyinterface.h"
#include <QString>
#include <QFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextStream>
#include <iostream>
#include <QSettings>
//#include <MeshQtCSVImportExport.h>

AutomaticAlignmentPyInterface::AutomaticAlignmentPyInterface(std::vector<Vertex*> *meshVertices)
{
    //export the vertices for the python script
    exportVerticesAsCSV(meshVertices);

}

bool AutomaticAlignmentPyInterface::startPythonScript(Matrix4D *pcaTransformationMatrix)
{

    QTemporaryFile PCAScriptTempFile;

    PCAScriptTempFile.setAutoRemove(true);

    if(PCAScriptTempFile.open())
    {
        std::cout << "[Mesh::" << __FUNCTION__ << "] "
                << "Opened PCA script temp file "
                << PCAScriptTempFile.fileName().toStdString()
                << std::endl;
    }

    else
    {
        std::clog << "[Mesh::" << __FUNCTION__ << "] "
                    << "Error: Could not open temp file"
                    << std::endl;
        return false;
    }

    const QString PCAScriptTempFileName = PCAScriptTempFile.fileName();

    QFile PCAScriptFile(":/pythonscripts/PCA_vertices.py");

    if(PCAScriptFile.open(QFile::ReadOnly))
    {
        std::cout << "[Mesh::" << __FUNCTION__ << "] "
                << "Opened PCA script file "
                << PCAScriptFile.fileName().toStdString()
                << std::endl;
    }

    else
    {
        std::clog << "[Mesh::" << __FUNCTION__ << "] "
                    << "Error: Could not open temp file"
                    << std::endl;
        return false;
    }

    PCAScriptTempFile.write(PCAScriptFile.readAll());

    PCAScriptTempFile.flush();
    PCAScriptTempFile.close();

    PCAScriptFile.close();

    const QString vertexCsvFilePath = mVertexCoordinatesTempFile.fileName();
    const QStringList PCAScriptArguments({PCAScriptTempFileName,
                                                vertexCsvFilePath});

    QProcess* runPCAScriptProcess = new QProcess(this);

    runPCAScriptProcess->setProcessChannelMode( QProcess::ProcessChannelMode::ForwardedChannels );

    QSettings settings;
    QString pythonPath = settings.value("Python3_Path", "").toString();

    if(pythonPath.length() == 0)
        pythonPath = QString("python3");

    runPCAScriptProcess->start(pythonPath,
                                    PCAScriptArguments);

    runPCAScriptProcess->waitForFinished();

    //if error != 0 --> python script doesn't run properly
    int error = runPCAScriptProcess->exitCode();
    if (error != 0){
        return false;
    }
    //I use the input file of the python script because I didn't find a way to get the Terminal output of the process
    // and the input file is tmp file, that means that the file is deleted after a short time by the OS
    if(!readPCsFromCSV(vertexCsvFilePath, pcaTransformationMatrix)){
        return false;
    }
    delete runPCAScriptProcess;
    return true;

}

//own export is written because only a temporary file is needed
bool AutomaticAlignmentPyInterface::exportVerticesAsCSV(std::vector<Vertex *> *vertices)
{


    mVertexCoordinatesTempFile.setAutoRemove(true);

    if(mVertexCoordinatesTempFile.open())
    {
        std::cout << "[Python PCA Interface::" << __FUNCTION__ << "] "
                << "Opened temp file "
                << mVertexCoordinatesTempFile.fileName().toStdString()
                << std::endl;
    }

    else
    {
        std::clog << "[Python PCA Interface::" << __FUNCTION__ << "] "
                    << "Error: Could not open temp file"
                    << std::endl;
        return false;
    }

    mVertexCoordinatesTempFile.setTextModeEnabled(true);

    QTextStream vertexCoordinatesOutputTextStream(
                                    &mVertexCoordinatesTempFile);

    createExportVerticesTextStream(vertices,vertexCoordinatesOutputTextStream);


    mVertexCoordinatesTempFile.flush();
    mVertexCoordinatesTempFile.close();

    /**
    if(QFile::exists(mVertCSVPath))
    {
        if(QFile::remove(mVertCSVPath))
        {
            std::cout << "[Python PCA Interface::" << __FUNCTION__ << "] "
                        << "Removed existing file "
                        << mVertCSVPath.toStdString()
                        << std::endl;
        }

        else
        {
            std::clog << "[Python PCA Interface::" << __FUNCTION__ << "] "
                        << "Error: Could not delete existing file "
                        << mVertCSVPath.toStdString()
                        << std::endl;
            return false;
        }
    }

    if(mVertexCoordinatesTempFile.copy(mVertCSVPath))
    {
        std::cout << "[Python PCA Interface::" << __FUNCTION__ << "] "
                    << "Copied data from temp file "
                    << mVertexCoordinatesTempFile.fileName().toStdString()
                    << " to output file "
                    << mVertCSVPath.toStdString()
                    << std::endl;
    }

    else
    {
        std::clog << "[Python PCA Interface::" << __FUNCTION__ << "] "
                    << "Error: Could not copy data from temp file "
                    << mVertexCoordinatesTempFile.fileName().toStdString()
                    << " to output file "
                    << mVertCSVPath.toStdString()
                    << std::endl;
        return false;
    }
    **/
    return true;
}

void AutomaticAlignmentPyInterface::createExportVerticesTextStream(std::vector<Vertex *> *vertices,QTextStream &outputTextStream)
{
    std::pair<int, std::array<double, 3>> indexAndPositionVectorPair;

    for(const Vertex* vertex : *vertices)
    {
        vertex->getIndexAndCoordinates(indexAndPositionVectorPair);

        outputTextStream << indexAndPositionVectorPair.first << ','
                                            << indexAndPositionVectorPair.second.at(0) << ','
                                            << indexAndPositionVectorPair.second.at(1)<< ','
                                            << indexAndPositionVectorPair.second.at(2) << '\n';
    }
}

//!read the principal components/result of the python script
//! the whole transformation matrix based on the PCs
//! input file = CSV
bool AutomaticAlignmentPyInterface::readPCsFromCSV(QString importPath, Matrix4D *pcaTranfMatrix)
{
    if(importPath.isEmpty() ||
                    importPath.isNull())
    {
        return false;
    }
    QFile principalComponentsInputFile(importPath);

    if(principalComponentsInputFile.open(QFile::ReadOnly | QFile::Text))
    {
        std::cout << "[Python PCA Interface::" << __FUNCTION__ << "] "
                << "Opened results file "
                << principalComponentsInputFile.fileName().toStdString()
                << std::endl;
    }

    else
    {
        std::clog << "[Python PCA Interface::" << __FUNCTION__ << "] "
                    << "Error: Could not open file "
                    << principalComponentsInputFile.fileName().toStdString()
                    << std::endl;
        return false;
    }

    //read data from CSV
    //-------------------------------------------------
    const QRegExp matchDelimitersRegex(",");
    std::vector<std::pair<int, std::array<double, 4>>> indexPrincipalComponentsMatrix;
    QStringList matrixRowStringList;
    QByteArray lineBuffer;

    while(true)
    {
        lineBuffer = principalComponentsInputFile.readLine();

        if(lineBuffer.size() <= 1)
        {
            break;
        }

        // Remove newline
        lineBuffer.remove(lineBuffer.size() - 1, 1);
        matrixRowStringList = QString(lineBuffer).split(
                                                matchDelimitersRegex);

        // Remove all empty strings
        matrixRowStringList.removeAll(QString(""));
        //assign all seperated values to pair and to the result vector
        if(matrixRowStringList.size() == 5)
        {
            std::pair<int, std::array<double, 4>> indexPrincipalComponentsValuePair;

            indexPrincipalComponentsValuePair.first =
                                matrixRowStringList.at(0).toInt();

            indexPrincipalComponentsValuePair.second.at(0) =
                                matrixRowStringList.at(1).toDouble();

            indexPrincipalComponentsValuePair.second.at(1) =
                                matrixRowStringList.at(2).toDouble();

            indexPrincipalComponentsValuePair.second.at(2) =
                                matrixRowStringList.at(3).toDouble();

            indexPrincipalComponentsValuePair.second.at(3) =
                                matrixRowStringList.at(4).toDouble();

            indexPrincipalComponentsMatrix.push_back(indexPrincipalComponentsValuePair);

        }
    }

    //convert the imported values to Matrix
    //transposed to the python matrix
    std::vector<double> transfMatrixVec;
    for(std::pair<int, std::array<double, 4>> vectorEntry: indexPrincipalComponentsMatrix ){
       transfMatrixVec.insert(transfMatrixVec.begin()+vectorEntry.first,vectorEntry.second.at(0));
       transfMatrixVec.insert(transfMatrixVec.begin()+2*vectorEntry.first+1,vectorEntry.second.at(1));
       transfMatrixVec.insert(transfMatrixVec.begin()+3*vectorEntry.first+2,vectorEntry.second.at(2));
       transfMatrixVec.insert(transfMatrixVec.begin()+4*vectorEntry.first+3,vectorEntry.second.at(3));
    }
    Matrix4D newTransformationMatrix(transfMatrixVec);
    pcaTranfMatrix->set(newTransformationMatrix);
    return true;
}
