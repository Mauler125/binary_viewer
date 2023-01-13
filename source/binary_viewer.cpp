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
#include <QGridLayout>
#include <QComboBox>
#include <QScrollBar>

#include "binary_viewer.h"


CHexLogic::CHexLogic(QWidget *p)
        : QWidget(p),
          m_Data(nullptr), m_Size(0), m_Offset(0) {
}

int CHexLogic::rowHeight() const {
    QFontMetrics fm(m_Font);

    int fh = fm.height();

    return fh;
}

int CHexLogic::columnStart(int c, int fw) const {
    int x = 0;

    if (c < 0) {
        c = 5;
    }

    if (c >= 0) {
        x += fw;
    }
    if (c >= 1) {
        x += (2 + 1 + 4 + 1 + 4) * fw;
        x += 4 * fw;
    }
    if (c >= 2) {
        x += int(1.2 * fw + 2 * fw) * 16 + 2 * fw;
        x += 4 * fw;
    }
    if (c >= 3) {
        x += int(1.2 * fw + 1 * fw) * 16 + 2 * fw;
    }
    if (c >= 4) {
        x += fw;
    }

    return x;
}

void CHexLogic::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);

    QPainter p(this);

    int w = width();
    int h = height();

    p.setFont(m_Font);
    QFontMetrics fm(m_Font);

    int fh = fm.height();
    int fw = fm.maxWidth();

    int nvis_rows = h / fh;

    QPen default_pen = p.pen();

    for (int i = 0; i < nvis_rows; i++) {
        int x = columnStart(0, fw);
        int y = (i + 1) * fh;

        long pos = (m_Offset + i) * 16;

        QString s1;
        s1 = QString("0x %1 %2").arg((pos >> 16) & 0xffff, 4, 16, QChar('0')).arg(pos & 0xffff, 4, 16, QChar('0'));

        p.setPen(default_pen);
        p.drawText(x, y, s1);

        x = columnStart(1, fw);

        for (int j = 0; j < 16; j++) {
            if (pos + j >= m_Size) break;

            if (j > 0) x += 1.2 * fw + 2 * fw;
            if (j == 16 / 2) x += 2 * fw;

            unsigned char c = m_Data[pos + j];

            int r, g, b;
            if (c == 0x00) {
                r = 0x55;
                g = 0x55;
                b = 0x55;
            } else if (0x00 < c && c <= 0x1f) {
                r = 0x60;
                g = 0x60;
                b = 0xf0;
            } else if (0x1f < c && c <= 0x7f) {
                r = 0x00;
                g = 0xf0;
                b = 0x00;
            } else if (0x7f < c && c < 0xff) {
                r = 0xf0;
                g = 0x00;
                b = 0x00;
            } else if (c == 0xff) {
                r = 0xff;
                g = 0xff;
                b = 0xff;
            }
            unsigned int v = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
            p.setPen(QPen(QRgb(v)));

            QString s2;
            s2 = QString("%1").arg(c, 2, 16, QChar('0'));
            p.drawText(x, y, s2);
        }

        x = columnStart(2, fw);

        p.setPen(default_pen);
        for (int j = 0; j < 16; j++) {
            if (pos + j >= m_Size) break;

            if (j > 0) x += 1.2 * fw + 1 * fw;
            if (j == 16 / 2) x += 2 * fw;

            unsigned char c = m_Data[pos + j];

            if (!(0x20 <= c && c <= 0x7e)) {
                p.setPen(QPen(0xff606060));
                c = '.';
            } else {
                p.setPen(default_pen);
            }

            QString s2;
            s2 = QString("%1").arg(char(c));
            p.drawText(x, y, s2);
        }

//        x = px + int(1.2 * fw + 1 * fw) * 16 + 2 * fw;
    }

    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void CHexLogic::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);

    QFont font("Courier New");
    for (int i = 48; i > 4; i--) {
        font = QFont("Courier New", i);
        QFontMetrics fm(font);
        int fw = fm.maxWidth();
        int mw = columnStart(-1, fw);
        if (mw < width()) break;
    }
    m_Font = font;
}

void CHexLogic::setData(const unsigned char *dat, long n) {
    m_Data = dat;
    m_Size = n;
    m_Offset = 0;
    update();
}

void CHexLogic::setStart(int off) {
    m_Offset = off;
    update();
}


CHexView::CHexView(QWidget *p)
        : QWidget(p)
//          hist_(nullptr), dat_(nullptr), dat_n_(0)
{
    auto layout = new QHBoxLayout(this);

    m_HexLogic = new CHexLogic(this);
    m_ScrollBar = new QScrollBar(this);

    layout->addWidget(m_HexLogic);
    layout->addWidget(m_ScrollBar);

    m_ScrollBar->setRange(0, 0);
    m_ScrollBar->setFocus();
    m_ScrollBar->setFocusPolicy(Qt::StrongFocus);

    connect(m_ScrollBar, SIGNAL(valueChanged(int)), m_HexLogic, SLOT(setStart(int)));

    setLayout(layout);
}

CHexView::~CHexView() {
}

void CHexView::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);
}

void CHexView::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);

    int nvis_rows = height() / m_HexLogic->rowHeight();
    int page_step = std::max(16, nvis_rows - 2);
    m_ScrollBar->setRange(0, int(ceil(m_Size / 16.)) - nvis_rows + 1);
    m_ScrollBar->setPageStep(page_step);
}

void CHexView::setData(const unsigned char *dat, long n) {
    m_Data = dat;
    m_Size = n;

    m_HexLogic->setData(dat, n);
    int nvis_rows = height() / m_HexLogic->rowHeight();
    m_ScrollBar->setRange(0, int(ceil(m_Size / 16.)) - nvis_rows + 1);
}

void CHexView::setStart(int s) {
    m_ScrollBar->setValue(s);
}

void CHexView::enterEvent(QEvent *e) {
    QWidget::enterEvent(e);
    m_ScrollBar->setFocus();
}

void CHexView::wheelEvent(QWheelEvent *e) {
    m_ScrollBar->event(e);
    e->accept();
}
