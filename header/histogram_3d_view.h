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

#ifndef _HISTOGRAM_3D_VIEW_
#define _HISTOGRAM_3D_VIEW_

#include <QGLWidget>

extern float alpha1;
extern float alpha2;
extern float alpha3;
extern float scaleX;
extern float scaleY;
extern float scaleZ;

extern bool g_bGLInitialized;

enum TransformFlags
{
    MOVE_UP      = 1<<0,
    MOVE_RIGHT   = 1<<1,
    MOVE_DOWN    = 1<<2,
    MOVE_LEFT    = 1<<3,
    SCALE_UP     = 1<<4,
    SCALE_DOWN   = 1<<5,
    SCALE_UP_Z   = 1<<6,
    SCALE_DOWN_Z = 1<<7,
};

class QSpinBox;

class QComboBox;

class QCheckBox;

class Histogram3dView : public QGLWidget {
Q_OBJECT
public:
    explicit Histogram3dView(QWidget *p = nullptr);

    ~Histogram3dView() override;

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

    bool isSpinning() const;

    void setTransformFlags(int flags);
    void removeTransformFlags(int flags);

protected slots:

    void regen_histo();

    void color_histo();

    void transform_histo();

    void initializeGL() override;

protected:

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    QSpinBox *thresh_, *scale_;
    QComboBox *type_;
    QCheckBox *overlap_;
    QCheckBox *color_;
    QCheckBox *antialias_;
    int *hist_;
    const unsigned char *dat_;
    long dat_n_;
    int flags_;
    bool spinning_;
};

#endif
