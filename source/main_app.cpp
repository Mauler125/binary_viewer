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

void MainApp::toggleFullScreen() {
	if (!isFullScreen()) {
		return_to_maximized = isMaximized();
	}
	if (isFullScreen() && !return_to_maximized) {
		showNormal();
	}
	else if (isFullScreen() && return_to_maximized) {
		showMaximized();
	}
	else {
		showFullScreen();
	}
}

void MainApp::toggleQDarkMode() {

	QFile f(":/qdarkstyle/style.qss");

	if (!f.exists()) {
		printf("Unable to set stylesheet, file not found\n");
	}
	else {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}
}

void MainApp::toggleLightMode()
{
	// Increase the "+0" int value to increase the font size for better reading
	QFont defaultFont = QApplication::font();
	defaultFont.setPointSize(defaultFont.pointSize() + 0);

	// Modify palette to light
	QPalette lightPalette;
	lightPalette.setColor(QPalette::Window, QColor(230, 230, 230));
	lightPalette.setColor(QPalette::WindowText, Qt::black);
	lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
	lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
	lightPalette.setColor(QPalette::AlternateBase, QColor(255, 255, 255));
	lightPalette.setColor(QPalette::ToolTipBase, Qt::black);
	lightPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
	lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
	lightPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	lightPalette.setColor(QPalette::Dark, QColor(200, 200, 200));
	lightPalette.setColor(QPalette::Shadow, QColor(150, 150, 150));
	lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
	lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
	lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
	lightPalette.setColor(QPalette::BrightText, Qt::red);
	lightPalette.setColor(QPalette::Link, QColor(42, 130, 218));
	lightPalette.setColor(QPalette::Highlight, QColor(220, 220, 220));
	lightPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(170, 170, 170));
	lightPalette.setColor(QPalette::HighlightedText, Qt::black);
	lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

	qApp->setFont(defaultFont);
	qApp->setPalette(lightPalette);
	qApp->setStyle(QStyleFactory::create("Fusion"));
	qApp->setStyleSheet("");

	QFile f("qinterface//light.css");
	if (!f.exists())
	{
		printf("Unable to set light stylesheet, file not found\n");
	}
	else
	{
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}
}

void MainApp::toggleDarkMode()
{
	// Increase the "+0" int value to increase the font size for better reading
	QFont defaultFont = QApplication::font();
	defaultFont.setPointSize(defaultFont.pointSize() + 0);

	// Modify palette to dark
	QPalette darkPalette;
	darkPalette.setColor(QPalette::Window, QColor(32, 34, 38));
	darkPalette.setColor(QPalette::WindowText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
	darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
	darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
	darkPalette.setColor(QPalette::ToolTipText, QColor(200, 200, 200));
	darkPalette.setColor(QPalette::Text, QColor(200, 200, 200));
	darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
	darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
	darkPalette.setColor(QPalette::Button, QColor(36, 38, 40));
	darkPalette.setColor(QPalette::ButtonText, QColor(200, 200, 200));
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::BrightText, Qt::red);
	darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	darkPalette.setColor(QPalette::HighlightedText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

	qApp->setFont(defaultFont);
	qApp->setPalette(darkPalette);
	qApp->setStyle(QStyleFactory::create("Fusion"));
	qApp->setStyleSheet("");

	QFile f("qinterface//dark.css");
	if (!f.exists())
	{
		printf("Unable to set dark stylesheet, file not found\n");
	}
	else
	{
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}
}

MainApp::MainApp(QWidget *p)
        : QDialog(p), cur_file_(-1), bin_(nullptr), bin_len_(0), start_(0), end_(0) {
    done_flag_ = false;

    auto top_layout = new QGridLayout;

    new QShortcut(QKeySequence(Qt::Key_F11), this, SLOT(toggleFullScreen()));
	new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(toggleQDarkMode()));
	new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(toggleLightMode()));	
	new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(toggleDarkMode()));

    {
        auto layout = new QHBoxLayout;
        {
            auto pb = new QPushButton("Load File");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(loadFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Prev File");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(prevFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Next File");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(nextFile()));
            layout->addWidget(pb);
        }
		{
			auto pb = new QPushButton("Full Screen");
			pb->setFixedSize(pb->sizeHint());
			connect(pb, SIGNAL(clicked()), SLOT(toggleFullScreen()));
			layout->addWidget(pb);
		}
        top_layout->addLayout(layout, 0, 0);
    }

    {
        overall_primary_ = new OverallView;
        overall_zoomed_ = new OverallView;
        plot_view_ = new PlotView;

        connect(overall_primary_, SIGNAL(rangeSelected(float, float)), SLOT(rangeSelected(float, float)));

        overall_primary_->setFixedWidth(scroller_w);
        overall_zoomed_->setFixedWidth(scroller_w);
        plot_view_->setFixedWidth(scroller_w);

        overall_zoomed_->enableSelection(false);
        plot_view_->enableSelection(false);

        auto layout = new QHBoxLayout;
        layout->addWidget(overall_primary_);
        layout->addWidget(overall_zoomed_);
        layout->addWidget(plot_view_);
        top_layout->addLayout(layout, 1, 0);
    }

    {
        auto layout = new QHBoxLayout;

        {
            cur_view_ = new QComboBox();
            cur_view_->addItem("3D histogram");
            cur_view_->addItem("2D histogram");
            cur_view_->addItem("Binary view");
            cur_view_->addItem("Image view");
            cur_view_->addItem("Dot plot");
            cur_view_->setFixedSize(cur_view_->sizeHint());
            connect(cur_view_, SIGNAL(currentIndexChanged(int)), SLOT(switchView(int)));
            layout->addWidget(cur_view_);
        }
        {
            filename_ = new QLabel();
            layout->addWidget(filename_);
        }

        top_layout->addLayout(layout, 0, 1);
    }

    {
        histogram_3d_ = new Histogram3dView;
        histogram_2d_ = new Histogram2dView;
        binary_viewer_ = new BinaryViewer;
        image_view_ = new ImageView;
        dot_plot_ = new DotPlot;

        views_.push_back(histogram_3d_);
        views_.push_back(histogram_2d_);
        views_.push_back(binary_viewer_);
        views_.push_back(image_view_);
        views_.push_back(dot_plot_);

        auto layout = new QHBoxLayout;
        for (const auto &j : views_) {
            layout->addWidget(j);
        }

        top_layout->addLayout(layout, 1, 1);
    }

    switchView(-1);

    setLayout(top_layout);
}

MainApp::~MainApp() {
    quit();
}

void MainApp::resizeEvent(QResizeEvent *e) {
    QDialog::resizeEvent(e);
    update_views();
}

void MainApp::quit() {
    if (!done_flag_) {
        done_flag_ = true;

        exit(EXIT_SUCCESS);
    }
}

void MainApp::reject() {
    quit();
}

bool MainApp::load_file(const QString &filename) {
    QString title;
    if (files_.size() > 1) {
        title = QString("%1/%2: %3").arg(cur_file_ + 1).arg(files_.size()).arg(filename);
    } else {
        title = filename;
    }
    filename_->setText(title);

    FILE *f = fopen(filename.toStdString().c_str(), "rb");
    if (!f) {
        fprintf(stderr, "Unable to open %s\n", filename.toStdString().c_str());
        return false;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (bin_ != nullptr) {
        delete[] bin_;
        bin_ = nullptr;
        bin_len_ = 0;
        start_ = 0;
        end_ = 0;
    }

    bin_ = new unsigned char[len];
    bin_len_ = fread(bin_, 1, len, f);
    fclose(f);

    if (len != bin_len_) {
        printf("premature read %ld of %ld\n", bin_len_, len);
    }

    start_ = 0;
    end_ = bin_len_;

    update_views();

    return true;
}

bool MainApp::load_files(const QStringList &filenames) {
    files_ = filenames;
    cur_file_ = -1;
    return nextFile();
}

bool MainApp::prevFile() {
    bool rv = false;
    while (cur_file_ > 0 && !rv) {
        cur_file_--;
        rv = load_file(files_[cur_file_]);
    }
    return rv;
}

bool MainApp::nextFile() {
    bool rv = false;
    while (cur_file_ + 1 < files_.size() && !rv) {
        cur_file_++;
        rv = load_file(files_[cur_file_]);
    }
    return rv;
}

void MainApp::loadFile() {
    QStringList files = QFileDialog::getOpenFileNames(
            this,
            "Select one or more files to open");
    if (!files.empty()) {
        load_files(files);
    }
}

void MainApp::update_views(bool update_iv1) {
    if (update_iv1) overall_primary_->clear();

    if (bin_ == nullptr) return;

    // iv1 shows the entire file, iv2 shows the current segment
    if (update_iv1) overall_primary_->set_data(bin_ + 0, bin_len_);
    overall_zoomed_->set_data(bin_ + start_, end_ - start_);

    {
        long n;
        auto dd = generate_entropy(bin_ + start_, end_ - start_, n);
        if (dd) {
            plot_view_->set_data(0, dd, n);
            delete[] dd;
        }
    }

    {
        auto dd = generate_histo(bin_ + start_, end_ - start_);
        if (dd) {
            plot_view_->set_data(1, dd, 256, false);
            delete[] dd;
        }
    }

    if (histogram_3d_->isVisible()) histogram_3d_->setData(bin_ + start_, end_ - start_);
    if (histogram_2d_->isVisible()) histogram_2d_->setData(bin_ + start_, end_ - start_);
    if (binary_viewer_->isVisible()) {
//        binary_viewer_->setData(bin_ + start_, end_ - start_);
        binary_viewer_->setData(bin_, end_);
        binary_viewer_->setStart(start_ / 16);
    }
    if (image_view_->isVisible()) image_view_->setData(bin_ + start_, end_ - start_);
    if (dot_plot_->isVisible()) dot_plot_->setData(bin_ + start_, end_ - start_);
}

void MainApp::rangeSelected(float s, float e) {
    start_ = s * bin_len_;
    end_ = e * bin_len_;
    update_views(false);
}

void MainApp::switchView(int ind) {
    for (const auto &j : views_) {
        j->hide();
    }

    {
        QSettings settings;
        if (ind == -1) {
            ind = settings.value("last_view", 0).toInt();
        }
        ind = std::max(ind, 0);
        ind = std::min(ind, int(views_.size() - 1));
        settings.setValue("last_view", ind);
        cur_view_->blockSignals(true);
        cur_view_->setCurrentIndex(ind);
        cur_view_->blockSignals(false);
    }

    views_[ind]->show();
    update_views(false);
}

void MainApp::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case (Qt::Key_W):
        {
            [[fallthrough]];
        }
        case (Qt::Key_A):
        {
            [[fallthrough]];
        }
        case (Qt::Key_S):
        {
            [[fallthrough]];
        }
        case (Qt::Key_D):
        {
            [[fallthrough]];
        }
        case (Qt::Key_Q):
        {
            [[fallthrough]];
        }
        case (Qt::Key_E):
        {
            [[fallthrough]];
        }
        default:
            break;
    }
}

void MainApp::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case (Qt::Key_W):
    {
        [[fallthrough]];
    }
    case (Qt::Key_A):
    {
        [[fallthrough]];
    }
    case (Qt::Key_S):
    {
        [[fallthrough]];
    }
    case (Qt::Key_D):
    {
        [[fallthrough]];
    }
    case (Qt::Key_Q):
    {
        [[fallthrough]];
    }
    case (Qt::Key_E):
    {
        [[fallthrough]];
    }
    default:
        break;
    }
}
