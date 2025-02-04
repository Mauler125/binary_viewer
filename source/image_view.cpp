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
#include <QSpinBox>
#include <QComboBox>

#include "image_view.h"
#include "bayer.h"


CImageView::CImageView(QWidget *p)
        : QLabel(p),
          m_Data(nullptr), m_Size(0), m_Inverted(true) {
    {
        auto layout = new QGridLayout(this);
        {
            auto l = new QLabel("Offset (B)", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 0, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            m_Offset = sb;
            layout->addWidget(sb, 0, 1);
        }
        {
            auto l = new QLabel("Width", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 1, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(512);
            m_Width = sb;
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
            cb->addItem("RGB 8");
            cb->addItem("RGB 12");
            cb->addItem("RGB 16");
            cb->addItem("RGBA 8");
            cb->addItem("RGBA 12");
            cb->addItem("RGBA 16");
            cb->addItem("BGR 8");
            cb->addItem("BGR 12");
            cb->addItem("BGR 16");
            cb->addItem("BGRA 8");
            cb->addItem("BGRA 12");
            cb->addItem("BGRA 16");
            cb->addItem("Grey 8");
            cb->addItem("Grey 12");
            cb->addItem("Grey 16");
            cb->addItem("Bayer 8 - 0: 0 1 2 3");
            cb->addItem("Bayer 8 - 1: 0 1 3 2");
            cb->addItem("Bayer 8 - 2: 0 2 1 3");
            cb->addItem("Bayer 8 - 3: 0 2 3 1");
            cb->addItem("Bayer 8 - 4: 0 3 1 2");
            cb->addItem("Bayer 8 - 5: 0 3 2 1");
            cb->addItem("Bayer 8 - 6: 1 0 2 3");
            cb->addItem("Bayer 8 - 7: 1 0 3 2");
            cb->addItem("Bayer 8 - 8: 1 2 0 3");
            cb->addItem("Bayer 8 - 9: 1 2 3 0");
            cb->addItem("Bayer 8 - 10: 1 3 0 2");
            cb->addItem("Bayer 8 - 11: 1 3 2 0");
            cb->addItem("Bayer 8 - 12: 2 0 1 3");
            cb->addItem("Bayer 8 - 13: 2 0 3 1");
            cb->addItem("Bayer 8 - 14: 2 1 0 3");
            cb->addItem("Bayer 8 - 15: 2 1 3 0");
            cb->addItem("Bayer 8 - 16: 2 3 0 1");
            cb->addItem("Bayer 8 - 17: 2 3 1 0");
            cb->addItem("Bayer 8 - 18: 3 0 1 2");
            cb->addItem("Bayer 8 - 19: 3 0 2 1");
            cb->addItem("Bayer 8 - 20: 3 1 0 2");
            cb->addItem("Bayer 8 - 21: 3 1 2 0");
            cb->addItem("Bayer 8 - 22: 3 2 0 1");
            cb->addItem("Bayer 8 - 23: 3 2 1 0");
            cb->setCurrentIndex(0);
            cb->setEditable(false);
            cb->setFixedWidth(cb->width() * 1.5);
            m_Type = cb;
            layout->addWidget(cb, 2, 1);
        }

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(3, 1);

        QObject::connect(m_Offset, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Width, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(parametersChanged()));
    }
}

void CImageView::setImage(QImage &img) {
    m_Image = img;

    updatePixmap();

    update();
}

void CImageView::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void CImageView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    updatePixmap();
}

void CImageView::updatePixmap() {
    if (m_Image.isNull()) return;

    int vw = width();
    int vh = height();
    m_Pixmap = QPixmap::fromImage(m_Image).scaled(vw, vh/*, Qt::KeepAspectRatio*/);
    setPixmap(m_Pixmap);
}


void CImageView::setData(const quint8 *dat, qsizetype n) {
    m_Data = dat;
    m_Size = n;

    regenImage();
}

void CImageView::regenImage() {
    parametersChanged();
}

void CImageView::parametersChanged() {
    int offset = m_Offset->value();
    int w = m_Width->value();

    dtype_t t;
    QString s = m_Type->currentText();
    if (s == "RGB 8") t = dtype_t::RGB8;
    else if (s == "RGB 12") t = dtype_t::RGB12;
    else if (s == "RGB 16") t = dtype_t::RGB16;
    else if (s == "RGBA 8") t = dtype_t::RGBA8;
    else if (s == "RGBA 12") t = dtype_t::RGBA12;
    else if (s == "RGBA 16") t = dtype_t::RGBA16;
    else if (s == "BGR 8") t = dtype_t::BGR8;
    else if (s == "BGR 12") t = dtype_t::BGR12;
    else if (s == "BGR 16") t = dtype_t::BGR16;
    else if (s == "BGRA 8") t = dtype_t::BGRA8;
    else if (s == "BGRA 12") t = dtype_t::BGRA12;
    else if (s == "BGRA 16") t = dtype_t::BGRA16;
    else if (s == "Grey 8") t = dtype_t::GREY8;
    else if (s == "Grey 12") t = dtype_t::GREY12;
    else if (s == "Grey 16") t = dtype_t::GREY16;
    else if (s == "Bayer 8 - 0: 0 1 2 3") t = dtype_t::BAYER8_0;
    else if (s == "Bayer 8 - 1: 0 1 3 2") t = dtype_t::BAYER8_1;
    else if (s == "Bayer 8 - 2: 0 2 1 3") t = dtype_t::BAYER8_2;
    else if (s == "Bayer 8 - 3: 0 2 3 1") t = dtype_t::BAYER8_3;
    else if (s == "Bayer 8 - 4: 0 3 1 2") t = dtype_t::BAYER8_4;
    else if (s == "Bayer 8 - 5: 0 3 2 1") t = dtype_t::BAYER8_5;
    else if (s == "Bayer 8 - 6: 1 0 2 3") t = dtype_t::BAYER8_6;
    else if (s == "Bayer 8 - 7: 1 0 3 2") t = dtype_t::BAYER8_7;
    else if (s == "Bayer 8 - 8: 1 2 0 3") t = dtype_t::BAYER8_8;
    else if (s == "Bayer 8 - 9: 1 2 3 0") t = dtype_t::BAYER8_9;
    else if (s == "Bayer 8 - 10: 1 3 0 2") t = dtype_t::BAYER8_10;
    else if (s == "Bayer 8 - 11: 1 3 2 0") t = dtype_t::BAYER8_11;
    else if (s == "Bayer 8 - 12: 2 0 1 3") t = dtype_t::BAYER8_12;
    else if (s == "Bayer 8 - 13: 2 0 3 1") t = dtype_t::BAYER8_13;
    else if (s == "Bayer 8 - 14: 2 1 0 3") t = dtype_t::BAYER8_14;
    else if (s == "Bayer 8 - 15: 2 1 3 0") t = dtype_t::BAYER8_15;
    else if (s == "Bayer 8 - 16: 2 3 0 1") t = dtype_t::BAYER8_16;
    else if (s == "Bayer 8 - 17: 2 3 1 0") t = dtype_t::BAYER8_17;
    else if (s == "Bayer 8 - 18: 3 0 1 2") t = dtype_t::BAYER8_18;
    else if (s == "Bayer 8 - 19: 3 0 2 1") t = dtype_t::BAYER8_19;
    else if (s == "Bayer 8 - 20: 3 1 0 2") t = dtype_t::BAYER8_20;
    else if (s == "Bayer 8 - 21: 3 1 2 0") t = dtype_t::BAYER8_21;
    else if (s == "Bayer 8 - 22: 3 2 0 1") t = dtype_t::BAYER8_22;
    else if (s == "Bayer 8 - 23: 3 2 1 0") t = dtype_t::BAYER8_23;
    else t = dtype_t::NONE;

    QImage img;

    switch (t) {
        case dtype_t::RGB8: {
            auto dat_u8 = m_Data + offset;
            int n = (m_Size - offset) / 1 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 3 + 0];
                unsigned char g = dat_u8[i * 3 + 1];
                unsigned char b = dat_u8[i * 3 + 2];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::RGB12: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 0] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 2] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::RGB16: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 0] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 2] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::RGBA8: {
            auto dat_u8 = (const unsigned char *) (m_Data + offset);
            int n = (m_Size - offset) / 1 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 4 + 0];
                unsigned char g = dat_u8[i * 4 + 1];
                unsigned char b = dat_u8[i * 4 + 2];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::RGBA12: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 0] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 2] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::RGBA16: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u16[i * 4 + 0] >> 8;
                unsigned char g = dat_u16[i * 4 + 1] >> 8;
                unsigned char b = dat_u16[i * 4 + 2] >> 8;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGR8: {
            auto dat_u8 = (const unsigned char *) (m_Data + offset);
            int n = (m_Size - offset) / 1 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 3 + 2];
                unsigned char g = dat_u8[i * 3 + 1];
                unsigned char b = dat_u8[i * 3 + 0];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGR12: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 2] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 0] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGR16: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 2] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 0] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGRA8: {
            auto dat_u8 = (const unsigned char *) (m_Data + offset);
            int n = (m_Size - offset) / 1 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 4 + 2];
                unsigned char g = dat_u8[i * 4 + 1];
                unsigned char b = dat_u8[i * 4 + 0];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGRA12: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 2] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 0] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BGRA16: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 2] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 0] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::GREY8: {
            auto dat_u8 = (const unsigned char *) (m_Data + offset);
            int n = (m_Size - offset) / 1;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = dat_u8[i];
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::GREY12: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = (dat_u16[i] >> 4) & 0xff;
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::GREY16: {
            auto dat_u16 = (const unsigned short *) (m_Data + offset);
            int n = (m_Size - offset) / 2;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = (dat_u16[i] >> 8) & 0xff;
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case dtype_t::BAYER8_0:
        case dtype_t::BAYER8_1:
        case dtype_t::BAYER8_2:
        case dtype_t::BAYER8_3:
        case dtype_t::BAYER8_4:
        case dtype_t::BAYER8_5:
        case dtype_t::BAYER8_6:
        case dtype_t::BAYER8_7:
        case dtype_t::BAYER8_8:
        case dtype_t::BAYER8_9:
        case dtype_t::BAYER8_10:
        case dtype_t::BAYER8_11:
        case dtype_t::BAYER8_12:
        case dtype_t::BAYER8_13:
        case dtype_t::BAYER8_14:
        case dtype_t::BAYER8_15:
        case dtype_t::BAYER8_16:
        case dtype_t::BAYER8_17:
        case dtype_t::BAYER8_18:
        case dtype_t::BAYER8_19:
        case dtype_t::BAYER8_20:
        case dtype_t::BAYER8_21:
        case dtype_t::BAYER8_22:
        case dtype_t::BAYER8_23: {
            int h = m_Size / w + 1;
            //int bayer_n = w * h;

            auto dat_u8 = (const unsigned char *) (m_Data + offset);

            const unsigned char *bayer = dat_u8;
            auto rgb = new unsigned char[w * h * 3];
            int perm = 0;
            switch (t) {
                case dtype_t::BAYER8_0:
                    perm = 0;
                    break;
                case dtype_t::BAYER8_1:
                    perm = 1;
                    break;
                case dtype_t::BAYER8_2:
                    perm = 2;
                    break;
                case dtype_t::BAYER8_3:
                    perm = 3;
                    break;
                case dtype_t::BAYER8_4:
                    perm = 4;
                    break;
                case dtype_t::BAYER8_5:
                    perm = 5;
                    break;
                case dtype_t::BAYER8_6:
                    perm = 6;
                    break;
                case dtype_t::BAYER8_7:
                    perm = 7;
                    break;
                case dtype_t::BAYER8_8:
                    perm = 8;
                    break;
                case dtype_t::BAYER8_9:
                    perm = 9;
                    break;
                case dtype_t::BAYER8_10:
                    perm = 10;
                    break;
                case dtype_t::BAYER8_11:
                    perm = 11;
                    break;
                case dtype_t::BAYER8_12:
                    perm = 12;
                    break;
                case dtype_t::BAYER8_13:
                    perm = 13;
                    break;
                case dtype_t::BAYER8_14:
                    perm = 14;
                    break;
                case dtype_t::BAYER8_15:
                    perm = 15;
                    break;
                case dtype_t::BAYER8_16:
                    perm = 16;
                    break;
                case dtype_t::BAYER8_17:
                    perm = 17;
                    break;
                case dtype_t::BAYER8_18:
                    perm = 18;
                    break;
                case dtype_t::BAYER8_19:
                    perm = 19;
                    break;
                case dtype_t::BAYER8_20:
                    perm = 20;
                    break;
                case dtype_t::BAYER8_21:
                    perm = 21;
                    break;
                case dtype_t::BAYER8_22:
                    perm = 22;
                    break;
                case dtype_t::BAYER8_23:
                    perm = 23;
                    break;
            }
            bayerBG(bayer, h, w, perm, rgb);

            qsizetype n = (m_Size - offset) / 1;
            img = QImage(w, h, QImage::Format_RGB32);
            img.fill(0);
            auto p = (unsigned int *) img.bits();
            for (qsizetype i = 0; i < n; i++) {
                unsigned char r = rgb[i * 3 + 0];
                unsigned char g = rgb[i * 3 + 1];
                unsigned char b = rgb[i * 3 + 2];
                unsigned int v = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        default:
            abort();
    }

    if (m_Inverted) {
        img = img.mirrored(true);
    }
    setImage(img);
}
