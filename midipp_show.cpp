/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "midipp_show.h"
#include "midipp_button.h"
#include "midipp_buttonmap.h"
#include "midipp_gridlayout.h"
#include "midipp_scores.h"
#include "midipp_import.h"
#include "midipp_mainwindow.h"

MppShowWidget :: MppShowWidget(MppShowControl *_parent)
{
	parent = _parent;

	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
}

MppShowWidget :: ~MppShowWidget()
{
}

void
MppShowWidget :: paintText(QPainter &paint, QString &str, int w, int h)
{
	QRectF txtBound;
	QFont fnt;
	qreal ps;
	qreal pt;
	qreal factor;
	int wm = w / 16;
	int hm = h / 16;

	if (str.size() == 0)
		return;

	w -= 2*wm;
	h -= 2*hm;

	fnt = parent->mw->defaultFont;

	for (ps = 1.0; ps < 128.0; ps *= 2.0) {
		fnt.setPixelSize(ps);
		paint.setFont(fnt);
		txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, str);

		if (txtBound.width() >= w || 
		    txtBound.height() >= h)
			break;
	}
	pt = ps;
	ps /= 2.0;

	for ( ; ps >= 1.0; ps /= 2.0) {
		pt -= ps;

		fnt.setPixelSize(pt);
		paint.setFont(fnt);
		txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, str);

		if (txtBound.width() < w &&
		    txtBound.height() < h)
			pt += ps;
	}

	fnt.setPixelSize(pt);
	paint.setFont(fnt);
	txtBound = paint.boundingRect(QRectF(wm,hm,w,h), Qt::AlignCenter, str);
	txtBound = QRectF(txtBound.x() - pt, txtBound.y() - pt,
	    txtBound.width() + (2.0 * pt), txtBound.height() + (2.0 * pt));
	paint.setPen(QPen(parent->fontBgColor, 16));
	paint.setBrush(parent->fontBgColor);
	factor = paint.opacity();
	paint.setOpacity(0.75 * factor);
	paint.drawRoundedRect(txtBound, 16, 16);
	paint.setOpacity(factor);
	paint.setPen(parent->fontFgColor);
	paint.drawText(QRectF(0,0,w+(2*wm),h+(2*hm)), Qt::AlignCenter, str);
}

void
MppShowWidget :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	QRectF txtBound;
	int w = width();
	int h = height();
	int bg_w = parent->background.width();
	int bg_h = parent->background.height();
	qreal ratio_w;
	qreal ratio_h;
	qreal ratio;
	int p;

	paint.setRenderHints(QPainter::Antialiasing, 1);

	QColor black(0,0,0);

	paint.fillRect(QRectF(0,0,w,h), black);

	p = parent->transition;

	if (bg_w != 0 && bg_h != 0) {
		ratio_w = (qreal)w / (qreal)bg_w;
		ratio_h = (qreal)h / (qreal)bg_h;
		if (ratio_w > ratio_h)
			ratio = ratio_w;
		else
			ratio = ratio_h;
	} else {
		ratio = 1.0;
	}

	QRect bg_rect((w - bg_w * ratio) / 2.0, (h - bg_h * ratio) / 2.0, bg_w * ratio, bg_h * ratio);

	if (p < MPP_TRAN_MAX) {
		paint.setOpacity((qreal)(MPP_TRAN_MAX - p - 1) / (qreal)MPP_TRAN_MAX);

		switch(parent->last_st) {
		case MPP_SHOW_ST_BLANK:
			break;
		case MPP_SHOW_ST_BACKGROUND:
			paint.drawPixmap(bg_rect,parent->background);
			break;
		case MPP_SHOW_ST_LYRICS:
			paint.drawPixmap(bg_rect,parent->background);
 			paintText(paint, parent->labelTxt[parent->trackview][parent->last_label], w, h);
			break;
		default:
			break;
		}
		paint.setOpacity((qreal)p / (qreal)MPP_TRAN_MAX);
	}

	switch(parent->curr_st) {
	case MPP_SHOW_ST_BLANK:
		break;
	case MPP_SHOW_ST_BACKGROUND:
		paint.drawPixmap(bg_rect,parent->background);
		break;
	case MPP_SHOW_ST_LYRICS:
		paint.drawPixmap(bg_rect,parent->background);
		paintText(paint, parent->labelTxt[parent->trackview][parent->curr_label], w, h);
		break;
	default:
		break;
	}
}

void
MppShowWidget :: keyPressEvent(QKeyEvent *key)
{
	if (key->key() == Qt::Key_Escape) {
		Qt::WindowStates state = windowState();
		if (state & Qt::WindowFullScreen)
			setWindowState(state ^ Qt::WindowFullScreen);
	}
}

void
MppShowWidget :: mouseDoubleClickEvent(QMouseEvent *e)
{
	/* toggle fullscreen */
	setWindowState(windowState() ^ Qt::WindowFullScreen);
}

MppShowControl :: MppShowControl(MppMainWindow *_mw)
{
	mw = _mw;

	last_label = 0;
	last_st = MPP_SHOW_ST_BLANK;

	curr_label = 0;
	curr_st = MPP_SHOW_ST_BLANK;

	transition = 0;
	trackview = 0;

	fontFgColor = QColor(0,0,0);
	fontBgColor = QColor(255,255,255);

	butTrack = new MppButtonMap("Track\0" "View-A\0" "View-B\0" "View-C\0",
#if MPP_MAX_VIEWS <= 3
				    MPP_MAX_VIEWS, MPP_MAX_VIEWS
#else
				    3, 3
#endif
				    );
	connect(butTrack, SIGNAL(selectionChanged(int)), this, SLOT(handle_track_change(int)));

	butMode = new MppButtonMap("Current mode\0" "BLANK\0" "BACKGROUND\0" "LYRICS\0", 3, 3);
	connect(butMode, SIGNAL(selectionChanged(int)), this, SLOT(handle_mode_change(int)));

	butShow = new QPushButton(tr("Show"));
	connect(butShow, SIGNAL(released()), this, SLOT(handle_show()));

	butFullScreen = new QPushButton(tr("FullScreen"));
	connect(butFullScreen, SIGNAL(released()), this, SLOT(handle_fullscreen()));

	butBackground = new QPushButton(tr("Background"));
	connect(butBackground, SIGNAL(released()), this, SLOT(handle_background()));

	butFontFgColor = new QPushButton(tr("Set Font Fg Color"));
	connect(butFontFgColor, SIGNAL(released()), this, SLOT(handle_change_font_fg_color()));

	butFontBgColor = new QPushButton(tr("Set Font Bg Color"));
	connect(butFontBgColor, SIGNAL(released()), this, SLOT(handle_change_font_bg_color()));

	gl_main = new MppGridLayout();

	gl_main->addWidget(butTrack, 0, 0, 1, 2);
	gl_main->addWidget(butMode, 0, 2, 1, 2);
	gl_main->addWidget(butBackground, 3, 0, 1, 1);
	gl_main->addWidget(butFontFgColor, 3, 1, 1, 1);
	gl_main->addWidget(butFontBgColor, 3, 2, 1, 1);
	gl_main->addWidget(butShow, 3, 3, 1, 1);
	gl_main->addWidget(butFullScreen, 3, 4, 1, 1);
	gl_main->setRowStretch(2, 1);
	gl_main->setColumnStretch(5, 1);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	wg_show = new MppShowWidget(this);

	watchdog->start(1000 / (2 * MPP_TRAN_MAX));
}

MppShowControl :: ~MppShowControl()
{
	watchdog->stop();
}

void
MppShowControl :: handle_mode_change(int mode)
{
	last_st = curr_st;
	curr_st = mode;
	transition = 0;
}

void
MppShowControl :: handle_label_change(int lbl)
{
	if (curr_label == lbl)
		return;

	last_label = curr_label;
	curr_label = lbl;

	if (curr_st == MPP_SHOW_ST_LYRICS)
		transition = 0;
}

void
MppShowControl :: handle_show()
{
	wg_show->setWindowState(wg_show->windowState() & ~Qt::WindowFullScreen);
	wg_show->show();
}

void
MppShowControl :: handle_fullscreen()
{
	wg_show->setWindowState(wg_show->windowState() | Qt::WindowFullScreen);
}

void
MppShowControl :: handle_watchdog()
{
	switch (trackview) {
	int label;
#if MPP_MAX_VIEWS > 0
	case 0:
		label = mw->scores_main[0]->getCurrLabel();
		if (label > -1 && label < MPP_MAX_LABELS)
			handle_label_change(label);
		break;
#endif
#if MPP_MAX_VIEWS > 1
	case 1:
		label = mw->scores_main[1]->getCurrLabel();
		if (label > -1 && label < MPP_MAX_LABELS)
			handle_label_change(label);
		break;
#endif
#if MPP_MAX_VIEWS > 2
	case 2:
		label = mw->scores_main[2]->getCurrLabel();
		if (label > -1 && label < MPP_MAX_LABELS)
			handle_label_change(label);
		break;
#endif
	default:
		break;
	}
	if (transition < MPP_TRAN_MAX) {
		transition++;
		wg_show->repaint();
	} else if (transition == MPP_TRAN_MAX) {
		transition++;
		last_label = curr_label;
		last_st = curr_st;
	}
}

void
MppShowControl :: handle_background()
{
        QFileDialog *diag = 
          new QFileDialog(mw, tr("Select Background File"), 
                MppHomeDirBackground,
                QString("Image Files (*.BMP *.bmp *.GIF *.gif *.JPG *.jpg *.JPEG "
		    "*.jpeg *.PNG *.png *.PBM *.pbm *.PGM *.pgm *.PPM *.ppm "
		    "*.XBM *.xbm *.XPM *.xpm)"));

	if (diag->exec()) {
                MppHomeDirBackground = diag->directory().path();
                background.load(diag->selectedFiles()[0]);
		transition = 0;
	}
}

void
MppShowControl :: handle_track_change(int n)
{
	trackview = n;
	transition = 0;
}

void
MppShowControl :: handle_change_font_fg_color()
{
	QColorDialog dlg(fontFgColor);

	if (dlg.exec() == QDialog::Accepted) {
		fontFgColor = dlg.currentColor();
		transition = 0;
	}
}

void
MppShowControl :: handle_change_font_bg_color()
{
	QColorDialog dlg(fontBgColor);

	if (dlg.exec() == QDialog::Accepted) {
		fontBgColor = dlg.currentColor();
		transition = 0;
	}
}
