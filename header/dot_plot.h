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

#ifndef _DOTPLOT_H_
#define _DOTPLOT_H_

#include <vector>

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;

class CDotPlot : public QLabel {
Q_OBJECT
public:
    explicit CDotPlot(QWidget *p = nullptr);
    ~CDotPlot() override;

public slots:
    void setData(const unsigned char *dat, long n);
    void parametersChanged();

protected slots:
    void setImage(QImage &img);
    void advanceMat(int bs, const std::vector<std::pair<int, int> > &rand);
    void regenImage();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void update_pix();

    QSpinBox *m_Offset1, *m_Offset2, *m_Width, *m_MaxSamples;
    const unsigned char *m_Data;
    long m_Size;

    int *m_Material;
    int m_MaterialMaxSize;
    int m_MaterialSize;

    int m_PointsIndex;
    std::vector<std::pair<int, int> > m_Points;

    QImage m_Image;
    QPixmap m_Pixmap;
};

#endif
