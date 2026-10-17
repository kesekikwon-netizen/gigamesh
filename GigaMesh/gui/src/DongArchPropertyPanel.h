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

#ifndef DONGARCHPROPERTYPANEL_H
#define DONGARCHPROPERTYPANEL_H

#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QWidget>
#include <QVariant>
#include <QString>

//!
//! \brief DongArch3D 전용 속성 패널
//!
//! 선택된 객체의 속성을 실시간으로 표시하는 패널입니다.
//! 고고학 실측 작업에 특화된 정보를 제공합니다.
//!
//! 표시 정보:
//! - 좌표 정보: X, Y, Z 위치 (mm 단위)
//! - 크기 정보: 길이, 폭, 높이, 면적, 부피
//! - 메타데이터: 유물명, 유적명, 발굴일자 등
//!
class DongArchPropertyPanel : public QDockWidget
{
	Q_OBJECT

public:
	explicit DongArchPropertyPanel(QWidget* parent = nullptr);
	~DongArchPropertyPanel();

	//! \brief 속성 값 업데이트
	//! \param key 속성 키 (예: "위치 X", "면적")
	//! \param value 속성 값 (QVariant로 다양한 타입 지원)
	void updateProperty(const QString& key, const QVariant& value);

	//! \brief 모든 속성 초기화
	void clearProperties();

	//! \brief 샘플 데이터로 패널 채우기 (테스트용)
	void loadSampleData();

private:
	void setupUI();
	void createCoordinateGroup();
	void createSizeGroup();
	void createMetadataGroup();

	// Helper methods
	QString formatDouble(double value, int decimals = 2) const;
	QLabel* createValueLabel();

	// Main layout
	QScrollArea* mScrollArea;
	QWidget* mContentWidget;
	QFormLayout* mMainLayout;

	// Property group widgets
	QGroupBox* mCoordinateGroup;    // 좌표 정보
	QGroupBox* mSizeGroup;          // 크기 정보
	QGroupBox* mMetadataGroup;      // 메타데이터

	// Coordinate property labels (좌표 정보)
	QLabel* mPosX;          // 위치 X
	QLabel* mPosY;          // 위치 Y
	QLabel* mPosZ;          // 위치 Z

	// Size property labels (크기 정보)
	QLabel* mLength;        // 길이
	QLabel* mWidth;         // 폭
	QLabel* mHeight;        // 높이
	QLabel* mArea;          // 면적
	QLabel* mVolume;        // 부피

	// Metadata labels (메타데이터)
	QLabel* mArtifactName;  // 유물명
	QLabel* mSiteName;      // 유적명
	QLabel* mExcavDate;     // 발굴일자
	QLabel* mGridID;        // 그리드 번호
	QLabel* mLayer;         // 층위
};

#endif // DONGARCHPROPERTYPANEL_H
