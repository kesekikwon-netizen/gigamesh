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

#ifndef DONGARCHPROJECT_H
#define DONGARCHPROJECT_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QVector>
#include <QDateTime>

/*!
 * @brief DongArchProject manages .dongarch3d project files.
 *
 * The .dongarch3d format is a ZIP archive containing:
 * - manifest.json: Project metadata and file registry
 * - meshes/: 3D mesh files (PLY format)
 * - sections/: Cross-section data (JSON)
 * - measurements/: Measurement data (CSV)
 * - annotations/: Annotation data (JSON)
 * - screenshots/: Preview images (PNG)
 *
 * File structure:
 * @code
 * project.dongarch3d (ZIP)
 * ├── manifest.json
 * ├── meshes/
 * │   └── artifact001.ply
 * ├── sections/
 * │   └── section001.json
 * ├── measurements/
 * │   └── measurements.csv
 * ├── annotations/
 * │   └── annotations.json
 * └── screenshots/
 *     └── view001.png
 * @endcode
 */
class DongArchProject
{
public:
    //! Mesh metadata structure
    struct MeshInfo {
        QString id;           //!< Unique identifier
        QString filename;     //!< Path within archive (e.g., "meshes/artifact001.ply")
        QString type;         //!< Mesh type (e.g., "lithic", "ceramic")
        QString name;         //!< Display name
        QJsonObject metadata; //!< Additional metadata
    };

    //! Section metadata structure
    struct SectionInfo {
        QString id;           //!< Unique identifier
        QString filename;     //!< Path within archive (e.g., "sections/section001.json")
        QString type;         //!< Section type (e.g., "top", "side", "profile")
        QVector<double> position; //!< 3D position [x, y, z]
        QJsonObject metadata; //!< Additional metadata
    };

    //! Measurement structure
    struct Measurement {
        QString type;         //!< Measurement type (e.g., "length", "angle", "area")
        QString label;        //!< Display label
        double value;         //!< Measured value
        QString unit;         //!< Unit of measurement (e.g., "mm", "degrees")
        QJsonObject metadata; //!< Additional metadata (points, coordinates, etc.)
    };

    DongArchProject();
    ~DongArchProject();

    //! @name Project Persistence
    //! @{

    /*!
     * @brief Save project to .dongarch3d file
     * @param filepath Output file path (must end with .dongarch3d)
     * @return true on success, false on failure
     */
    bool save(const QString& filepath);

    /*!
     * @brief Load project from .dongarch3d file
     * @param filepath Input file path
     * @return true on success, false on failure
     */
    bool load(const QString& filepath);

    //! @}

    //! @name Content Management
    //! @{

    /*!
     * @brief Add mesh to project
     * @param id Unique mesh identifier
     * @param plyPath Path to PLY mesh file (will be copied into archive)
     * @param type Mesh type (e.g., "lithic", "ceramic")
     * @param name Display name
     */
    void addMesh(const QString& id, const QString& plyPath,
                 const QString& type = "lithic", const QString& name = "");

    /*!
     * @brief Add section to project
     * @param id Unique section identifier
     * @param sectionData Section geometry and metadata (JSON)
     * @param type Section type (e.g., "top", "side", "profile")
     */
    void addSection(const QString& id, const QJsonObject& sectionData, const QString& type = "top");

    /*!
     * @brief Add measurement to project
     * @param measurement Measurement data
     */
    void addMeasurement(const Measurement& measurement);

    /*!
     * @brief Add screenshot to project
     * @param screenshotPath Path to PNG screenshot file
     * @param id Screenshot identifier (auto-generated if empty)
     */
    void addScreenshot(const QString& screenshotPath, const QString& id = "");

    /*!
     * @brief Add annotation to project
     * @param annotationData Annotation data (JSON)
     */
    void addAnnotation(const QJsonObject& annotationData);

    //! @}

    //! @name Data Retrieval
    //! @{

    /*!
     * @brief Get list of all meshes
     * @return Vector of mesh metadata
     */
    QVector<MeshInfo> getMeshes() const;

    /*!
     * @brief Get list of all sections
     * @return Vector of section metadata
     */
    QVector<SectionInfo> getSections() const;

    /*!
     * @brief Get list of all measurements
     * @return Vector of measurements
     */
    QVector<Measurement> getMeasurements() const;

    /*!
     * @brief Get file path for a mesh
     * @param id Mesh identifier
     * @return Temporary file path (valid until project is closed)
     */
    QString getMeshFilePath(const QString& id) const;

    /*!
     * @brief Get section data
     * @param id Section identifier
     * @return Section JSON data
     */
    QJsonObject getSectionData(const QString& id) const;

    //! @}

    //! @name Project Metadata
    //! @{

    /*!
     * @brief Get project version
     * @return Version string (e.g., "1.0.0")
     */
    QString getVersion() const;

    /*!
     * @brief Get project creation timestamp
     * @return Creation date/time
     */
    QDateTime getCreated() const;

    /*!
     * @brief Get project modification timestamp
     * @return Modification date/time
     */
    QDateTime getModified() const;

    /*!
     * @brief Get full manifest as JSON
     * @return Manifest object
     */
    QJsonObject getManifest() const { return mManifest; }

    /*!
     * @brief Clear all project data
     */
    void clear();

    //! @}

    //! @name Error Handling
    //! @{

    /*!
     * @brief Get last error message
     * @return Error description
     */
    QString getLastError() const { return mLastError; }

    //! @}

private:
    //! @name Internal Helpers
    //! @{

    /*!
     * @brief Create ZIP archive from temporary directory
     * @param tempDir Temporary directory containing files
     * @param zipPath Output ZIP file path
     * @return true on success
     */
    bool createZipArchive(const QString& tempDir, const QString& zipPath);

    /*!
     * @brief Extract ZIP archive to temporary directory
     * @param zipPath Input ZIP file path
     * @param tempDir Output temporary directory
     * @return true on success
     */
    bool extractZipArchive(const QString& zipPath, const QString& tempDir);

    /*!
     * @brief Write manifest.json to file
     * @param filepath Output file path
     * @return true on success
     */
    bool writeManifest(const QString& filepath);

    /*!
     * @brief Read manifest.json from file
     * @param filepath Input file path
     * @return true on success
     */
    bool readManifest(const QString& filepath);

    /*!
     * @brief Write measurements to CSV
     * @param filepath Output CSV file path
     * @return true on success
     */
    bool writeMeasurements(const QString& filepath);

    /*!
     * @brief Read measurements from CSV
     * @param filepath Input CSV file path
     * @return true on success
     */
    bool readMeasurements(const QString& filepath);

    /*!
     * @brief Copy directory recursively
     * @param srcPath Source directory
     * @param dstPath Destination directory
     * @return true on success
     */
    bool copyDirectory(const QString& srcPath, const QString& dstPath);

    //! @}

private:
    QJsonObject mManifest;              //!< Project manifest (metadata)
    QVector<MeshInfo> mMeshes;          //!< Mesh metadata
    QVector<SectionInfo> mSections;     //!< Section metadata
    QVector<Measurement> mMeasurements; //!< Measurement data
    QJsonArray mAnnotations;            //!< Annotation data
    QVector<QString> mScreenshots;      //!< Screenshot identifiers

    QMap<QString, QString> mFiles;      //!< Virtual path → temporary file mapping
    QString mTempDir;                   //!< Temporary extraction directory
    QString mLastError;                 //!< Last error message
};

#endif // DONGARCHPROJECT_H
