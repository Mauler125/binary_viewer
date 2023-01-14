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

#include <vector>
#include <algorithm>

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include <stdlib.h>

#include "dot_plot.h"
#include "main_app.h"

using std::max;
using std::min;
using std::vector;
using std::shuffle;
using std::pair;
using std::make_pair;

#if defined(_WIN32)
// For MSVC x86, x64, and 32/64-bit ARM
//  MSVC lacks the random() function so we will wrap rand
//  to provide random values for these targets since this
//  program is not providing any cryptographic functions.
//  TODO consider replacing with <random> from C++ 11.
long random() { return rand(); }
#endif

CDotPlot::CDotPlot(QWidget *p)
        : QLabel(p),
          m_Data(nullptr), m_Size(0),
          m_Material(nullptr), m_MaterialMaxSize(0), m_MaterialSize(0),
          m_PointsIndex(0) {
    {
        auto layout = new QGridLayout(this);
        int r = 0;

        {
            auto l = new QLabel("Offset1 (B)", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            m_Offset1 = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Offset2 (B)", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            m_Offset2 = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Width", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10000);
            m_Width = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Max Samples", this);
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox(this);
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10);
            m_MaxSamples = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto pb = new QPushButton("Resample", this);
            pb->setFixedSize(pb->sizeHint());
            layout->addWidget(pb, r, 1);
            QObject::connect(pb, SIGNAL(clicked()), this, SLOT(parametersChanged()));
        }
        r++;

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(r, 1);

        QObject::connect(m_Offset1, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Offset2, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_Width, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
        QObject::connect(m_MaxSamples, SIGNAL(valueChanged(int)), this, SLOT(parametersChanged()));
    }
}

CDotPlot::~CDotPlot() {
    delete[] m_Material;
}

void CDotPlot::setImage(QImage &img) {
    m_Image = img;

    update_pix();

    update();
}

void CDotPlot::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void CDotPlot::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    int tmp = min(width(), height());
    if (tmp != m_MaterialMaxSize) {
        delete[] m_Material;
        m_MaterialMaxSize = tmp;
        m_MaterialSize = 0;
        m_Material = new int[m_MaterialMaxSize * m_MaterialMaxSize];
    }

    parametersChanged();
}

void CDotPlot::update_pix() {
    if (m_Image.isNull()) return;

    int vw = width();
    int vh = height();
    m_Pixmap = QPixmap::fromImage(m_Image).scaled(vw, vh/*, Qt::KeepAspectRatio*/);
    setPixmap(m_Pixmap);
}


void CDotPlot::setData(const quint8 *dat, qsizetype n) {
    m_Data = dat;
    m_Size = n;

    m_Offset1->setRange(0, m_Size);
    m_Offset2->setRange(0, m_Size);
    m_Width->setRange(1, m_Size);

    m_Width->setValue(m_Size);

    // parameters_changed() triggered by the previous setValue() call.
    // parameters_changed();
}

void CDotPlot::parametersChanged() {
    std::random_device rd;
    std::mt19937 g(rd());

    puts("called");

    qsizetype mdw = min(m_Size, (qsizetype)m_Width->value());
    int bs = int(mdw / m_MaterialMaxSize) + ((mdw % m_MaterialMaxSize) > 0 ? 1 : 0);
    m_MaterialSize = 0;

    if (m_Size > 0) {
        m_MaterialSize = min(int(mdw / bs), m_MaterialMaxSize);
        printf("Setting max to: '%d'\n", bs);
        m_MaxSamples->setMaximum(bs);
    }

    int mul = bs * m_MaterialSize;

    printf("%s(%lld) %s(%lld) %s(%d) %s(%d) %s(%d) %s(%d)\n",
        NAMEOF(m_Size), m_Size, 
        NAMEOF(mdw), mdw, 
        NAMEOF(m_MaterialMaxSize), m_MaterialMaxSize, 
        NAMEOF(bs), bs,
        NAMEOF(m_MaterialSize), m_MaterialSize,
        NAMEOF(mul), mul);

    memset(m_Material, 0, sizeof(m_Material[0]) * m_MaterialMaxSize * m_MaterialMaxSize);

    m_Points.clear();
    m_Points.reserve(m_MaterialSize * m_MaterialSize);
#if 1
    for (int i = 0; i < m_MaterialSize; i++) {
        for (int j = i; j < m_MaterialSize; j++) {
            m_Points.emplace_back(make_pair(i, j));
        }
    }
#else
    for (int i = 0; i < mat_n_; i++) {
        pts_.emplace_back(make_pair(i, mat_n_-i-1));
//        pts_.emplace_back(make_pair(i, i));
    }
#endif
    shuffle(m_Points.begin(), m_Points.end(), g);

    {
//        float sf = 1.f;
//        if (mwh < mdw) {
//            sf = mwh / float(mdw);
//        }
//
//        int xyn = 1 / sf + 1;

//        printf("min(width(), height()): %d mwh:%d mdw:%d dat_n:%d sf:%f xyn:%d", min(width(), height()), mwh, mdw, dat_n_, sf, xyn);


//        printf("pts_.size(): %d sf: %f\n", pts_.size(), sf);
        // pts_i_ is decremented in advance_mat()
        m_PointsIndex = m_Points.size();
        int ii = 0;
        // Precompute some random values for sampling
        std::vector<pair<int, int> > random;
        while (m_PointsIndex > 0) {
            if ((ii++ % 100) == 0) {
                {
                    int n = min(m_MaxSamples->value(), bs);
//            n = n * n;
                    random.clear();
                    random.reserve(n);
                    printf("Generating '%d' points in range [0, %d-1] along the diagonal axis\n", n, bs);
                    for (int tt = 0; tt < n; tt++) {
                        int a = rand() % bs;
                        random.emplace_back(make_pair(a, a));
                    }
                    int n2 = min(m_MaxSamples->value(), bs * bs - bs);
                    printf("Generating '%d' points in range [0, %d-1] off the diagonal axis\n", n2, bs);
                    for (int tt = 0; tt < n2;) {
                        int a = rand() % bs;
                        int b = rand() % bs;
                        if (a == b) continue;
                        random.emplace_back(make_pair(a, b));
                        tt++;
                    }
//            for (int tt = 0; tt < n; tt++) {
//                rand.emplace_back(make_pair(random() % bs, random() % bs));
//            }
                }
            }


//            random_shuffle(rand.begin(), rand.end());
            advanceMat(bs, random);
        }
    }
    regenImage();
}

void CDotPlot::advanceMat(int bs, const vector<pair<int, int> > &rand) {
    if (m_PointsIndex == 0 || m_Points.empty()) return;

    m_PointsIndex--;
    pair<int, int> pt = m_Points[m_PointsIndex];

    int x = pt.first;
    int y = pt.second;

    int xo = x * bs;
    int yo = y * bs;

    if (true) {
//        for (int tt = 0; tt < bs; tt++) {
//            int i = xo + tt;
//            int j = yo + tt;
//
//            if (dat_[i] == dat_[j]) {
//                int ii = y * mat_n_ + x;
//                int jj = x * mat_n_ + y;
//                if (0 <= ii && ii < mat_n_ * mat_n_) mat_[ii]++;
//                if (0 <= jj && jj < mat_n_ * mat_n_) mat_[jj]++;
//            }
////            printf("%d %d %d %d %d %d %d %d\n", xo, yo, tt, rand[tt], i, j, dat_[i], dat_[j]);
//        }
        for (int tt = 0; tt < rand.size(); tt++) {
            int i = xo + rand[tt].first;
            int j = yo + rand[tt].second;
//            int i = xo + (random() % bs);
//            int j = yo + (random() % bs);

            if (m_Data[i] == m_Data[j]) {
                int ii = y * m_MaterialSize + x;
                int jj = x * m_MaterialSize + y;
                if (0 <= ii && ii < m_MaterialSize * m_MaterialSize) m_Material[ii]++;
                if (0 <= jj && jj < m_MaterialSize * m_MaterialSize) m_Material[jj]++;
            }
//            printf("%d %d %d %d %d %d %d %d\n", xo, yo, tt, rand[tt], i, j, dat_[i], dat_[j]);
        }
    } else {
        for (int tt1 = 0; tt1 < bs; tt1++) {
            for (int tt2 = 0; tt2 < bs; tt2++) {
                int i = xo + tt1;
                int j = yo + tt2;

                if (m_Data[i] == m_Data[j]) {
                    int ii = y * m_MaterialSize + x;
                    int jj = x * m_MaterialSize + y;
                    if (0 <= ii && ii < m_MaterialSize * m_MaterialSize) m_Material[ii]++;
//                    else abort();
                    if (0 <= jj && jj < m_MaterialSize * m_MaterialSize) m_Material[jj]++;
//                    else abort();
                }
                printf("%d %d %d %d %d %d %u %u\n", xo, yo, tt1, tt2, i, j, m_Data[i], m_Data[j]);
            }
        }
    }
}

void CDotPlot::regenImage() {
    // Find the maximum value, ignoring the diagonal.
    // Could stop the search once m = max_samples_->value()
    int m = 0;
    for (int j = 0; j < m_MaterialSize; j++) {
        for (int i = 0; i < j; i++) {
            int k = j * m_MaterialSize + i;
            if (m < m_Material[k]) m = m_Material[k];
        }
        for (int i = j + 1; i < m_MaterialSize; i++) {
            int k = j * m_MaterialSize + i;
            if (m < m_Material[k]) m = m_Material[k];
        }
    }

    printf("max(1): %d\n", m);
    if (true) {
        // Brighten image
        m = max(1, int(m * .75));
        printf("max(2): %d\n", m);
    }

    QImage img(m_MaterialSize, m_MaterialSize, QImage::Format_RGB32);
    img.fill(0);
    auto p = (unsigned int *) img.bits();
    for (int i = 0; i < m_MaterialSize * m_MaterialSize; i++) {
        int c = min(255, int(m_Material[i] / float(m) * 255. + .5));
        unsigned char r = c;
        unsigned char g = c;
        unsigned char b = c;
        unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
        *p++ = v;
    }

    if (!g_currentfile.isEmpty())
    {
        QString imgFile = QString("%1.png").arg(g_currentfile);

        if (!img.save(imgFile)) {
            printf("Failed to save sample image: '%s'\n", imgFile.toLocal8Bit().constData());
        }
    }

//    long mdw = min(dat_n_, (long) width_->value());
//    int mwh = min(width(), height());
//    if (mwh > mdw) mwh = mdw;

    setImage(img);
}
