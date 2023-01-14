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

#include <cstdio>
#include <cstdlib>

#include <QtGui>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStyleFactory>
#include <QApplication>

#include "main_app.h"
#include "binary_viewer.h"
#include "overall_view.h"
#include "histogram_2d_view.h"
#include "image_view.h"
#include "dot_plot.h"
#include "histogram_3d_view.h"
#include "plot_view.h"
#include "histogram_calc.h"

static int scroller_w = 16 * 8;

void CMain::toggleFullScreen() {
	if (!isFullScreen()) {
		m_ReturnToMaximized = isMaximized();
	}
	if (isFullScreen() && !m_ReturnToMaximized) {
		showNormal();
	}
	else if (isFullScreen() && m_ReturnToMaximized) {
		showMaximized();
	}
	else {
		showFullScreen();
	}
}

void CMain::toggleLightMode()
{
    loadStyle(":/qstyle/light/style.qss");
    QSettings s(QString("settings/style.ini"), QSettings::IniFormat);
    s.setValue("theme/darkMode", "0");
}

void CMain::toggleDarkMode()
{
    loadStyle(":/qstyle/dark/style.qss");
    QSettings s(QString("settings/style.ini"), QSettings::IniFormat);
    s.setValue("theme/darkMode", "1");
}

CMain::CMain(QWidget *p)
        : QDialog(p), m_CurrentFile(-1), m_Data(nullptr), m_Size(0), m_Start(0), m_End(0) {
    m_DoneFlag = false;

    this->setSizeGripEnabled(true);
    this->setAcceptDrops(true);
    this->setMinimumHeight(300);

    auto top_layout = new QGridLayout(this);

    new QShortcut(QKeySequence(Qt::Key_F11), this, SLOT(toggleFullScreen()));
	new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(toggleDarkMode()));
	new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(toggleLightMode()));	

    {
        auto layout = new QHBoxLayout(p);
        {
            auto pb = new QPushButton("Load File", this);
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(loadFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Prev File", this);
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(prevFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Next File", this);
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(nextFile()));
            layout->addWidget(pb);
        }
		{
			auto pb = new QPushButton("Full Screen", this);
			pb->setFixedSize(pb->sizeHint());
			connect(pb, SIGNAL(clicked()), SLOT(toggleFullScreen()));
			layout->addWidget(pb);
		}
        top_layout->addLayout(layout, 0, 0);
    }

    {
        m_OverallPrimary = new COverallView(this);
        m_OverallZoomed = new COverallView(this);
        m_PlotView = new CPlotView(this);

        m_OverallPrimary->setMinimumSize(QSize(1, 1));
        m_OverallZoomed->setMinimumSize(QSize(1, 1));
        m_PlotView->setMinimumSize(QSize(1, 1));

        connect(m_OverallPrimary, SIGNAL(rangeSelected(float, float)), SLOT(rangeSelected(float, float)));

        m_OverallPrimary->setFixedWidth(scroller_w);
        m_OverallZoomed->setFixedWidth(scroller_w);
        m_PlotView->setFixedWidth(scroller_w);

        m_OverallZoomed->enableSelection(false);
        m_OverallZoomed->enableHilbertCurve(false);

        m_PlotView->enableSelection(false);

        auto layout = new QHBoxLayout(p);
        layout->addWidget(m_OverallPrimary);
        layout->addWidget(m_OverallZoomed);
        layout->addWidget(m_PlotView);
        top_layout->addLayout(layout, 1, 0);
    }

    {
        auto layout = new QHBoxLayout(p);

        {
            m_CurrentView = new QComboBox(this);
            m_CurrentView->addItem("3D histogram");
            m_CurrentView->addItem("2D histogram");
            m_CurrentView->addItem("Binary view");
            m_CurrentView->addItem("Image view");
            m_CurrentView->addItem("Dot plot");
            m_CurrentView->setFixedSize(m_CurrentView->sizeHint());
            connect(m_CurrentView, SIGNAL(currentIndexChanged(int)), SLOT(switchView(int)));
            layout->addWidget(m_CurrentView);
        }
        {
            m_Filename = new QLabel(this);
            layout->addWidget(m_Filename);
        }

        top_layout->addLayout(layout, 0, 1);
    }

    {
        m_Histogram3D = new CHistogram3D(this);
        m_Histogram2D = new CHistogram2D(this);
        m_HexView = new CHexView(this);
        m_ImageView = new CImageView(this);
        m_DotPlot = new CDotPlot(this);

        m_Histogram3D->setMinimumSize(QSize(1, 1));
        m_Histogram2D->setMinimumSize(QSize(1, 1));
        m_HexView->setMinimumSize(QSize(1, 1));
        m_ImageView->setMinimumSize(QSize(1, 1));
        m_DotPlot->setMinimumSize(QSize(1, 1));

        m_Views.push_back(m_Histogram3D);
        m_Views.push_back(m_Histogram2D);
        m_Views.push_back(m_HexView);
        m_Views.push_back(m_ImageView);
        m_Views.push_back(m_DotPlot);

        auto layout = new QHBoxLayout(p);
        for (const auto &j : m_Views) {
            layout->addWidget(j);
        }

        top_layout->addLayout(layout, 1, 1);
    }

    switchView(-1);
    setLayout(top_layout);
}

CMain::~CMain() {
    quit();
}

void CMain::resizeEvent(QResizeEvent *e) {
    QDialog::resizeEvent(e);
    updateViews();
}

void CMain::dropEvent(QDropEvent* ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();
    if (!urls.empty()) {
        QStringList list;
        for (QUrl url : urls) {
            list.append(url.toLocalFile());
        }
        loadFiles(list);
    }
}

void CMain::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->acceptProposedAction();
}

void CMain::quit() {
    if (!m_DoneFlag) {
        m_DoneFlag = true;

        exit(EXIT_SUCCESS);
    }
}

void CMain::reject() {
    quit();
}

bool CMain::loadFile(const QString &filename) {
    if (m_FileList.size() > 1) {
        m_Filename->setText(QString("%1/%2: %3").arg(m_CurrentFile + 1).arg(m_FileList.size()).arg(filename));
    } else {
        m_Filename->setText(filename);
    }
    g_currentfile = filename;

    QFile file(filename.toStdString().c_str());
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
        fprintf(stderr, "Unable to open %s\n", filename.toStdString().c_str());
        return false;
    }

    if (m_Data != nullptr) {
        delete[] m_Data;
        m_Data = nullptr;
        m_Size = 0;
        m_Start = 0;
        m_End = 0;
    }

    qsizetype len = file.size();
    m_Data = new quint8[len];
    m_Size = file.read(reinterpret_cast<char*>(m_Data), len);

    if (m_Size != len) {
        printf("premature read %zu of %zu\n", m_Size, len);
    }

    m_Start = 0;
    m_End = m_Size;

    updateViews();
    return true;
}

bool CMain::loadFiles(const QStringList &filenames) {
    m_FileList = filenames;
    m_CurrentFile = -1;
    return nextFile();
}

bool CMain::prevFile() {
    bool rv = false;
    while (m_CurrentFile > 0 && !rv) {
        m_CurrentFile--;
        rv = loadFile(m_FileList[m_CurrentFile]);
    }
    return rv;
}

bool CMain::nextFile() {
    bool rv = false;
    while (m_CurrentFile + 1 < m_FileList.size() && !rv) {
        m_CurrentFile++;
        rv = loadFile(m_FileList[m_CurrentFile]);
    }
    return rv;
}

void CMain::loadFile() {
    QStringList files = QFileDialog::getOpenFileNames(
            this,
            "Select one or more files to open");
    if (!files.empty()) {
        loadFiles(files);
    }
}

bool CMain::loadStyle(QString s)
{
    QFile f(s);
    bool rv = false;

    if (!f.exists()) {
        printf("Unable to set style; file '%s' not found\n", f.fileName().toLocal8Bit().constData());
    }
    else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());

        rv = true;
    }

    return rv;
}

void CMain::updateViews(bool update_iv1) {
    if (update_iv1) m_OverallPrimary->clear();

    if (m_Data == nullptr) return;

    // iv1 shows the entire file, iv2 shows the current segment
    if (update_iv1) m_OverallPrimary->setData(m_Data + 0, m_Size);
    m_OverallZoomed->setData(m_Data + m_Start, m_End - m_Start);

    {
        int64_t n;
        auto dd = generate_entropy(m_Data + m_Start, m_End - m_Start, n);
        if (dd) {
            m_PlotView->setData(0, dd, n);
            delete[] dd;
        }
    }

    {
        auto dd = generate_histo(m_Data + m_Start, m_End - m_Start);
        if (dd) {
            m_PlotView->setData(1, dd, 256, false);
            delete[] dd;
        }
    }

    if (m_Histogram3D->isVisible()) m_Histogram3D->setData(m_Data + m_Start, m_End - m_Start);
    if (m_Histogram2D->isVisible()) m_Histogram2D->setData(m_Data + m_Start, m_End - m_Start);
    if (m_HexView->isVisible()) {
//        binary_viewer_->setData(bin_ + start_, end_ - start_);
        m_HexView->setData(m_Data, m_End);
        m_HexView->setStart(m_Start / 16);
    }
    if (m_ImageView->isVisible()) m_ImageView->setData(m_Data + m_Start, m_End - m_Start);
    if (m_DotPlot->isVisible()) m_DotPlot->setData(m_Data + m_Start, m_End - m_Start);
}

void CMain::rangeSelected(float s, float e) {
    m_Start = s * m_Size;
    m_End = e * m_Size;
    updateViews(false);
}

void CMain::switchView(int ind) {
    for (const auto &j : m_Views) {
        j->hide();
    }

    {
        QSettings settings;
        if (ind == -1) {
            ind = settings.value("last_view", 0).toInt();
        }
        ind = std::max(ind, 0);
        ind = std::min(ind, int(m_Views.size() - 1));
        settings.setValue("last_view", ind);
        m_CurrentView->blockSignals(true);
        m_CurrentView->setCurrentIndex(ind);
        m_CurrentView->blockSignals(false);
    }

    m_Views[ind]->show();
    updateViews(false);
}

void CMain::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case (Qt::Key_W):
    case (Qt::Key_8):
    {
        m_Histogram3D->setTransformFlags(MOVE_UP);
        break;
    }
    case (Qt::Key_S):
    case (Qt::Key_5):
    case (Qt::Key_2):
    {
        m_Histogram3D->setTransformFlags(MOVE_DOWN);
        break;
    }
    case (Qt::Key_A):
    case (Qt::Key_4):
    {
        m_Histogram3D->setTransformFlags(MOVE_LEFT);
        break;
    }
    case (Qt::Key_D):
    case (Qt::Key_6):
    {
        m_Histogram3D->setTransformFlags(MOVE_RIGHT);
        break;
    }
    case (Qt::Key_Q):
    case (Qt::Key_7):
    {
        m_Histogram3D->setTransformFlags(SCALE_DOWN);
        break;
    }
    case (Qt::Key_E):
    case (Qt::Key_9):
    {
        m_Histogram3D->setTransformFlags(SCALE_UP);
        break;
    }
    case (Qt::Key_1):
    {
        m_Histogram3D->setTransformFlags(SCALE_DOWN | SCALE_DOWN_Z);
        break;
    }
    case (Qt::Key_3):
    {
        m_Histogram3D->setTransformFlags(SCALE_UP | SCALE_UP_Z);
        break;
    }
    default:
        break;
    }
}

void CMain::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case (Qt::Key_W):
    case (Qt::Key_8):
    {
        m_Histogram3D->removeTransformFlags(MOVE_UP);
        break;
    }
    case (Qt::Key_S):
    case (Qt::Key_5):
    case (Qt::Key_2):
    {
        m_Histogram3D->removeTransformFlags(MOVE_DOWN);
        break;
    }
    case (Qt::Key_A):
    case (Qt::Key_4):
    {
        m_Histogram3D->removeTransformFlags(MOVE_LEFT);
        break;
    }
    case (Qt::Key_D):
    case (Qt::Key_6):
    {
        m_Histogram3D->removeTransformFlags(MOVE_RIGHT);
        break;
    }
    case (Qt::Key_Q):
    case (Qt::Key_7):
    {
        m_Histogram3D->removeTransformFlags(SCALE_DOWN);
        break;
    }
    case (Qt::Key_E):
    case (Qt::Key_9):
    {
        m_Histogram3D->removeTransformFlags(SCALE_UP);
        break;
    }
    case (Qt::Key_1):
    {
        m_Histogram3D->removeTransformFlags(SCALE_DOWN | SCALE_DOWN_Z);
        break;
    }
    case (Qt::Key_3):
    {
        m_Histogram3D->removeTransformFlags(SCALE_UP | SCALE_UP_Z);
        break;
    }
    default:
        break;
    }
}
