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

#ifndef EXTERNALPROGRAMSDIALOG_H
#define EXTERNALPROGRAMSDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
	class ExternalProgramsDialog;
}

class ExternalProgramsDialog : public QDialog
{
		Q_OBJECT

	public:
		explicit ExternalProgramsDialog(QWidget *parent = nullptr);
		~ExternalProgramsDialog() override;

		[[nodiscard]] QString pdfLatexPath() const;
		void setPdfLatexPath(const QString& pdfLatexPath);

		[[nodiscard]] QString pdfViewerPath() const;
		void setPdfViewerPath(const QString& pdfViewerPath);

		[[nodiscard]] QString inkscapePath() const;
		void setInkscapePath(const QString& inkscapePath);

		[[nodiscard]] QString pythonPath() const;
		void setPythonPath(const QString& pythonPath);

	private:
		Ui::ExternalProgramsDialog *ui;

		QString mPdfLatexPath;
		QString mPdfViewerPath;
		QString mInkscapePath;
		QString mPythonPath;

	private slots:
		void inkscapePathChanged(const QString& string);
		void pdfLatexPathChanged(const QString& string);
		void pdfViewerPathChanged(const QString& string);
		void pythonPathChanged(const QString& string);

		void inkscapePathChoose();
		void pdfLatexPathChoose();
		void pdfViewerPathChoose();
		void pythonPathChoose();

		void defaultPressed();

		// QWidget interface
	protected:
		virtual void changeEvent(QEvent* event) override;
};

#endif // EXTERNALPROGRAMSDIALOG_H
