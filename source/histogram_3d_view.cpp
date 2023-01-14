/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cfloat>
#include <QtGui>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QKeyEvent>

#include <glut.h>

#include "histogram_calc.h"
#include "histogram_3d_view.h"

using std::isnan;
using std::signbit;
using std::isinf;

CHistogram3D::CHistogram3D(QWidget *p)
        : QGLWidget(p)
        , m_Vertices(nullptr)
        , m_Colors(nullptr)
        , m_Histogram(nullptr) 
        , m_Data(nullptr)
        , m_Size(0)
        , m_Flags(0)
        , m_VertexCount(0)
        , m_MouseX(0)
        , m_MouseY(0)
        , m_AngleX(0)
        , m_AngleY(0)
        , m_ScaleX(1)
        , m_ScaleY(1)
        , m_ScaleZ(1) {
    auto update_timer = new QTimer(this);
    QObject::connect(update_timer, SIGNAL(timeout()), this, SLOT(updateGL())); //, Qt::QueuedConnection);
    update_timer->start(10);


    this->setCursor(QCursor(Qt::CrossCursor));
    auto layout = new QGridLayout(this);
    int r = 0;
    {
        auto l = new QLabel("Threshold", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto sb = new QSpinBox(this);
        sb->setFixedSize(sb->sizeHint());
        sb->setFixedWidth(sb->width() * 1.5);
        sb->setRange(1, 10000);
        sb->setValue(4);
        m_Threshold = sb;
        layout->addWidget(sb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Scale", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto sb = new QSpinBox(this);
        sb->setFixedSize(sb->sizeHint());
        sb->setFixedWidth(sb->width() * 1.5);
        sb->setRange(1, 10000);
        sb->setValue(100);
        m_Scale = sb;
        layout->addWidget(sb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Histogram-Type", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QComboBox(this);
        cb->setFixedSize(cb->sizeHint());
        cb->addItem("U8");
        cb->addItem("U12");
        cb->addItem("U16");
        cb->addItem("U32");
        cb->addItem("U64");
        cb->addItem("F32");
        cb->addItem("F64");
        cb->setCurrentIndex(0);
        cb->setEditable(false);
        m_Type = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Histogram-Color", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox(this);
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(true);
        m_Color = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Overlap", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox(this);
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(true);
        m_Overlap = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Anti-Aliasing", this);
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox(this);
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(false);
        m_Antialias = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto pb = new QPushButton("Resample", this);
        pb->setFixedSize(pb->sizeHint());
        layout->addWidget(pb, r, 0);
        QObject::connect(pb, SIGNAL(clicked()), this, SLOT(regenHisto()));
    }
    r++;

    layout->setColumnStretch(2, 1);
    layout->setRowStretch(r, 1);

    QObject::connect(m_Threshold, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
    QObject::connect(m_Scale, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
    QObject::connect(m_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(regenHisto()));
    QObject::connect(m_Overlap, SIGNAL(toggled(bool)), this, SLOT(regenHisto()));
    QObject::connect(m_Color, SIGNAL(toggled(bool)), this, SLOT(colorHisto()));
    QObject::connect(m_Antialias, SIGNAL(toggled(bool)), this, SLOT(initializeGL()));
}

CHistogram3D::~CHistogram3D() {

    if (m_Histogram) {
        delete[] m_Histogram;
    }
    if (m_Vertices) {
        delete[] m_Vertices;
    }
    if (m_Colors) {
        delete[] m_Colors;
    }
}

void CHistogram3D::setData(const quint8 *dat, qsizetype n) {
    m_Data = dat;
    m_Size = n;

    regenHisto();
}

void CHistogram3D::initializeGL() {
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    if (!m_Antialias->isChecked())
    {
        glDisable(GL_MULTISAMPLE);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_POINT_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_LINE_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_POLYGON_SMOOTH);
    }
    else
    {
        glEnable(GL_MULTISAMPLE);
        glutSetOption(GLUT_MULTISAMPLE, 64);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CHistogram3D::resizeGL(int /*w*/, int /*h*/) {
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(20, (float)width() / (float)height(), 5, 100);
    glViewport(0, 0, (float)width(), (float)height());

    glMatrixMode(GL_MODELVIEW);
}

void CHistogram3D::paintGL() {

    transformHisto();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0, 0, -10);

    glRotatef(360, 1, 0, 0);
    glRotatef(m_AngleX, 1, 0, 0);
    glRotatef(m_AngleY, 0, 1, 0);

	glScalef(m_ScaleX, m_ScaleY, m_ScaleZ);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    if (1) {
        // Start at <-1, -1, -1>, and flip the sign on one dimension to produce a new unique point, continue until all paths terminate at <1,1,1>
        GLfloat lines_vertices[] = {
                -1, -1, -1, 1, -1, -1,
                -1, -1, -1, -1, 1, -1,
                -1, -1, -1, -1, -1, 1,

                1, -1, -1, 1, 1, -1,
                1, -1, -1, 1, -1, 1,

                -1, 1, -1, 1, 1, -1,
                -1, 1, -1, -1, 1, 1,

                -1, -1, 1, 1, -1, 1,
                -1, -1, 1, -1, 1, 1,

                -1, 1, 1, 1, 1, 1,
                1, -1, 1, 1, 1, 1,
                1, 1, -1, 1, 1, 1,

                -1.05, -1.05, -1.05, -1 + 2.00, -1 - .05, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 + 2.00, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 - .05, -1 + 2.00
        };

        // slightly offset the edges from the data
        for (int i = 0; i < 24 * 3; i++) {
            if (lines_vertices[i] < 0) { lines_vertices[i] -= .01; }
            if (lines_vertices[i] > 0) { lines_vertices[i] += .01; }
        }

        GLfloat lines_colors[] = {
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                1, .0, .0, 1, .0, .0,
                .0, 1, .0, .0, 1, .0,
                .0, .0, 1, .0, .0, 1
        };

        glVertexPointer(3, GL_FLOAT, 0, lines_vertices);
        glColorPointer(3, GL_FLOAT, 0, lines_colors);

        glDrawArrays(GL_LINES, 0, 24 + 6);
    }

    {
        if (m_Vertices) {
            glVertexPointer(3, GL_FLOAT, 0, m_Vertices);
            glColorPointer(3, GL_FLOAT, 0, m_Colors);

            glDrawArrays(GL_POINTS, 0, m_VertexCount);
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

	//Check if high-order bit is set (1 << 15)
    glFlush();
}

void CHistogram3D::colorHisto() {
    regenHisto();
}

void CHistogram3D::transformHisto() {
    if (m_Flags & MOVE_UP) {
        m_AngleX = m_AngleX + -0.7 * 1;
    }
    if (m_Flags & MOVE_RIGHT) {
        m_AngleY = m_AngleY + 0.7 * 1;
    }
    if (m_Flags & MOVE_DOWN) {
        m_AngleX = m_AngleX + 0.7 * 1;
    }
    if (m_Flags & MOVE_LEFT) {
        m_AngleY = m_AngleY + -0.7 * 1;
    }
    if (m_Flags & SCALE_UP) {

        if (!(m_Flags & SCALE_UP_Z)) { // Z Only.
            m_ScaleX += 0.01;
            m_ScaleY += 0.01;
        }
        m_ScaleZ += 0.01;
    }
    if (m_Flags & SCALE_DOWN) {

        if (!(m_Flags & SCALE_DOWN_Z)) { // Z Only.
            if (m_ScaleX >= 0.05) {
                m_ScaleX += -0.01;
            }
            if (m_ScaleY >= 0.05) {
                m_ScaleY += -0.01;
            }
        }
        if (m_ScaleZ >= 0.05) {
            m_ScaleZ += -0.01;
        }
    }
}

void CHistogram3D::regenHisto() {
    if (m_Histogram) {
        delete[] m_Histogram;
        m_Histogram = nullptr;
    }

    HistoDtype_t t = string_to_histo_dtype(m_Type->currentText().toStdString());
    m_Histogram = generate_histo_3d(m_Data, m_Size, t, m_Overlap->isChecked());

    parametersChanged();
}

void CHistogram3D::parametersChanged() {
    if (!m_Histogram)
        return;

    int thresh = m_Threshold->value();
    float scale_factor = m_Scale->value();

    m_VertexCount = 0;
    for (int i = 0; i < 256 * 256 * 256; i++) {
        if (m_Histogram[i] >= thresh) {
            m_VertexCount++;
        }
    }

    delete[] m_Vertices;
    m_Vertices = nullptr;
    delete[] m_Colors;
    m_Colors = nullptr;

    if (m_VertexCount > 0) {
        m_Vertices = new GLfloat[m_VertexCount * 3];
        m_Colors = new GLfloat[m_VertexCount * 3];
        for (int i = 0, j = 0; i < 256 * 256 * 256; i++) {
            if (m_Histogram[i] >= thresh) {
                float x = i / (256 * 256);
                float y = (i % (256 * 256)) / 256;
                float z = i % 256;
                x = x / 255.;
                y = y / 255.;
                z = z / 255.;
                
                if(x < -1 || x > 1.0 ||
                   y < -1 || y > 1.0 ||
                   z < -1 || z > 1.0) {
                  printf("Warning: 3dpc %f %f %f\n", x, y, z);
                }
                
                m_Vertices[j * 3 + 0] = x * 2. - 1.;
                m_Vertices[j * 3 + 1] = y * 2. - 1.;
                m_Vertices[j * 3 + 2] = z * 2. - 1.;

                float cc = m_Histogram[i] / scale_factor;
                cc += .2;
                if (cc > 1.) {
                    cc = 1.;
                }

                if (m_Color->isChecked())
                {
                    if (cc < 0.7f)
                    {
                        if (cc > 0.375f)
                        {
                            float g = (cc - 0.35f) / (0.7f - 0.35f) * (1.0f - 0.35f) + 0.35f;
                            m_Colors[j * 3 + 0] = sqrt(cc/3);
                            m_Colors[j * 3 + 1] = g;
                            m_Colors[j * 3 + 2] = sqrt(cc/3);
                        }
                        else
                        {
                            m_Colors[j * 3 + 0] = cc;
                            m_Colors[j * 3 + 1] = cc;
                            m_Colors[j * 3 + 2] = cc+0.0f;
                        }
                    }
                    else
                    {
                        float r = (cc > 0.7f) ? 1.0f : cc * 2;
                        float g = (cc < 0.7f) ? 1.0f : 1.0f - (cc - 0.7f) * 2;
                        float b = 0.0f;
                        m_Colors[j * 3 + 0] = r;
                        m_Colors[j * 3 + 1] = g;
                        m_Colors[j * 3 + 2] = b;
                    }
                }
                else
                {
                    m_Colors[j * 3 + 0] = cc;
                    m_Colors[j * 3 + 1] = cc;
                    m_Colors[j * 3 + 2] = cc;
                }
                j++;
            }
        }
    }

    printf("%s(%d)\n", "m_VertexCount", m_VertexCount);
    updateGL();
}

void CHistogram3D::setPositionScale(float f) {
    m_ScaleX = f;
    m_ScaleY = f;
    m_ScaleZ = f;
}

void CHistogram3D::setTransformFlags(int flags) {
    m_Flags |= flags;
}

void CHistogram3D::removeTransformFlags(int flags) {
    m_Flags &= ~flags;
}

void CHistogram3D::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_MouseX = event->x();
        m_MouseY = event->y();
        event->accept();
    }

    setFocus();
}

void CHistogram3D::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() == Qt::RightButton) {
        return;
    }

    int delta_x = m_MouseX - event->x();
    int delta_y = m_MouseY - event->y();

    m_AngleX -= delta_y;
    m_AngleY -= delta_x;

    m_MouseX = event->x();
    m_MouseY = event->y();

    event->accept();
}

void CHistogram3D::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();
}

void CHistogram3D::wheelEvent(QWheelEvent* event)
{
    float scaleFactor = 0.0005;
    float scale = m_ScaleX;
    scale = scale + (event->delta() * scaleFactor);

    if (scale >= 0.0001) {
        setPositionScale(scale);
    }
}