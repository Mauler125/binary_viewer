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

#ifndef _BINARY_VIEWER_
#define _BINARY_VIEWER_

#include <QWidget>

class CHexLogic;
class QScrollBar;

class CHexLogic : public QWidget {
Q_OBJECT
public:
    explicit CHexLogic(QWidget *p = nullptr);
    ~CHexLogic() override = default;

    int rowHeight() const;

public slots:
    void setData(const unsigned char *dat, long n);
    void setStart(int);

protected slots:

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

    int columnStart(int c, int fw) const;

    const unsigned char *m_Data;
    long m_Size;
    int m_Offset;
    QFont m_Font;
};

class CHexView : public QWidget {
Q_OBJECT
public:
    explicit CHexView(QWidget *p = nullptr);
    ~CHexView() override;

public slots:
    void setData(const unsigned char *dat, long n);
    void setStart(int);

protected slots:

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void enterEvent(QEvent *) override;
    void wheelEvent(QWheelEvent *) override;

    CHexLogic *m_HexLogic;
    QScrollBar *m_ScrollBar;

    const unsigned char *m_Data;
    long m_Size;
};

#endif
