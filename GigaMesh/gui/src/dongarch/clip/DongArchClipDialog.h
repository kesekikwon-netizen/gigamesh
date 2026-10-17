/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_CLIP_DIALOG_H
#define DONGARCH_CLIP_DIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "DongArchClipManager.h"

//! \file DongArchClipDialog.h
//! \brief Phase 4: Clip Interactive Preview Dialog (v4.0)
//!
//! Qt Dialog for interactive mesh clipping with real-time OpenGL preview
//!
//! Features:
//! - Height slider with real-time preview
//! - Clip mode selection (Keep Front/Back/Both)
//! - Preview toggle checkbox
//! - Execute button to apply clipping
//! - Auto-zoom to clip plane option

class MeshWidget;

namespace DongArch {
namespace Clip {

//! \class DongArchClipDialog
//! \brief Interactive clipping dialog with OpenGL preview
//!
//! Provides UI controls for:
//! - Height slider (real-time update)
//! - Clip mode radio buttons
//! - Preview checkbox
//! - Execute/Cancel buttons
class DongArchClipDialog : public QDialog {
    Q_OBJECT

public:
    //! Constructor
    //! \param meshWidget OpenGL widget for preview
    //! \param clipManager Clip manager for operations
    //! \param parent Parent widget
    explicit DongArchClipDialog(MeshWidget* meshWidget,
                                DongArchClipManager* clipManager,
                                QWidget* parent = nullptr);

    //! Destructor
    ~DongArchClipDialog() override;

    //! Get current clip height (0.0-1.0 normalized)
    //! \return Normalized height
    double getClipHeight() const;

    //! Get current clip mode
    //! \return Selected clip mode
    ClipMode getClipMode() const;

    //! Get preview enabled state
    //! \return True if preview is enabled
    bool isPreviewEnabled() const;

signals:
    //! Emitted when clip height changes
    //! \param height Normalized height (0.0-1.0)
    void clipHeightChanged(double height);

    //! Emitted when clip mode changes
    //! \param mode New clip mode
    void clipModeChanged(ClipMode mode);

    //! Emitted when preview state changes
    //! \param enabled Preview enabled/disabled
    void previewStateChanged(bool enabled);

    //! Emitted when user clicks Execute
    void executeClip();

private slots:
    //! Handle height slider value change
    void onHeightSliderChanged(int value);

    //! Handle height spin box value change
    void onHeightSpinBoxChanged(double value);

    //! Handle clip mode radio button change
    void onClipModeChanged();

    //! Handle preview checkbox state change
    void onPreviewCheckboxChanged(int state);

    //! Handle Execute button click
    void onExecuteClicked();

    //! Handle Cancel button click
    void onCancelClicked();

    //! Auto-zoom to clip plane
    void onAutoZoomClicked();

protected:
    //! Override wheel event for mouse wheel control
    //! Allows direct height adjustment with mouse wheel
    void wheelEvent(QWheelEvent* event) override;

private:
    //! Create UI controls
    void createUI();

    //! Create height controls (slider + spin box)
    QWidget* createHeightControls();

    //! Create mode selection controls
    QWidget* createModeControls();

    //! Create action buttons (Execute/Cancel)
    QWidget* createActionButtons();

    //! Update preview in OpenGL viewport
    void updatePreview();

    //! Compute clip plane from current height
    //! \return Clip plane in Hesse Normal Form
    Vector3D computeClipPlane() const;

    // UI Components
    QSlider* mHeightSlider;           //!< Height slider (0-1000)
    QDoubleSpinBox* mHeightSpinBox;   //!< Height value display (0.0-1.0)
    QRadioButton* mRadioKeepFront;    //!< Keep Front mode
    QRadioButton* mRadioKeepBack;     //!< Keep Back mode
    QRadioButton* mRadioKeepBoth;     //!< Keep Both mode (split)
    QButtonGroup* mModeButtonGroup;   //!< Mode button group
    QCheckBox* mPreviewCheckbox;      //!< Enable/disable preview
    QPushButton* mExecuteButton;      //!< Execute button
    QPushButton* mCancelButton;       //!< Cancel button
    QPushButton* mAutoZoomButton;     //!< Auto-zoom to plane

    // Data
    MeshWidget* mMeshWidget;          //!< OpenGL widget (non-owning)
    DongArchClipManager* mClipManager;//!< Clip manager (non-owning)
    double mCurrentHeight;            //!< Current height (0.0-1.0)
    bool mPreviewEnabled;             //!< Preview state
    ClipMode mCurrentMode;            //!< Current clip mode

    // Mesh bounds (for height normalization)
    double mMeshMinZ;                 //!< Mesh minimum Z
    double mMeshMaxZ;                 //!< Mesh maximum Z
    double mMeshCenterZ;              //!< Mesh center Z
};

} // namespace Clip
} // namespace DongArch

#endif // DONGARCH_CLIP_DIALOG_H
