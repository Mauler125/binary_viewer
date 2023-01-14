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

#ifndef _OVERALL_VIEW_H_
#define _OVERALL_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class COverallView : public QLabel {
Q_OBJECT
public:
    explicit COverallView(QWidget *p = nullptr);

    ~COverallView() override = default;

public slots:

    void setImage(QImage &img);
    void setData(const quint8 *bin, qsizetype len, bool reset_selection = true);

    void enableSelection(bool);
    void enableByteClasses(bool);
    void enableHilbertCurve(bool);

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
    enum class allow_selection_{
        NONE, M1_MOVING, M2_MOVING, M12_MOVING
    } m_SelectionType;

    bool m_AllowSelection;
    bool m_UseByteClasses;
    bool m_UseHilbertCurve;

    const quint8 *m_Data;
    qsizetype m_Size;

    QImage m_Image;
    QPixmap m_Pixmap;

signals:
    void rangeSelected(float, float);
};

#endif
