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
#include <QSpinBox>
#include <QComboBox>

#include "histogram_2d_view.h"
#include "histogram_calc.h"

using std::isnan;
using std::signbit;
using std::isinf;


CHistogram2D::CHistogram2D(QWidget *p)
        : QLabel(p),
          m_Histogram(nullptr), m_Data(nullptr), m_Size(0) {
    {
        auto layout = new QGridLayout(this);
        {
            auto l = new QLabel("Threshold", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 0, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(4);
            m_Threshold = sb;
            layout->addWidget(sb, 0, 1);
        }
        {
            auto l = new QLabel("Scale", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 1, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(100);
            m_Scale = sb;
            layout->addWidget(sb, 1, 1);
        }
        {
            auto l = new QLabel("Type", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 2, 0);
        }
        {
            auto cb = new QComboBox(this);
            cb->setFixedSize(cb->sizeHint());
            cb->addItem("U8");
            cb->addItem("U16");
            cb->addItem("U32");
            cb->addItem("U64");
            cb->addItem("F32");
            cb->addItem("F64");
            cb->setCurrentIndex(0);
            cb->setEditable(false);
            m_Type = cb;
            layout->addWidget(cb, 2, 1);
        }

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(3, 1);

        QObject::connect(m_Threshold, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Scale, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(regenHisto()));
    }
}

CHistogram2D::~CHistogram2D() {
    delete[] m_Histogram;
}

void CHistogram2D::setImage(QImage &img) {
    m_Image = img;

    updatePixmap();

    update();
}

void CHistogram2D::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void CHistogram2D::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    updatePixmap();
}

void CHistogram2D::updatePixmap() {
    if (m_Image.isNull()) return;

    int vw = width();
    int vh = height();
    m_Pixmap = QPixmap::fromImage(m_Image).scaled(vw, vh/*, Qt::KeepAspectRatio*/);
    setPixmap(m_Pixmap);
}

void CHistogram2D::setData(const quint8 *dat, qsizetype n) {
    m_Data = dat;
    m_Size = n;

    regenHisto();
}

void CHistogram2D::regenHisto() {
    delete[] m_Histogram;
    m_Histogram = nullptr;

    HistoDtype_t t = string_to_histo_dtype(m_Type->currentText().toStdString());
    m_Histogram = generate_histo_2d(m_Data, m_Size, t);

    parametersChanged();
}

void CHistogram2D::parametersChanged() {
    int thresh = m_Threshold->value();
    float scale_factor = m_Scale->value();

    QImage img(256, 256, QImage::Format_RGB32);
    img.fill(0);

    auto p = (unsigned int *) img.bits();

    for (int i = 0; i < 256 * 256; i++, p++) {
        if (m_Histogram[i] >= thresh) {
            float cc = m_Histogram[i] / scale_factor;
            cc += .2;
            if (cc > 1.) cc = 1.;
            int c = cc * 255 + .5;
            if (c < 0) c = 0;
            if (c > 255) c = 255;

            unsigned char r = 20;
            unsigned char g = c;
            unsigned char b = 20;
            unsigned int v = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);

            *p = v;
        }
    }

    setImage(img);

    update();
}
