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

#include "ExternalProgramsDialog.h"
#include "ui_ExternalProgramsDialog.h"
#include <QFileDialog>
#include <QStyle>

ExternalProgramsDialog::ExternalProgramsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ExternalProgramsDialog),
	mPdfLatexPath(""),
	mPdfViewerPath(""),
	mInkscapePath("")
{
	ui->setupUi(this);

	connect(ui->inkscape_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(inkscapePathChanged(const QString&)));
	connect(ui->pdf_latex_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pdfLatexPathChanged(const QString&)));
	connect(ui->pdf_viewer_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pdfViewerPathChanged(const QString&)));
	connect(ui->python_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pythonPathChanged(const QString&)));

	connect(ui->inkscape_pushButton, SIGNAL(pressed()), this, SLOT(inkscapePathChoose()));
	connect(ui->pdf_latex_pushButton, SIGNAL(pressed()), this, SLOT(pdfLatexPathChoose()));
	connect(ui->pdf_viewer_pushButton, SIGNAL(pressed()), this, SLOT(pdfViewerPathChoose()));
	connect(ui->python_pushButton, SIGNAL(pressed()), this, SLOT(pythonPathChoose()));

	connect(ui->defaultButton, SIGNAL(pressed()), this, SLOT(defaultPressed()));
}

ExternalProgramsDialog::~ExternalProgramsDialog()
{
	delete ui;
}

QString ExternalProgramsDialog::pdfLatexPath() const
{
	return mPdfLatexPath;
}

void ExternalProgramsDialog::setPdfLatexPath(const QString& pdfLatexPath)
{
	mPdfLatexPath = pdfLatexPath;
	ui->pdf_latex_lineEdit->setText(pdfLatexPath);
}

QString ExternalProgramsDialog::pdfViewerPath() const
{
	return mPdfViewerPath;
}

void ExternalProgramsDialog::setPdfViewerPath(const QString& pdfViewerPath)
{
	mPdfViewerPath = pdfViewerPath;
	ui->pdf_viewer_lineEdit->setText(pdfViewerPath);
}

QString ExternalProgramsDialog::inkscapePath() const
{
	return mInkscapePath;
}

void ExternalProgramsDialog::setInkscapePath(const QString& inkscapePath)
{
	mInkscapePath = inkscapePath;
	ui->inkscape_lineEdit->setText(inkscapePath);
}

QString ExternalProgramsDialog::pythonPath() const
{
	return mPythonPath;
}

void ExternalProgramsDialog::setPythonPath(const QString& pythonPath)
{
	mPythonPath = pythonPath;
	ui->python_lineEdit->setText(pythonPath);
}

void ExternalProgramsDialog::inkscapePathChanged(const QString& string)
{
	mInkscapePath = string;
}

void ExternalProgramsDialog::pdfLatexPathChanged(const QString& string)
{
	mPdfLatexPath = string;
}

void ExternalProgramsDialog::pdfViewerPathChanged(const QString& string)
{
	mPdfViewerPath = string;
}

void ExternalProgramsDialog::pythonPathChanged(const QString& string)
{
	mPythonPath = string;
}

void choosePath(const QString& header, QLineEdit* lineEditElement, QWidget* parent)
{
	auto fileName = QFileDialog::getOpenFileName(parent, header);

	if(!fileName.isNull())
	{
		#ifdef WIN32
			fileName = "\"" + fileName + "\"";
		#endif
		lineEditElement->setText(fileName);
	}
}

void ExternalProgramsDialog::inkscapePathChoose()
{
	choosePath(tr("Select Inkscape executable"), ui->inkscape_lineEdit, this);
}

void ExternalProgramsDialog::pdfLatexPathChoose()
{
	choosePath(tr("Select pdflatex executable"), ui->pdf_latex_lineEdit, this);
}

void ExternalProgramsDialog::pdfViewerPathChoose()
{
	choosePath(tr("Select a program to view pdf files"), ui->pdf_viewer_lineEdit, this);
}

void ExternalProgramsDialog::pythonPathChoose()
{
	choosePath(tr("Select python3 executable"), ui->python_lineEdit, this);
}

void ExternalProgramsDialog::defaultPressed()
{
}



void ExternalProgramsDialog::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
