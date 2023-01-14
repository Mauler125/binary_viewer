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

#ifndef _MAIN_APP_H_
#define _MAIN_APP_H_

#define NAMEOF(s) #s

#include <QDialog>

class COverallView;
class CHistogram2D;
class CImageView;
class CHexView;
class CDotPlot;
class CHistogram3D;
class CPlotView;
class QComboBox;
class QLabel;

class CMain : public QDialog {
Q_OBJECT
public:
    explicit CMain(QWidget *p = nullptr);
    ~CMain() override;
    bool loadFile(const QString &filename);
    bool loadFiles(const QStringList &filenames);
    static bool loadStyle(QString s);

public slots:
    void reject() override;

private slots:
	void toggleFullScreen();
	void toggleDarkMode();
	void toggleLightMode();

protected slots:
    void quit();
    void rangeSelected(float, float);
    void switchView(int);

    bool nextFile();
    bool prevFile();
    void loadFile();

protected:
    //    void updatePositions(bool resized = false);
    bool eventFilter(QObject* object, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void updateViews(bool update_iv1 = true, bool optimize = false);

    QComboBox *m_CurrentView;

    COverallView *m_OverallPrimary;
    COverallView *m_OverallZoomed;
    CPlotView *m_PlotView;
    CHistogram2D *m_Histogram2D;
    CHexView *m_HexView;
    CImageView *m_ImageView;
    CDotPlot *m_DotPlot;
    CHistogram3D *m_Histogram3D;
    QLabel *m_Filename;

    quint8* m_Data;
    qsizetype m_Size;

    qsizetype m_Start;
    qsizetype m_End;

    int m_CurrentFile;

    bool m_Initialized;
    bool m_DoneFlag;
    bool m_ReturnToMaximized;
    bool m_IsResizing;

    QStringList m_FileList;
    std::vector<QWidget*> m_Views;
};

inline QString g_currentfile;

#endif
