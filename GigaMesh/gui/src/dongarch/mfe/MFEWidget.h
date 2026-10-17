/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 6: Mesh File Explorer Widget
 *
 * MFE Widget:
 * - Qt TreeView 기반 파일 목록
 * - 파일 정보 표시 패널
 * - 썸네일 미리보기
 * - 파일 열기/복사/삭제
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_MFE_WIDGET_H
#define DONGARCH_MFE_WIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QString>
#include <QFutureWatcher>
#include <memory>

#include "MFEManager.h"

namespace Ui {
class MFEWidget;
}

namespace DongArch {
namespace MFE {

//! \class MFEWidget
//! \brief Mesh File Explorer UI 위젯
//!
//! UI 구성:
//! - 상단: 디렉토리 선택 버튼
//! - 좌측: TreeView (파일 목록)
//! - 우측: 파일 정보 패널 + 썸네일
//! - 하단: 상태 바 (파일 개수, 총 용량)
//!
//! 사용법:
//! ```cpp
//! MFEWidget* mfeWidget = new MFEWidget(this);
//! mfeWidget->setDirectory("/path/to/meshes");
//!
//! connect(mfeWidget, &MFEWidget::fileSelected, this, [](const QString& path) {
//!     qDebug() << "Selected:" << path;
//! });
//!
//! connect(mfeWidget, &MFEWidget::fileDoubleClicked, this, [](const QString& path) {
//!     // Open mesh file
//! });
//! ```
class MFEWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MFEWidget(QWidget *parent = nullptr);
    ~MFEWidget();

    //! 디렉토리 설정 및 스캔
    //! \param directoryPath 디렉토리 경로
    //! \param recursive 재귀적 스캔 여부
    void setDirectory(const QString& directoryPath, bool recursive = true);

    //! 현재 선택된 파일 경로 조회
    //! \return 파일 경로 (선택 없으면 빈 문자열)
    QString getSelectedFilePath() const;

    //! 파일 목록 새로고침
    void refresh();

signals:
    //! 파일이 선택되었을 때
    //! \param filePath 파일 경로
    void fileSelected(const QString& filePath);

    //! 파일이 더블클릭되었을 때 (파일 열기)
    //! \param filePath 파일 경로
    void fileDoubleClicked(const QString& filePath);

    //! 파일 목록이 변경되었을 때
    //! \param fileCount 파일 개수
    void fileListChanged(int fileCount);

private slots:
    //! "디렉토리 선택" 버튼 클릭
    void onBrowseButtonClicked();

    //! "새로고침" 버튼 클릭
    void onRefreshButtonClicked();

    //! TreeView 선택 변경
    void onTreeViewSelectionChanged();

    //! TreeView 더블클릭
    void onTreeViewDoubleClicked(const QModelIndex& index);

    //! "파일 열기" 버튼 클릭
    void onOpenFileButtonClicked();

private:
    //! UI 초기화
    void setupUi();

    //! TreeView 모델 초기화
    void setupTreeViewModel();

    //! TreeView 업데이트 (파일 목록 표시)
    void updateTreeView();

    //! 파일 정보 패널 업데이트
    //! \param filePath 파일 경로
    void updateFileInfoPanel(const QString& filePath);

    //! 상태 바 업데이트
    void updateStatusBar();

    //! 썸네일 업데이트
    //! \param thumbnail 썸네일 이미지
    void updateThumbnail(const QImage& thumbnail);

    //! 썸네일 비동기 로딩 시작
    //! \param filePath 파일 경로
    void loadThumbnailAsync(const QString& filePath);

    //! 썸네일 로딩 완료 (비동기)
    void onThumbnailReady();

    Ui::MFEWidget *ui;                          //!< Qt Designer UI
    std::unique_ptr<MFEManager> mManager;       //!< MFE 관리자
    QStandardItemModel* mTreeModel;             //!< TreeView 모델

    QString mCurrentDirectory;                  //!< 현재 디렉토리
    QString mSelectedFilePath;                  //!< 현재 선택된 파일

    // Async thumbnail loading
    QFutureWatcher<QImage>* mThumbnailWatcher;  //!< 썸네일 비동기 로딩 감시자
    QString mThumbnailLoadingPath;              //!< 현재 로딩 중인 썸네일 파일 경로
};

} // namespace MFE
} // namespace DongArch

#endif // DONGARCH_MFE_WIDGET_H
