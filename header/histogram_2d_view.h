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

#ifndef _HISTOGRAM_2D_VIEW_
#define _HISTOGRAM_2D_VIEW_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;
class QComboBox;

class CHistogram2D : public QLabel {
Q_OBJECT
public:
    explicit CHistogram2D(QWidget *p = nullptr);
    ~CHistogram2D() override;

public slots:
    void setData(const unsigned char *dat, long n);
    void parametersChanged();

protected slots:
    void setImage(QImage &img);
    void regenHisto();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void updatePixmap();

    QSpinBox *m_Threshold, *m_Scale;
    QComboBox *m_Type;
    int *m_Histogram;
    const unsigned char *m_Data;
    long m_Size;

    QImage m_Image;
    QPixmap m_Pixmap;

signals:
    void rangeSelected(float, float);
};

#endif
