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

static GLfloat *vertices = nullptr;
static GLfloat *colors = nullptr;

float alpha1 = 0;
float alpha2 = 0;
float alpha3 = 0;
float scaleX = 1;
float scaleY = 1;
float scaleZ = 1;

int n_vertices = 0;
bool b_initialized = false;
bool b_usecolor = false;

Histogram3dView::Histogram3dView(QWidget *p)
        : QGLWidget(p)
        , hist_(nullptr) 
        , dat_(nullptr)
        , dat_n_(0)
        , flags_(0)
        , spinning_(true) {
    auto update_timer = new QTimer(this);
    QObject::connect(update_timer, SIGNAL(timeout()), this, SLOT(updateGL())); //, Qt::QueuedConnection);
    update_timer->start(10);

    auto layout = new QGridLayout(this);
    int r = 0;
    {
        auto l = new QLabel("Threshold");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto sb = new QSpinBox;
        sb->setFixedSize(sb->sizeHint());
        sb->setFixedWidth(sb->width() * 1.5);
        sb->setRange(1, 10000);
        sb->setValue(4);
        thresh_ = sb;
        layout->addWidget(sb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Scale");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto sb = new QSpinBox;
        sb->setFixedSize(sb->sizeHint());
        sb->setFixedWidth(sb->width() * 1.5);
        sb->setRange(1, 10000);
        sb->setValue(100);
        scale_ = sb;
        layout->addWidget(sb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Histogram-Type");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QComboBox;
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
        type_ = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Overlap");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox;
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(true);
        overlap_ = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Histogram-Color");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox;
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(false);
        color_ = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto l = new QLabel("Anti-Aliasing");
        l->setFixedSize(l->sizeHint());
        layout->addWidget(l, r, 0);
    }
    {
        auto cb = new QCheckBox;
        cb->setFixedSize(cb->sizeHint());
        cb->setChecked(false);
        antialias_ = cb;
        layout->addWidget(cb, r, 1);
    }
    r++;

    {
        auto pb = new QPushButton("Resample");
        pb->setFixedSize(pb->sizeHint());
        layout->addWidget(pb, r, 0);
        QObject::connect(pb, SIGNAL(clicked()), this, SLOT(parameters_changed()));
    }
    r++;

    layout->setColumnStretch(2, 1);
    layout->setRowStretch(r, 1);

    QObject::connect(thresh_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(scale_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(regen_histo()));
    QObject::connect(overlap_, SIGNAL(toggled(bool)), this, SLOT(regen_histo()));
    QObject::connect(color_, SIGNAL(toggled(bool)), this, SLOT(color_histo()));
    QObject::connect(antialias_, SIGNAL(toggled(bool)), this, SLOT(initializeGL()));
}

Histogram3dView::~Histogram3dView() {
    delete[] hist_;
    delete[] vertices;
    delete[] colors;
}

void Histogram3dView::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    regen_histo();
}

void Histogram3dView::initializeGL() {

    static bool glSwitch = false;
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    if (!glSwitch)
    {
        glDisable(GL_MULTISAMPLE);
        glDisable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glSwitch = !glSwitch;
    b_initialized = true;
}

void Histogram3dView::resizeGL(int /*w*/, int /*h*/) {
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(20, (float)width() / (float)height(), 5, 100);
    glViewport(0, 0, (float)width(), (float)height());

    glMatrixMode(GL_MODELVIEW);
}

void Histogram3dView::paintGL() {

    transform_histo();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0, 0, -10);

    glRotatef(360, 1, 0, 0);
    glRotatef(alpha1, 1, 0, 0);
    glRotatef(alpha2, 0, 1, 0);

	glScalef(scaleX, scaleY, scaleZ);

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

                -1.05, -1.05, -1.05, -1 + .05, -1 - .05, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 + .05, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 - .05, -1 + .05
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
        if (vertices) {
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glColorPointer(3, GL_FLOAT, 0, colors);

            glDrawArrays(GL_POINTS, 0, n_vertices);
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

	//Check if high-order bit is set (1 << 15)
    glFlush();
}

void Histogram3dView::color_histo() {
    b_usecolor = !b_usecolor;
    regen_histo();
}

void Histogram3dView::transform_histo() {

    if (!spinning_) {
        return;
    }

    if (flags_ & MOVE_UP) {
        alpha1 = alpha1 + -0.7 * 1;
    }
    if (flags_ & MOVE_RIGHT) {
        alpha2 = alpha2 + 0.7 * 1;
    }
    if (flags_ & MOVE_DOWN) {
        alpha1 = alpha1 + 0.7 * 1;
    }
    if (flags_ & MOVE_LEFT) {
        alpha2 = alpha2 + -0.7 * 1;
    }
    if (flags_ & SCALE_UP) {

        if (!(flags_ & SCALE_UP_Z)) { // Z Only.
            scaleX += 0.01;
            scaleY += 0.01;
        }
        scaleZ += 0.01;
    }
    if (flags_ & SCALE_DOWN) {

        if (!(flags_ & SCALE_DOWN_Z)) { // Z Only.
            if (scaleX >= 0.05) {
                scaleX += -0.01;
            }
            if (scaleY >= 0.05) {
                scaleY += -0.01;
            }
        }
        if (scaleZ >= 0.05) {
            scaleZ += -0.01;
        }
    }
}

void Histogram3dView::regen_histo() {
    delete[] hist_;
    hist_ = nullptr;

    histo_dtype_t t = string_to_histo_dtype(type_->currentText().toStdString());

    hist_ = generate_histo_3d(dat_, dat_n_, t, overlap_->isChecked());

    parameters_changed();
}

void Histogram3dView::parameters_changed() {
    int thresh = thresh_->value();
    float scale_factor = scale_->value();

    n_vertices = 0;
    for (int i = 0; i < 256 * 256 * 256; i++) {
        if (hist_[i] >= thresh) {
            n_vertices++;
        }
    }

    delete[] vertices;
    vertices = nullptr;
    delete[] colors;
    colors = nullptr;

    if (n_vertices > 0) {
        vertices = new GLfloat[n_vertices * 3];
        colors = new GLfloat[n_vertices * 3];
        for (int i = 0, j = 0; i < 256 * 256 * 256; i++) {
            if (hist_[i] >= thresh) {
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
                
                vertices[j * 3 + 0] = x * 2. - 1.;
                vertices[j * 3 + 1] = y * 2. - 1.;
                vertices[j * 3 + 2] = z * 2. - 1.;

                float cc = hist_[i] / scale_factor;
                cc += .2;
                if (cc > 1.) {
                    cc = 1.;
                }

                // TEMP color display
                // TODO: replace with proper vertex coloring
                if (b_usecolor)
                {
                    if(colors[j * 3 + 0] == NULL)
                        colors[j * 3 + 0] = cc;
                    else
                        colors[j * 3 + 0] = cc;

                    if(colors[j * 3 + 4] == NULL)
                        colors[j * 3 + 4] = cc;
                    else
                        colors[j * 3 + 1] = cc;

                    if(colors[j * 3 + 8] == NULL)
                        colors[j * 3 + 8] = cc;
                    else
                        colors[j * 3 + 2] = cc;
                }
                else
                {
                    colors[j * 3 + 0] = cc;
                    colors[j * 3 + 1] = cc;
                    colors[j * 3 + 2] = cc;
                }
                j++;
            }
        }
    }
    printf("VERTEX COUNT: %d\n", n_vertices);

    updateGL();
}

bool Histogram3dView::isSpinning() const {
    return spinning_;
}

void Histogram3dView::setTransformFlags(int flags) {
    flags_ |= flags;
}

void Histogram3dView::removeTransformFlags(int flags) {
    flags_ &= ~flags;
}

void Histogram3dView::mousePressEvent(QMouseEvent *e) {
    e->accept();
}

void Histogram3dView::mouseMoveEvent(QMouseEvent *e) {
    e->accept();
}

void Histogram3dView::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();
    spinning_ = !spinning_;
}
