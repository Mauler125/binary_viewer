/*
 * MIT License
 * 
 * Copyright (c) 2015, 2017 Kent A. Vander Velden
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QtGui>

#include "image_view.h"

ImageView::ImageView(QWidget *p)
        : QLabel(p),
          m1_(0.), m2_(1.), s_(none), allow_selection_(true) {
}

ImageView::~ImageView() {
}

void ImageView::enableSelection(bool v) {
    allow_selection_ = v;
    update();
}

void ImageView::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void ImageView::set_data(const unsigned char *dat, long len) {
    int w = width();
    int h = height();

    {
        int wh = w * h;

        if (len <= wh) {
            QImage img(w, len / w + 1, QImage::Format_RGB32);
            img.fill(0);

            unsigned int *p = (unsigned int *) img.bits();

            for (int i = 0; i < len; i++) {
                unsigned char c = dat[i];
                unsigned char r = 20;
                unsigned char g = c;
                unsigned char b = 20;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }

            img = img.scaled(size());
            setImage(img);
        } else {
            int sf = len / wh + 1;

            QImage img(w, len / sf / w + 1, QImage::Format_RGB32);
            img.fill(0);

            unsigned int *p = (unsigned int *) img.bits();

            for (int i = 0; i < len;) {
                unsigned char c;
                int cn = 0;
                int j;
                for (j = 0; i < len && j < sf; i++, j++) {
                    cn += dat[i];
                }
                c = cn / j;
                unsigned char r = 20;
                unsigned char g = c;
                unsigned char b = 20;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }

            img = img.scaled(size());
            setImage(img);
        }
    }
}

void ImageView::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    if (allow_selection_) {
        int ry1 = m1_ * height();
        int ry2 = m2_ * height();

        QBrush brush(QColor(128, 64, 64, 128 + 32));
        QPen pen(brush, 5, Qt::SolidLine, Qt::RoundCap);
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

void ImageView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void ImageView::update_pix() {
    int vw = width();
    int vh = height();
    pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
}

void ImageView::mousePressEvent(QMouseEvent *e) {
    e->accept();

    if (!allow_selection_) return;
    if (e->button() != Qt::LeftButton) return;

    int x = e->pos().x();
    int y = e->pos().y();

    if (x < 0) x = 0;
    if (x > width() - 1) x = width() - 1;
    if (y < 0) y = 0;
    if (y > height() - 1) y = height() - 1;

    float yp = y / float(height());

    if (yp > m1_ && (yp - m1_) < .01) {
        s_ = m1_moving;
    } else if (yp < m2_ && (m2_ - yp) < .01) {
        s_ = m2_moving;
    } else if (m1_ < yp && yp < m2_) {
        s_ = m12_moving;
    } else {
        s_ = none;
    }

    px_ = x;
    py_ = y;
}

void ImageView::mouseMoveEvent(QMouseEvent *e) {
    e->accept();

    if (s_ == none) return;

    int x = e->pos().x();
    int y = e->pos().y();

    if (x < 0) x = 0;
    if (x > width() - 1) x = width() - 1;
    if (y < 0) y = 0;
    if (y > height() - 1) y = height() - 1;

    if (y == py_) return;

    int h = height();

    float m1 = m1_;
    float m2 = m2_;
    if (s_ == m1_moving) {
        m1 = y / float(h);
    } else if (s_ == m2_moving) {
        m2 = y / float(h);
    } else if (s_ == m12_moving) {
        float dy = (y - py_) / float(h);
        m1 += dy;
        m2 += dy;
    }
    if (m1 >= m2_ - .01) m1 = m2_ - .01;
    if (m1 < 0.) m1 = 0.;
    if (m2 <= m1_ + .01) m2 = m1_ + .01;
    if (m2 > 1.) m2 = 1.;

    m1_ = m1;
    m2_ = m2;

    px_ = x;
    py_ = y;

    update();

    emit(rangeSelected(m1_, m2_));
}

void ImageView::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() != Qt::LeftButton) return;

//    int x = e->pos().x();
//    int y = e->pos().y();
//
//    if (x < 0) x = 0;
//    if (x > width() - 1) x = width() - 1;
//    if (y < 0) y = 0;
//    if (y > height() - 1) y = height() - 1;

    px_ = -1;
    py_ = -1;
    s_ = none;
}
