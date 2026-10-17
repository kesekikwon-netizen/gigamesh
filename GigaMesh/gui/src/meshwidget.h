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

#ifndef MESHWIDGET_H
#define MESHWIDGET_H

//#define GL_GLEXT_PROTOTYPES
//#define DEAD_CORE_PROFILE

// generic C++ includes:
#include <chrono>

// C++ includes:
#include "meshwidget_params.h"

// generic Qt includes:
//.

// Qt includes:
#include "meshQt.h"

#include "QDir"

#include "svg/SvgWriter.h"


// OpenGL stuff
// OpenGL headers should better be included AFTER Qt headers and any header that includes Qt ...
#ifdef MACXPORT
	#include <gl.h>
	#include <OpenGL/glu.h>
#else
	//#include <QtOpenGL>
	//#include <GL/glu.h>
#endif
#include <QtOpenGL/QGLWidget>
#include <QOpenGLShaderProgram>

class QGMMainWindow;
class MeshQt;
class Section;

//!
//! \brief Qt OpenGL widget for visualization of a 3D-mesh. (Layer 2)
//!
//! Initalizes OpenGL and provides interactive rotation of a mesh; its viewpoint
//! and light sources using Qt.
//!
//! Layer 2
//!
//! Navigation using a two-button mouse with wheel (required!)
//! <ul>
//!   <li>Leftbutton          = rotate up/down and left/right</li>
//!   <li>Rightbutton         = shift up/down and left/right</li>
//!   <li>Rightbutton + Shift = shift forward/backward and left/right</li>
//!   <li>Wheel               = zoom in/out</li>
//! </ul>
//!
//! Selection of a Primitive: only one type (Vertex, Edge or Face) can be selected.

class MeshWidget : public QGLWidget, public MeshWidgetParams, public MeshGLColors {
    Q_OBJECT

public:
	// Constructor and Destructor:
	MeshWidget( const QGLFormat& format, QWidget* parent );
	~MeshWidget() override;

	bool    getViewPortResolution( double& rRealWidth, double& rRealHeight ) const;
	bool    getViewPortPixelWorldSize( double& rPixelWidth, double& rPixelHeight ) const;
	bool    getViewPortDPI( double& rDPI ) const;
	bool    getViewPortDPM( double& rDPM ) const;

	// Mesh access for Section system (Archaeological)
	MeshQt* getMeshVisual() { return mMeshVisual; }
	void setActiveSectionPlane(Section* section);

	// Section overlay rendering (DongArch Arch3D Liner style)
	void setSectionPolyline(const QVector<QPointF>& screenPoints);
	void setSectionPolyline3D(const std::vector<Vector3D>& worldPoints);  // 3D world coordinates version
	void setSectionStyle(const QColor& color, float width, float alpha);
	void clearSectionOverlay();

public slots: // ... overloaded from MeshWidgetParams:
	bool    setParamFlagMeshWidget(    MeshWidgetParams::eParamFlag rFlagNr,  bool   rState  ) override;
	bool    toggleShowFlag(            MeshWidgetParams::eParamFlag rFlagNr ) override;
	virtual bool    setParamIntegerMeshWidget( MeshWidgetParams::eParamInt  rParam ); // will trigger an enter-text-dialog.
	bool    setParamIntegerMeshWidget( MeshWidgetParams::eParamInt  rParam,   int    rValue  ) override;
	bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID, double rValue  ) override;
	virtual bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID, double rMinValue, double rMaxValue ); // will trigger a slider-dialog.
	virtual bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID ); // will trigger text-enter-dialog.
	bool    setParamStringMeshWidget(const eParamStr rParamID, const std::string& rString ) override;
private slots: // ... to be avoided - for compatibility of old methods only!
	virtual bool    setParamFloatMeshWidgetSlider( int rParamID, double rValue );
	// new method:
	        bool    callFunctionMeshWidget( MeshWidgetParams::eFunctionCall rFunctionID, bool rFlagOptional=false );

private:
			bool    paramFlagChanged(const eParamFlag rFlagNr );

private slots:
    virtual bool    setPlaneHNFByView();

public slots:
	// File menu (and related)
	bool fileOpen( const QString& rFileName );
    bool reloadFile(const bool askQuestion=true);
	//.
	bool saveStillImagesSettings();
	void saveStillImages360( Vector3D rotCenter, Vector3D rotAxis );
	void saveStillImages360HLR();
	void saveStillImages360VUp();
	void saveStillImages360PrimN();
	void saveStillImages360PlaneN();
	void saveStillImages360Angles( float** stepAngles, int* stepAnglesNr );
	//.
	void sphericalImagesLight();
	void sphericalImagesLight(const QString& rFileName );
    void sphericalImagesLight(const QString& rFileName, const bool rUseTiled );
    void sphericalImagesLight(const QString& rFileName, const bool rUseTiled, const double rStepLight );
    void sphericalImagesLightConstTheta(const QString& rFileName, const bool rUseTiled, const double rStepLight );
    void sphericalImagesLightDir();
    void sphericalImages();
	void sphericalImages(const QString& rFileName );
	void sphericalImages(const QString& rFileName, const bool rUseTiled );
	bool sphericalImagesStateNr();
	//.
	void unloadMesh();

    //. Edit->Transform
    void applyAutomaticMeshAlignmentDir();

private:
	// Screenshot - Single
	bool screenshotSingle();
	bool screenshotSingle( const QString& rFileName );
	bool screenshotSingle( const QString& rFileName, bool rUseTiled, double& rWidthReal, double& rHeigthReal );

	// Screenshot - Wrapping methods
	bool screenshotPDFUser();
    bool screenshotPDF( const QString& rFileName, const bool rUseTiled, const QString rTemplate, const QString rTexFileName, const QString rCCparameter, const QString rCCversion );

private:
	// Screenshot - Views - Wrapping methods
	bool screenshotViewsDirectoryFiles( QString& rPathChoosen, QStringList& rCurrFiles ); // Internal use only


private:
    //Help functions of PDF Latex export

    //structure of all possible Latex Placeholder
    // need for a Helping Text
    struct LatexPlaceholder{
        QString placeholder;
        QString descr; //description for the Helper Text
    };
    std::vector<LatexPlaceholder> mPdfViewsPlaceholders;


    void setLatexPlaceholderDefinition();
    bool checkUserdefinedLatexFile(QString *latexTemplate,std::vector<LatexPlaceholder> rPlaceHolders);
    bool askForCCLicenseParameters(QString *ccParameter,QString *ccVersion);
    bool mUserContinue; //needed for the directory function. Don't give for every mesh the information box

public:
	bool screenshotViewsDirectory();
	bool directoryFuncValToRGB();
	bool screenshotViewsPDFUser();
    bool screenshotViewsPDF( const QString& rFileName, const QString rTemplate, const QString rTexFileName, const QString rCCparameter, const QString rCCversion );
private:
	bool screenshotPDFMake( const QString& rPrefixPath, const QString& rFilePrefixTex );
    std::vector<LatexPlaceholder> mPdfSinglePlaceholders;
public slots:
	// Screenshot - Views - Rendering
	void screenshotViews();
	void screenshotViews( const QString& rFileName );
	void screenshotViews( const QString&          rFilePattern,
	                      const QString&          rFilePrefix,
	                      const bool              rUseTiled,
	                      std::vector<QString>&   rImageFiles,
	                      std::vector<double>&    rImageSizes );

private:

	// Helper class to render Screenshots into an OffscreenBuffer => only have one of the class initialized at a time
	class OffscreenBuffer
	{
	public:
		OffscreenBuffer(QGLContext *context);
		~OffscreenBuffer();

		OffscreenBuffer(const OffscreenBuffer& other) = delete;
		OffscreenBuffer(const OffscreenBuffer&& other) = delete;

		OffscreenBuffer& operator=(const OffscreenBuffer& other) = delete;
		OffscreenBuffer& operator=(const OffscreenBuffer&& other) = delete;

		GLuint getFboID();
		unsigned char* getColorTexture(int &width, int &height);
		void getColorTextureRegion(unsigned char* &data, int width, int height, int xOffset, int yOffset);
		float* getDepthTexture(int &width, int &height);
		void getDepthTextureRegion(float* &data, int width, int height, int xOffset, int yOffset);

	private:
		GLuint mColorTextureID;
		GLuint mDepthTextureID;
		GLuint mFboID;
		QGLContext* mContext;
		int mTexWidth;
		int mTexHeight;
		unsigned char* mColorTextureBuffer;
		float* mDepthTextureBuffer;
	};

	void bindFramebuffer(int framebufferID);

	// Fetch screenshots:
	bool prepareTile(uint64_t rTilesX, uint64_t rTilesY, unsigned char** rImRGBA, uint64_t* rImWidth, uint64_t* rImHeight, uint64_t rBorderSize = 0 );
	bool fetchFrameAndZBufferTile(unsigned int rTilesX, unsigned int rTilesY, unsigned int rTX, unsigned int rTY, unsigned char* rImRGBA, uint64_t rImWidth, uint64_t rImHeight, OffscreenBuffer* offscreenBuffer, long rBorderSize = 0 );
	bool fetchFrameAndZBuffer( unsigned char*& rImRGBA, uint64_t& rImWidth, uint64_t& rImHeight, bool rCropUsingZBuffer, OffscreenBuffer* offscreenBuffer, bool keepBackground = false );
	bool fetchFrameBuffer( unsigned char** rImArray, int* rImWidth, int* rImHeight, bool rCropUsingZBuffer, OffscreenBuffer* offscreenBuffer );
	// Write screenshots:
	bool screenshotPNG(const QString& rFileName, double& rWidthReal, double& rHeigthReal , OffscreenBuffer *offscreenBuffer);

	// View menu
	bool getViewSettingsTxt( QString& rSettingsStr ) const;
	bool getViewSettingsTTL( QString& rSettingsStr, QString& uri) const;
	bool getViewSettingsJSON( QString& rSettingsStr ) const;
	bool showView2DBoundingBox();
	bool showViewMatrix();
	bool setViewMatrix();
	bool setViewMatrix( std::vector<double> rMatrix );
	bool setViewAxisUp();

public slots:
	void selectColorBackground();

	// Archaeological mode - cortex selection
	void enterCortexSelectionMode(bool rEnabled);
	void clearCortexSelection();

	// View menu
	bool orthoSetDPI();
	bool orthoSetDPI( double rSetTo );

	// v4 Phase 1: Align - ViewPoint 6개
	void setViewBottom();     //!< 하면 뷰
	void setViewBack();       //!< 후면 뷰
	void setViewLeft();       //!< 좌측면 뷰

	// v4 Phase 1: Align - Rotation 변환
	void rotateAroundX();     //!< X축 90도 회전
	void rotateAroundY();     //!< Y축 90도 회전
	void rotateAroundZ();     //!< Z축 90도 회전

	// v4 Phase 1: Align - Ground Plane
	void fitMeshToGroundPlane();  //!< 메시를 Ground Plane(Z=0)에 정렬

	// v4 Phase 2: Cutline - Mesh-Plane Intersection
	bool extractCutline(const Vector3D& planeHNF, std::vector<Vector3D>* outPoints);  //!< Extract cutline from mesh-plane intersection
	//.
	bool screenshotSVG();
	bool screenshotSVG(const QString& rFileName, const QString& rFileNamePNG );

    bool checkInkscapeAvailability();
	bool exportPlaneIntersectPolyLinesSVG();
	bool screenshotSVGexportPlaneIntersections( double rOffsetX, double rOffsetY, double rPolyLineWidth, double axisOffset, SvgWriter& svgWriter, const std::set<unsigned int>& polylineIDs );

	bool screenshotSVGexportPolyLines( Vector3D& cameraViewDir, Matrix4D& matView, double polyScaleWdith, double polyScaleHeight, double polyLineWidth, SvgWriter& svgWriter );

	void screenshotRuler();
	bool screenshotRuler( const QString& rFileName );
	bool screenshotTiledPNG(QString rFileName, double& rWidthReal, double& rHeigthReal, OffscreenBuffer *offscreenBuffer, int rBorderSize=0);
	bool fetchRuler( unsigned char** rImArray, int* rImWidth, int* rImHeight, double& rPixelWidth, double& rPixelHeight );
	bool writePNG( const QString& rFileName, uint64_t rImWidth, uint64_t rImHeight, unsigned char* rImRGBA, int rDotsPerMeterWidth, int rDotsPerMeterHeight );

	bool cropRGBAbyAlpha( uint64_t* rImWidth, uint64_t* rImHeight, unsigned char** rImRGBA, double& rRealWidth, double& rRealHeight );
	//.
	void defaultViewLight();
	void defaultViewLightZoom();
	//.
	bool rotYaw();
	bool rotPitch();
	bool rotRoll();
	//..
	bool rotYaw( double rAngle );
	bool rotPitch( double rAngle );
	bool rotRoll( double rAngle );
	bool rotRollPitchYaw(double rAngle, double pAngle, double yAngle);
	void rotOrthoPlane();
	void rotArbitAxis( Vector3D rCenter, Vector3D rAxis, double rAngle );
	//.
    bool rotPlaneYaw( double rAngle );
    bool rotPlanePitch( double rAngle );
    bool rotPlaneRoll( double rAngle );
	//.
	void selPrimViewReference();
	//.
	bool currentViewToDefault();

	void openNormalSphereSelectionDialog(bool faces);
	void setCameraRotation(QQuaternion rotationQuat);

signals:
	void sStatusMessage(QString);                                   //!< Notify (the MainWindow) about status changes.
	void sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString);    //!< Infos emitted by the viewport e.g. for display purposes.
	// signals to the select menu:
	void sParamFlagMesh(MeshGLParams::eParamFlag,bool);             //!< Sets a specific display flag see MeshGL::setShowFlag.
	void sParamFlagMeshWidget(MeshWidgetParams::eParamFlag,bool);   //!< Sets a specific display flag see MeshWidget::setShowFlag.
	void sParamIntegerMeshWidget(MeshWidgetParams::eParamInt,int);  //!< Sets a specific display integer see MeshWidget::
    void sSelectPoly(std::vector<QPoint>&);                         //!< Pixel coordinates for polygonal/prism selection.
    void sDeSelectPoly(std::vector<QPoint>&);                       //!< Pixel coordinates for polygonal/prism deselection.
	// changes to plane position
	void sApplyTransfromToPlane(Matrix4D);                          //!< Emitted when the planes postion was (interactivly) changed.
	// guide for docked window
	void sGuideIDCommon(MeshWidgetParams::eGuideIDCommon);          //!< guide ID for sidebar
	void sGuideIDSelection(MeshWidgetParams::eGuideIDSelection);    //!< guide ID for sidebar

	void camRotationChanged(Vector3D, Vector3D);                            //!< signal emitted when camera is rotated by mouse

	void loadedMeshIsTextured(bool);

	// Section overlay signals
	void sectionOffsetDeltaRequested(float delta);                       //!< Emitted when wheel event requests section offset change

private:
	void initializeGL() override;
	void initializeVAO();
	void initializeShaders();
	void resizeGL( int width, int height ) override;
	void paintEvent( QPaintEvent *rEvent ) override;
	void paintSelection();
	bool paintHistogram();
	bool paintHistogramScence();
	void resizeEvent( QResizeEvent * event ) override;

	// Keyboard and Mouse interaction:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void mousePressEvent( QMouseEvent *rEvent ) override;
	void mouseDoubleClickEvent( QMouseEvent* rEvent) override;
	void mouseReleaseEvent( QMouseEvent *rEvent) override;
	void mouseMoveEvent( QMouseEvent *rEvent ) override;
	void wheelEvent( QWheelEvent * rEvent ) override;
	void wheelEventZoom( double rWheelDelta ); // Helper function
	void keyPressEvent( QKeyEvent *rEvent ) override;

	// User Interaction:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	bool userSetConeAxisCentralPixel();
	bool userSelectByMouseClick( QPoint rPoint, QFlags<Qt::MouseButton> rMouseButton );
	bool userSelectAtMouseLeft( const QPoint& rPoint );
	bool userSelectAtMouseRight( const QPoint& rPoint );

	// Cortex Selection:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	bool selectCortexVertexAt( int xPixel, int yPixel );
	void cortexVerticesChanged();

	// Main members
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QGMMainWindow*      mMainWindow;     //!< reference to our main/top-level window (AKA parent)
	MeshQt*             mMeshVisual;     //!< Pointer to the Mesh (overloaded with functions for OpenGL and Qt) shown.
	bool                mCortexSelectionModeActive; //!< True when in cortex vertex selection mode.
	Section*            mActiveSectionPlane; //!< Currently active section plane for interactive editing (Archaeological)

	// drawing stuff and setting the camera:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void setViewInitial();
	void setViewInitialZoom();
	void setView( GLdouble* rOrthoViewPort=nullptr );
	void setViewModelMat();
	int height() const;
	int width() const;

	// Camera parameters (OpenGL)
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	Vector3D   mCenterView;            //!< the central point we(=camera/eye) are looking at.
	Vector3D   mCameraCenter;          //!< center of the camera(=eye) (X-coordinate).
	Vector3D   mCameraUp;              //!< camera orientation.

	// Matrices for OpenGL computed using the camera parameters (above):
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QMatrix4x4 mMatProjection;      //!< OpenGL projection matrix.
	QMatrix4x4 mMatModelView;       //!< OpenGL modelview matrix.

	// Mouse and keyboard interaction
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QPoint  mLastPos;                 //!< Stores the last cursor position. Used to determine movement of the mouse for interaction.
	std::vector<QPoint> mSelectionPoly;    //!< Screen coordinates of the selection poylgon.
	std::vector<Vector3D> mSectionDragPoints;  //!< 3D points for section plane creation by dragging (archaeological illustration mode).
	std::vector<Vector3D> mSectionIntersectionPoints;  //!< Intersection points between section plane and mesh (archaeological illustration mode).

	// performance evaluation (frames per second):
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QElapsedTimer mFrameTime;  //!< Timer to estimate the frames per second (OpenGL).
	int   mFrameCount; //!< Counter to estimate the frames per second (OpenGL).

	// Vertex Buffer Objects and Shaders for the widget's background
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	GLuint mVAO; //!< Vertex Array Object - has to be created!
	enum eVBOs {
		VBO_BACKGROUND_VERTICES, //!< VBO: with four vertices for the background canvas.
		VBO_ARRAY_COUNT          //!< Number of VBOs.
	};
	QOpenGLBuffer mVertBufObjs[VBO_ARRAY_COUNT]; //!< Array of Vertex Buffer Objects (within the VAO).

	// One shader per background type and one shader for images:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QOpenGLShaderProgram* mShaderGridOrtho;        //!< Shader program for rectangular grid.
	QOpenGLShaderProgram* mShaderGridPolarLines;   //!< Shader program for polar grid -- lines.
	QOpenGLShaderProgram* mShaderGridPolarCircles; //!< Shader program for polar grid -- concentric circles.
	QOpenGLShaderProgram* mShaderGridHighLightCenter;  //!< Shader program highlighting the central pixel.
	QOpenGLShaderProgram* mShaderImage;            //!< Shader program for display of images e.g. logos, because QPainter is not compliant with OpenGL CoreProfile.

	// Texture maps used by the shader programs
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	enum eTextureMaps {
		TEXMAP_GIGAMESH_LOGO,              //!< Image showing the GigaMesh logo.
		TEXMAP_KEYBOARD_LAYOUT,            //!< Image showing the keyboard layout.
		TEXMAP_HISTOGRAM_SCENE,            //!< Computed image showing the histogram of the scence.
		TEXMAP_HISTOGRAM_MESH_FUNCVAL,     //!< Computed using the function values of the mesh.
		TEXMAP_SELECTION_POLYGON_OVERLAY,  //!< Polygon related to the selection.
		TEXMAP_COUNT                       //!< Number of texture maps.
	};
	QOpenGLTexture* mTextureMaps[TEXMAP_COUNT];  //!< Texturemap IDs for the shader. Currently there is just one holding the color ramps.

	// Methods for initalization and painting:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	bool initializeTextureMap( eTextureMaps rTextureMap, const QString& rFileName );
	bool initializeTextureMap( eTextureMaps rTextureMap, QImage* rImage );
	void initializeShader(const QString& rFileName, QOpenGLShaderProgram** rShaderProgram );
	bool paintBackgroundShader( QOpenGLShaderProgram** rShaderProgram );
	bool paintRasterImage( eTextureMaps rTexMap, int rPixelX, int rPixelY, int rPixelWidth, int rPixelHeight );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	//! Checkes if mesh might cause problems. E.g. too small or georeferenced (far away from origin)
	void checkMeshSanity();
	void checkMissingTextures(ModelMetaData& metadata);

	// Section rendering (Archaeological)
	//------------------------------------------------------------------------------------------------------------------------------------------------------
public:
	void setSectionManager(class SectionManager* manager);
	void renderSectionPlane(class Section* section);
	void renderAllSectionPlanes();
	void renderSectionPolylines();

private:
	class SectionManager* mSectionManager;  //!< Section manager for archaeological sections

	// Section overlay rendering members
	QVector<QPointF> m_sectionPolylineScreen; //!< Screen-space polyline for section overlay
	bool m_showSectionOverlay;                //!< Show section overlay flag
	QColor m_sectionLineColor;                //!< Section line color
	float m_sectionLineWidth;                 //!< Section line width in pixels
	float m_sectionLineAlpha;                 //!< Section line transparency (0-1)
};

#endif // MESHWIDGET_H
