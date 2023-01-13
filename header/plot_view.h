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

#ifndef _PLOT_VIEW_H_
#define _PLOT_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class CPlotView : public QLabel {
Q_OBJECT
public:
    explicit CPlotView(QWidget *p = nullptr);
    ~CPlotView() override = default;

public slots:
    void setImage(int ind, QImage &img);
    void setData(const float *bin, long len, bool normalize = true);
    void setData(int ind, const float *bin, long len, bool normalize = true);
    void enableSelection(bool);

protected slots:

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void update_pix();

    float m_UpperBandPos, m_LowerBandPos;
    int m_MousePosX, m_MousePosY;
    int m_ImageIndex;
    enum class allow_selection_ {
        NONE,
        M1_MOVING,
        M2_MOVING,
        M12_MOVING
    } m_SelectionType;

    QImage m_Images[2];
    QPixmap m_Pixmap;
    bool m_AllowSelection;

signals:
    void rangeSelected(float, float);
};

#endif
