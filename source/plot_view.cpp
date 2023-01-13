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

#include <algorithm>
#include <QtGui>

#include "plot_view.h"

using std::min;
using std::max;

CPlotView::CPlotView(QWidget *p)
        : QLabel(p),
          m_UpperBandPos(0.), m_LowerBandPos(1.), m_MousePosX(-1), m_MousePosY(-1), m_ImageIndex(0), m_SelectionType(allow_selection_::NONE), m_AllowSelection(true) {
}

void CPlotView::enableSelection(bool v) {
    m_AllowSelection = v;
    update();
}

void CPlotView::setImage(int ind, QImage &img) {
    m_Images[ind] = img;

    update_pix();

    update();
}

void CPlotView::setData(const float *dat, long len, bool normalize) {
    m_ImageIndex = 0;
    setData(0, dat, len, normalize);
}

void CPlotView::setData(int ind, const float *dat, long len, bool normalize) {
    int w = width();
    int h = height();

    float mn = 0.;
    float mx = 1.;
    if (normalize) {
        mn = 99999999.;
        mx = -99999999.;
        for (int i = 0; i < len; i++) {
            mn = min(mn, dat[i]);
            mx = max(mx, dat[i]);
        }
        if (mn == mx) {
            mn -= .5;
            mx += .5;
        }
    }

    {
        QImage img(w, h, QImage::Format_RGB32);
        img.fill(0);

        auto acc = new float[h];
        memset(acc, 0, h * sizeof(float));
        auto cnt = new int[h];
        memset(cnt, 0, h * sizeof(int));

        for (int i = 0; i < len; i++) {
            float v = dat[i];
            int ind2 = int((i / float(len)) * (h - 1) + .5);
            acc[ind2] += (v - mn) / (mx - mn);
            cnt[ind2]++;
        }

        auto p = (unsigned int *) img.bits();
        int px = -1;
        int pc = -1;
        for (int i = 0; i < h; i++) {
            int x = px;
            int c = pc;
            if (cnt[i] == 0 && px == -1) continue;
            if (cnt[i] > 0) {
                float na = acc[i] / cnt[i];
                x = int(na * (w - 4) + .5) + 2; // slight offset so not to interfere with border
                px = x;
                c = 20 + int(na * (255 - 20));
                pc = c;
            }

            quint8 r = (c > 127) ? (c * 2) : (0);
            quint8 g = (c <= 127) ? (255 - c * 2) : (0);
            quint8 b = (c > 127) ? (255 - c * 2) : (c * 2);

            unsigned int v = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
            p[i * w + x] = v;
        }

        delete[] acc;
        delete[] cnt;

        setImage(ind, img);
    }
}

void CPlotView::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    if (m_AllowSelection) {
        int ry1 = m_UpperBandPos * height();
        int ry2 = m_LowerBandPos * height();

        QBrush brush(QColor(128, 64, 64, 128 + 32));
        QPen pen(brush, 5.5, Qt::SolidLine, Qt::FlatCap);
        p.setPen(pen);
        p.drawLine(0 + 3, ry1, width() - 1 - 3, ry1);
        p.drawLine(0 + 3, ry2, width() - 1 - 3, ry2);
    }

    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void CPlotView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void CPlotView::update_pix() {
    if (m_Images[m_ImageIndex].isNull()) return;

    int vw = width();
    int vh = height();
    m_Pixmap = QPixmap::fromImage(m_Images[m_ImageIndex]).scaled(vw, vh/*, Qt::KeepAspectRatio*/);
    setPixmap(m_Pixmap);
}

void CPlotView::mousePressEvent(QMouseEvent *e) {
    e->accept();

    if (!m_AllowSelection) return;
    if (e->button() != Qt::LeftButton) return;

    int x = e->pos().x();
    int y = e->pos().y();

    if (x < 0) x = 0;
    if (x > width() - 1) x = width() - 1;
    if (y < 0) y = 0;
    if (y > height() - 1) y = height() - 1;

    float yp = y / float(height());

    if (yp > m_UpperBandPos && (yp - m_UpperBandPos) < .01) {
        m_SelectionType = allow_selection_::M1_MOVING;
    } else if (yp < m_LowerBandPos && (m_LowerBandPos - yp) < .01) {
        m_SelectionType = allow_selection_::M2_MOVING;
    } else if (m_UpperBandPos < yp && yp < m_LowerBandPos) {
        m_SelectionType = allow_selection_::M12_MOVING;
    } else {
        m_SelectionType = allow_selection_::NONE;
    }

    m_MousePosX = x;
    m_MousePosY = y;
}

void CPlotView::mouseMoveEvent(QMouseEvent *e) {
    e->accept();

    if (m_SelectionType == allow_selection_::NONE) return;

    int x = e->pos().x();
    int y = e->pos().y();

    if (x < 0) x = 0;
    if (x > width() - 1) x = width() - 1;
    if (y < 0) y = 0;
    if (y > height() - 1) y = height() - 1;

    if (y == m_MousePosY) return;

    int h = height();

    float m1 = m_UpperBandPos;
    float m2 = m_LowerBandPos;
    if (m_SelectionType == allow_selection_::M1_MOVING) {
        m1 = y / float(h);
    } else if (m_SelectionType == allow_selection_::M2_MOVING) {
        m2 = y / float(h);
    } else if (m_SelectionType == allow_selection_::M12_MOVING) {
        float dy = (y - m_MousePosY) / float(h);
        m1 += dy;
        m2 += dy;
    }
    if (m1 >= m_LowerBandPos - .01) m1 = m_LowerBandPos - .01;
    if (m1 < 0.) m1 = 0.;
    if (m2 <= m_UpperBandPos + .01) m2 = m_UpperBandPos + .01;
    if (m2 > 1.) m2 = 1.;

    m_UpperBandPos = m1;
    m_LowerBandPos = m2;

    m_MousePosX = x;
    m_MousePosY = y;

    update();

    emit(rangeSelected(m_UpperBandPos, m_LowerBandPos));
}

void CPlotView::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() == Qt::RightButton) {
        m_ImageIndex = (m_ImageIndex + 1) % 2;
        update_pix();
        update();
    }

    if (e->button() != Qt::LeftButton) return;

//  int x = e->pos().x();
//  int y = e->pos().y();
//
//  if (x < 0) x = 0;
//  if (x > width() - 1) x = width() - 1;
//  if (y < 0) y = 0;
//  if (y > height() - 1) y = height() - 1;

    m_MousePosX = -1;
    m_MousePosY = -1;
    m_SelectionType = allow_selection_::NONE;
}
