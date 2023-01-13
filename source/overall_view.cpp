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

#include <QtGui>

#include "hilbert.h"
#include "overall_view.h"

using std::min;

COverallView::COverallView(QWidget *p)
        : QLabel(p),
          m_UpperBandPos(0.), m_LowerBandPos(1.), m_MousePosX(-1), m_MousePosY(-1), m_SelectionType(allow_selection_::NONE),
          m_AllowSelection(true),
          m_UseByteClasses(true),
          m_UseHilbertCurve(true),
          m_Data(nullptr), m_Size(0) {
}

void COverallView::enableSelection(bool v) {
    m_AllowSelection = v;
    update();
}

void COverallView::enableByteClasses(bool v) {
    m_UseByteClasses = v;
    update();
}

void COverallView::enableHilbertCurve(bool v) {
    m_UseHilbertCurve = v;
    update();
}

void COverallView::setImage(QImage &img) {
    m_Image = img;

    update_pix();

    update();
}

void COverallView::set_data(const unsigned char *dat, long len, bool reset_selection) {
    m_Data = dat;
    m_Size = len;

    if (reset_selection) {
        m_UpperBandPos = 0.;
        m_LowerBandPos = 1.;
    }

    int w = width();
    int h = height();
    int wh = w * h;
    int sf = len / wh + 1;

    int img_w = w, img_h = len / sf / w + 1;
    QImage img(img_w, img_h, QImage::Format_RGB32);
    printf("%d %d   %d %d\n", w, h, img_w, img_h);
    img.fill(0);

    curve_t hilbert;
    int h_ind = 0;
    if (m_UseHilbertCurve) gilbert2d(img_w, img_h, hilbert);

    auto p = (unsigned int *) img.bits();

    for (int i = 0; i < len;) {
        int r = 0, g = 0, b = 0;

        if (!m_UseByteClasses) {
            int cn = 0;
            int j;
            for (j = 0; i < len && j < sf; i++, j++) {
                cn += dat[i];
            }
            r = 20;
            g = cn / j;
            b = 20;
        } else {
            int j;
            for (j = 0; i < len && j < sf; i++, j++) {
                unsigned char c = dat[i];
                if (c == 0x00) {
                    r += 0x00;
                    g += 0x00;
                    b += 0x00;
                } else if (0x00 < c && c <= 0x1f) {
                    r += 0x00;
                    g += 0x00;
                    b += 0xf0;
                } else if (0x1f < c && c <= 0x7f) {
                    r += 0x00;
                    g += 0xf0;
                    b += 0x00;
                } else if (0x7f < c && c < 0xff) {
                    r += 0xf0;
                    g += 0x00;
                    b += 0x00;
                } else if (c == 0xff) {
                    r += 0xff;
                    g += 0xff;
                    b += 0xff;
                }
            }

            r /= j;
            g /= j;
            b /= j;
        }

        r = min(255, r) & 0xff;
        g = min(255, g) & 0xff;
        b = min(255, b) & 0xff;

        unsigned int v = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);

        if (!m_UseHilbertCurve) {
            *p++ = v;
        } else {
            if (h_ind >= hilbert.size()) abort();

            int x = hilbert[h_ind].first;
            int y = hilbert[h_ind++].second;
            int ind = y * w + x;
            if (ind < wh) {
                p[ind] = v;
            } else {
                abort();
            }
        }
    }

    img = img.scaled(size());
    setImage(img);
}

void COverallView::paintEvent(QPaintEvent *e) {
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

    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void COverallView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void COverallView::update_pix() {
    if (m_Image.isNull()) return;

    int vw = width();
    int vh = height();
    m_Pixmap = QPixmap::fromImage(m_Image).scaled(vw, vh/*, Qt::KeepAspectRatio*/);
    setPixmap(m_Pixmap);
    printf("%d %d   %d %d   %d %d\n", vw, vh, m_Image.width(), m_Image.height(), width(), height());
}

// Gray code related functions are from https://en.wikipedia.org/wiki/Gray_code
static unsigned int BinaryToGray(unsigned int num) {
    return num ^ (num >> 1);
}

static unsigned int GrayToBinary(unsigned int num) {
    unsigned int mask = num >> 1;
    while (mask != 0) {
        num = num ^ mask;
        mask = mask >> 1;
    }
    return num;
}

void COverallView::mousePressEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() == Qt::RightButton) {
        unsigned char v = (m_UseByteClasses ? 0x02 : 0x00) | (m_UseHilbertCurve ? 0x01 : 0x00);
        v = BinaryToGray((GrayToBinary(v) + 1) & 0x03);
        m_UseByteClasses = v & 0x02;
        m_UseHilbertCurve = v & 0x01;
        set_data(m_Data, m_Size, false);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        if (!m_AllowSelection) return;

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
}

void COverallView::mouseMoveEvent(QMouseEvent *e) {
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

void COverallView::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() != Qt::LeftButton) return;

//    int x = e->pos().x();
//    int y = e->pos().y();
//
//    if (x < 0) x = 0;
//    if (x > width() - 1) x = width() - 1;
//    if (y < 0) y = 0;
//    if (y > height() - 1) y = height() - 1;

    m_MousePosX = -1;
    m_MousePosY = -1;
    m_SelectionType = allow_selection_::NONE;
}
