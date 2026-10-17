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

#include "DialogFindTextures.h"
#include "ui_DialogFindTextures.h"

#include <QGLWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QImageReader>

DialogFindTextures::DialogFindTextures(const QStringList& missingTextures, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFindTextures)
{
	ui->setupUi(this);

	for(auto textureIt = missingTextures.begin(); textureIt != missingTextures.end(); ++textureIt)
	{
		auto selectWidget = new FileSelectWidget(*textureIt);
		ui->fileSelectLayout->addWidget(selectWidget);
		mSelectWidgets.push_back(selectWidget);
	}
}


DialogFindTextures::~DialogFindTextures()
{
	delete ui;
}

QStringList DialogFindTextures::getFileNames()
{
	QStringList fileNames;

	for(auto it = mSelectWidgets.begin(); it != mSelectWidgets.end(); ++it)
	{
		fileNames.push_back((*it)->getFileName());
	}

	return fileNames;
}

QString FileSelectWidget::sSupportedImageFormats = QString();

FileSelectWidget::FileSelectWidget(const QString& fileName, QWidget* parent) : QWidget(parent), mFile(fileName)
{
	auto layout = new QHBoxLayout;
	layout->setMargin(0);

	this->setLayout(layout);

	QFileInfo fileInfo(fileName);

	mFileNameLabel = new QLabel(fileInfo.fileName());
	layout->addWidget(mFileNameLabel);

	auto dialogButton = new QPushButton(QString("..."));
	layout->addWidget(dialogButton);

	connect(dialogButton, &QPushButton::clicked, this, &FileSelectWidget::selectFile);
}

QString FileSelectWidget::getFileName() const {
	return mFile;
}

void FileSelectWidget::selectFile()
{
	if(sSupportedImageFormats.isNull())
	{
		auto imgFiles = QImageReader::supportedImageFormats();

		for(const auto& imgType : imgFiles)
		{
			sSupportedImageFormats += QString("*." + imgType + " ");
		}
	}

	auto fileName = QFileDialog::getOpenFileName(this, tr("select Texture for") + " " + mFileNameLabel->text(),
	                                             "",tr("Images") + "(" + sSupportedImageFormats + ")");

	if(!fileName.isNull())
	{
		QFileInfo fileInfo(fileName);
		mFileNameLabel->setText(fileInfo.fileName());
		mFile = fileName;
	}
}
