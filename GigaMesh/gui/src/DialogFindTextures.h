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

#ifndef DIALOGFINDTEXTURES_H
#define DIALOGFINDTEXTURES_H

#include <QDialog>
#include <QStringList>
#include <vector>

class QLabel;
class FileSelectWidget;

namespace Ui {
class DialogFindTextures;
}

class DialogFindTextures : public QDialog
{
		Q_OBJECT

	public:
		explicit DialogFindTextures(const QStringList& missingTextures, QWidget *parent = nullptr);
		~DialogFindTextures();
		QStringList getFileNames();

	private:
		Ui::DialogFindTextures *ui;
		QList<FileSelectWidget*> mSelectWidgets;
};


class FileSelectWidget : public QWidget {
		Q_OBJECT

	public:
		FileSelectWidget(const QString& fileName, QWidget* parent = nullptr);

		~FileSelectWidget() = default;

		QString getFileName() const;

	private slots:
		void selectFile();

	private:
		QLabel* mFileNameLabel = nullptr;
		QString mFile;

		static QString sSupportedImageFormats;
};

#endif // DIALOGFINDTEXTURES_H
