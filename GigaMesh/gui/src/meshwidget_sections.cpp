// meshwidget_sections.cpp
// Section rendering functions for MeshWidget

#include "meshwidget.h"
#include "section.h"
#include "sectionmanager.h"
#include <GL/gl.h>
#include <iostream>

//! Render a semi-transparent plane for section visualization
void MeshWidget::renderSectionPlane(Section* section)
{
    if (section == nullptr || !section->isVisible()) {
        return;
    }

    // Get plane parameters (Hesse Normal Form: ax + by + cz + d = 0)
    Vector3D planeHNF = section->getPlaneHNF();
    double nx = planeHNF.getX();
    double ny = planeHNF.getY();
    double nz = planeHNF.getZ();
    double d = planeHNF.getH();  // distance from origin

    // Get mesh bounding box to determine plane size
    // TODO: Get actual bounding box from mesh
    double minX = -100.0, maxX = 100.0;
    double minY = -100.0, maxY = 100.0;
    double minZ = -100.0, maxZ = 100.0;

    // Calculate four corners of the plane rectangle
    // We need to find two perpendicular vectors in the plane
    Vector3D normal(nx, ny, nz, 0.0);
    normal = ::normalize3(normal);

    // Find a perpendicular vector
    Vector3D u;
    if (abs(nx) < 0.9) {
        u = Vector3D(1.0, 0.0, 0.0, 0.0);
    } else {
        u = Vector3D(0.0, 1.0, 0.0, 0.0);
    }

    // Make u perpendicular to normal
    double dot = ::dot3(u, normal);
    u = u - (normal * dot);
    u = ::normalize3(u);

    // Get second perpendicular vector
    Vector3D v = normal % u;  // cross product operator
    v = ::normalize3(v);

    // Calculate plane center point
    Vector3D center = normal * (-d);

    // Calculate plane size based on bounding box
    double size = std::max({
        maxX - minX,
        maxY - minY,
        maxZ - minZ
    });

    // Four corners of the plane
    Vector3D corner1 = center + (u * size) + (v * size);
    Vector3D corner2 = center - (u * size) + (v * size);
    Vector3D corner3 = center - (u * size) - (v * size);
    Vector3D corner4 = center + (u * size) - (v * size);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth writing for transparent objects
    glDepthMask(GL_FALSE);

    // Set plane color (orange with transparency)
    QColor planeColor = section->getColor();
    glColor4f(
        planeColor.redF(),
        planeColor.greenF(),
        planeColor.blueF(),
        0.3f  // 30% opacity
    );

    // Draw the plane as a filled quad
    glBegin(GL_QUADS);
        glVertex3d(corner1.getX(), corner1.getY(), corner1.getZ());
        glVertex3d(corner2.getX(), corner2.getY(), corner2.getZ());
        glVertex3d(corner3.getX(), corner3.getY(), corner3.getZ());
        glVertex3d(corner4.getX(), corner4.getY(), corner4.getZ());
    glEnd();

    // Draw plane border (thicker, more opaque)
    glLineWidth(2.0f);
    glColor4f(
        planeColor.redF(),
        planeColor.greenF(),
        planeColor.blueF(),
        0.8f  // 80% opacity for border
    );

    glBegin(GL_LINE_LOOP);
        glVertex3d(corner1.getX(), corner1.getY(), corner1.getZ());
        glVertex3d(corner2.getX(), corner2.getY(), corner2.getZ());
        glVertex3d(corner3.getX(), corner3.getY(), corner3.getZ());
        glVertex3d(corner4.getX(), corner4.getY(), corner4.getZ());
    glEnd();

    // Draw normal vector (for debugging)
    if (false) {  // Set to true for debugging
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for normal
        glLineWidth(3.0f);
        glBegin(GL_LINES);
            glVertex3d(center.getX(), center.getY(), center.getZ());
            Vector3D normalEnd = center + (normal * size * 0.2);
            glVertex3d(normalEnd.getX(), normalEnd.getY(), normalEnd.getZ());
        glEnd();
    }

    // Re-enable depth writing
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
}

//! Render all section planes
void MeshWidget::renderAllSectionPlanes()
{
    if (mSectionManager == nullptr) {
        return;
    }

    // Save current OpenGL state
    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);

    for (int i = 0; i < mSectionManager->getSectionCount(); ++i) {
        Section* section = mSectionManager->getSection(i);
        if (section != nullptr && section->isVisible()) {
            renderSectionPlane(section);
        }
    }

    // Restore OpenGL state
    if (depthTest) {
        glEnable(GL_DEPTH_TEST);
    }
}

//! Render section intersection polylines
void MeshWidget::renderSectionPolylines()
{
    if (mSectionManager == nullptr) {
        return;
    }

    for (int i = 0; i < mSectionManager->getSectionCount(); ++i) {
        Section* section = mSectionManager->getSection(i);
        if (section == nullptr || !section->isVisible() || !section->hasPolyLine()) {
            continue;
        }

        // Get polyline
        PolyLine* polyline = section->getPolyLine();
        if (polyline == nullptr || polyline->length() < 2) {
            continue;
        }

        // Set polyline color
        QColor lineColor = section->getColor();
        glColor3f(
            lineColor.redF(),
            lineColor.greenF(),
            lineColor.blueF()
        );

        // Draw polyline
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < polyline->length(); ++j) {
            Vertex* vert = polyline->getVertexRef(j);
            if (vert != nullptr) {
                Vector3D vertex = vert->getPositionVector();
                glVertex3d(vertex.getX(), vertex.getY(), vertex.getZ());
            }
        }
        glEnd();

        // Draw vertices as points
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for (int j = 0; j < polyline->length(); ++j) {
            Vertex* vert = polyline->getVertexRef(j);
            if (vert != nullptr) {
                Vector3D vertex = vert->getPositionVector();
                glVertex3d(vertex.getX(), vertex.getY(), vertex.getZ());
            }
        }
        glEnd();
    }

    glLineWidth(1.0f);
    glPointSize(1.0f);
}

//! Set the section manager for rendering
void MeshWidget::setSectionManager(SectionManager* manager)
{
    mSectionManager = manager;
    update();  // Trigger repaint
}