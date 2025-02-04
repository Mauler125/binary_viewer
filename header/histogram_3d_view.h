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

class CHistogram3D : public QGLWidget {
Q_OBJECT
public:
    explicit CHistogram3D(QWidget *p = nullptr);
    ~CHistogram3D() override;

public slots:
    void setData(const quint8 *dat, qsizetype n);
    void parametersChanged();

    void setPositionScale(float f);
    void setTransformFlags(int flags);
    void removeTransformFlags(int flags);

protected slots:
    void regenHisto();
    void colorHisto();
    void transformHisto();
    void initializeGL() override;

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent* event) override;

    QSpinBox *m_Threshold, *m_Scale;
    QComboBox *m_Type;
    QCheckBox *m_Overlap;
    QCheckBox *m_Color;
    QCheckBox *m_Antialias;

    GLfloat* m_Vertices;
    GLfloat* m_Colors;

    int *m_Histogram;
    const quint8 *m_Data;
    qsizetype m_Size;
    int m_Flags;

    int m_VertexCount;

    int m_MouseX;
    int m_MouseY;

    float m_AngleX;
    float m_AngleY;
    float m_ScaleX;
    float m_ScaleY;
    float m_ScaleZ;
};

#endif
