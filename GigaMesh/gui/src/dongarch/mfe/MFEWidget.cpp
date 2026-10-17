/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 6: Mesh File Explorer Widget Implementation
 */

#include "MFEWidget.h"
#include "ui_MFEWidget.h"

#include <QFileDialog>
#include <QStandardItem>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

namespace DongArch {
namespace MFE {

//==============================================================================
// Constructor / Destructor
//==============================================================================

MFEWidget::MFEWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MFEWidget)
    , mManager(std::make_unique<MFEManager>())
    , mTreeModel(nullptr)
    , mThumbnailWatcher(new QFutureWatcher<QImage>(this))
{
    ui->setupUi(this);
    setupUi();
    setupTreeViewModel();

    // Connect thumbnail watcher
    connect(mThumbnailWatcher, &QFutureWatcher<QImage>::finished,
            this, &MFEWidget::onThumbnailReady);
}

MFEWidget::~MFEWidget()
{
    // Cancel any pending thumbnail generation
    if (mThumbnailWatcher->isRunning()) {
        mThumbnailWatcher->cancel();
        mThumbnailWatcher->waitForFinished();
    }

    delete ui;
    delete mTreeModel;
}

//==============================================================================
// UI Setup
//==============================================================================

void MFEWidget::setupUi()
{
    // Connect buttons
    connect(ui->browseButton, &QPushButton::clicked, this, &MFEWidget::onBrowseButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &MFEWidget::onRefreshButtonClicked);
    connect(ui->openFileButton, &QPushButton::clicked, this, &MFEWidget::onOpenFileButtonClicked);

    // Connect tree view
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MFEWidget::onTreeViewSelectionChanged);
    connect(ui->treeView, &QTreeView::doubleClicked,
            this, &MFEWidget::onTreeViewDoubleClicked);

    // Initial state
    ui->fileInfoGroupBox->setEnabled(false);
    ui->openFileButton->setEnabled(false);
}

void MFEWidget::setupTreeViewModel()
{
    mTreeModel = new QStandardItemModel(this);

    // Header
    mTreeModel->setHorizontalHeaderLabels({"파일 이름", "형식", "버텍스", "면", "크기"});

    ui->treeView->setModel(mTreeModel);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Column widths
    ui->treeView->setColumnWidth(0, 250);  // File name
    ui->treeView->setColumnWidth(1, 60);   // Format
    ui->treeView->setColumnWidth(2, 80);   // Vertices
    ui->treeView->setColumnWidth(3, 80);   // Faces
    ui->treeView->setColumnWidth(4, 100);  // Size
}

//==============================================================================
// Directory Management
//==============================================================================

void MFEWidget::setDirectory(const QString& directoryPath, bool recursive)
{
    mCurrentDirectory = directoryPath;

    ui->directoryLineEdit->setText(directoryPath);

    int fileCount = mManager->scanDirectory(directoryPath, recursive);

    updateTreeView();
    updateStatusBar();

    emit fileListChanged(fileCount);

    qDebug() << "MFEWidget: Loaded" << fileCount << "files from" << directoryPath;
}

void MFEWidget::refresh()
{
    if (!mCurrentDirectory.isEmpty()) {
        setDirectory(mCurrentDirectory, true);
    }
}

//==============================================================================
// TreeView Update
//==============================================================================

void MFEWidget::updateTreeView()
{
    mTreeModel->removeRows(0, mTreeModel->rowCount());

    const auto& fileList = mManager->getFileList();

    for (const auto& fileInfo : fileList) {
        QList<QStandardItem*> row;

        // File name
        QStandardItem* nameItem = new QStandardItem(fileInfo.fileName);
        nameItem->setData(fileInfo.filePath, Qt::UserRole);  // Store full path
        row.append(nameItem);

        // Format
        row.append(new QStandardItem(fileInfo.fileExtension.toUpper()));

        // Vertex count
        row.append(new QStandardItem(QString::number(fileInfo.vertexCount)));

        // Face count
        row.append(new QStandardItem(QString::number(fileInfo.faceCount)));

        // File size
        double sizeMB = fileInfo.fileSize / (1024.0 * 1024.0);
        row.append(new QStandardItem(QString("%1 MB").arg(sizeMB, 0, 'f', 2)));

        mTreeModel->appendRow(row);
    }

    ui->treeView->resizeColumnToContents(0);
}

//==============================================================================
// File Info Panel Update
//==============================================================================

void MFEWidget::updateFileInfoPanel(const QString& filePath)
{
    auto fileInfoOpt = mManager->getFileInfo(filePath);

    if (!fileInfoOpt) {
        ui->fileInfoGroupBox->setEnabled(false);
        return;
    }

    const MeshFileInfo& fileInfo = *fileInfoOpt;

    ui->fileInfoGroupBox->setEnabled(true);

    // Update labels
    ui->fileNameLabel->setText(fileInfo.fileName);
    ui->filePathLabel->setText(fileInfo.filePath);
    ui->formatLabel->setText(fileInfo.fileExtension.toUpper());
    ui->vertexCountLabel->setText(QString::number(fileInfo.vertexCount));
    ui->faceCountLabel->setText(QString::number(fileInfo.faceCount));

    double sizeMB = fileInfo.fileSize / (1024.0 * 1024.0);
    ui->fileSizeLabel->setText(QString("%1 MB").arg(sizeMB, 0, 'f', 2));

    // Bounding box
    ui->boundingBoxLabel->setText(QString("Min: (%1, %2, %3)\nMax: (%4, %5, %6)")
        .arg(fileInfo.boundingBoxMin.x(), 0, 'f', 2)
        .arg(fileInfo.boundingBoxMin.y(), 0, 'f', 2)
        .arg(fileInfo.boundingBoxMin.z(), 0, 'f', 2)
        .arg(fileInfo.boundingBoxMax.x(), 0, 'f', 2)
        .arg(fileInfo.boundingBoxMax.y(), 0, 'f', 2)
        .arg(fileInfo.boundingBoxMax.z(), 0, 'f', 2));

    // Thumbnail - Load asynchronously
    loadThumbnailAsync(filePath);
}

void MFEWidget::updateThumbnail(const QImage& thumbnail)
{
    QPixmap pixmap = QPixmap::fromImage(thumbnail);
    ui->thumbnailLabel->setPixmap(pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MFEWidget::updateStatusBar()
{
    const auto& fileList = mManager->getFileList();

    qint64 totalSize = 0;
    for (const auto& fileInfo : fileList) {
        totalSize += fileInfo.fileSize;
    }

    double totalSizeMB = totalSize / (1024.0 * 1024.0);

    ui->statusLabel->setText(QString("%1개 파일 | 총 %2 MB")
        .arg(fileList.size())
        .arg(totalSizeMB, 0, 'f', 2));
}

//==============================================================================
// Slots
//==============================================================================

void MFEWidget::onBrowseButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "메시 파일 디렉토리 선택",
        mCurrentDirectory,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        setDirectory(dir);
    }
}

void MFEWidget::onRefreshButtonClicked()
{
    refresh();
}

void MFEWidget::onTreeViewSelectionChanged()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        mSelectedFilePath.clear();
        ui->fileInfoGroupBox->setEnabled(false);
        ui->openFileButton->setEnabled(false);
        return;
    }

    QModelIndex index = selectedIndexes.first();
    QString filePath = mTreeModel->item(index.row(), 0)->data(Qt::UserRole).toString();

    mSelectedFilePath = filePath;

    updateFileInfoPanel(filePath);
    ui->openFileButton->setEnabled(true);

    emit fileSelected(filePath);
}

void MFEWidget::onTreeViewDoubleClicked(const QModelIndex& index)
{
    QString filePath = mTreeModel->item(index.row(), 0)->data(Qt::UserRole).toString();

    emit fileDoubleClicked(filePath);
}

void MFEWidget::onOpenFileButtonClicked()
{
    if (!mSelectedFilePath.isEmpty()) {
        emit fileDoubleClicked(mSelectedFilePath);
    }
}

//==============================================================================
// Async Thumbnail Loading
//==============================================================================

void MFEWidget::loadThumbnailAsync(const QString& filePath)
{
    // Cancel any pending thumbnail generation
    if (mThumbnailWatcher->isRunning()) {
        mThumbnailWatcher->cancel();
        mThumbnailWatcher->waitForFinished();
    }

    // Show loading placeholder
    QImage loadingImage(128, 128, QImage::Format_RGB888);
    loadingImage.fill(QColor(64, 64, 64));
    QPainter painter(&loadingImage);
    painter.setPen(QColor(192, 192, 192));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(loadingImage.rect(), Qt::AlignCenter, "Loading...");
    updateThumbnail(loadingImage);

    // Store path for later
    mThumbnailLoadingPath = filePath;

    // Start async thumbnail generation
    QFuture<QImage> future = QtConcurrent::run([filePath]() {
        return MFEManager::generateThumbnail(filePath, 128);
    });

    mThumbnailWatcher->setFuture(future);
}

void MFEWidget::onThumbnailReady()
{
    // Check if the thumbnail is still relevant (user might have selected different file)
    if (mThumbnailLoadingPath != mSelectedFilePath) {
        qDebug() << "Thumbnail ready but selection changed, discarding";
        return;
    }

    QImage thumbnail = mThumbnailWatcher->result();
    updateThumbnail(thumbnail);

    qDebug() << "Thumbnail loaded for:" << mThumbnailLoadingPath;
}

//==============================================================================
// Getters
//==============================================================================

QString MFEWidget::getSelectedFilePath() const
{
    return mSelectedFilePath;
}

} // namespace MFE
} // namespace DongArch
