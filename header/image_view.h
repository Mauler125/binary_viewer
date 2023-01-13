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

#ifndef _IMAGE_VIEW_H_
#define _IMAGE_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;
class QComboBox;

class CImageView : public QLabel {
Q_OBJECT
public:
    explicit CImageView(QWidget *p = nullptr);
    ~CImageView() override = default;

public slots:
    void setData(const unsigned char *dat, long n);
    void parametersChanged();

protected slots:
    void setImage(QImage &img);
    void regenImage();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void updatePixmap();

    typedef enum class dtype_t {
        NONE, RGB8, RGB12, RGB16, RGBA8, RGBA12, RGBA16, BGR8, BGR12, BGR16, BGRA8, BGRA12, BGRA16, GREY8, GREY12, GREY16,
        BAYER8_0,
        BAYER8_1,
        BAYER8_2,
        BAYER8_3,
        BAYER8_4,
        BAYER8_5,
        BAYER8_6,
        BAYER8_7,
        BAYER8_8,
        BAYER8_9,
        BAYER8_10,
        BAYER8_11,
        BAYER8_12,
        BAYER8_13,
        BAYER8_14,
        BAYER8_15,
        BAYER8_16,
        BAYER8_17,
        BAYER8_18,
        BAYER8_19,
        BAYER8_20,
        BAYER8_21,
        BAYER8_22,
        BAYER8_23
    } dtype_t;

    QSpinBox *m_Offset, *m_Width;
    QComboBox *m_Type;
    const unsigned char *m_Data;
    long m_Size;
    bool m_Inverted;

    QImage m_Image;
    QPixmap m_Pixmap;
};

#endif
