/*-
 * Copyright (c) 2009-2013 Hans Petter Selasky. All rights reserved.
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

#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_looptab.h"
#include "midipp_pattern.h"
#include "midipp_bpm.h"
#include "midipp_mode.h"
#include "midipp_decode.h"
#include "midipp_import.h"
#include "midipp_replace.h"
#include "midipp_spinbox.h"
#include "midipp_groupbox.h"

MppScoreView :: MppScoreView(MppScoreMain *parent)
{
	pScores = parent;
}

void
MppScoreView :: mousePressEvent(QMouseEvent *e)
{
	pScores->viewMousePressEvent(e);
}

void
MppScoreView :: paintEvent(QPaintEvent *event)
{
	pScores->viewPaintEvent(event);
}

MppScoreTextEdit :: MppScoreTextEdit(MppScoreMain *parent)
{
	sm = parent;
}

MppScoreTextEdit :: ~MppScoreTextEdit(void)
{

}

void
MppScoreTextEdit :: mouseDoubleClickEvent(QMouseEvent *e)
{
	/* make sure the compiled version is up to date */
	sm->mainWindow->handle_compile();
	/* edit the line */
	if (sm->handleEditLine() == 0)
		sm->mainWindow->handle_compile();
}

MppScoreMain :: MppScoreMain(MppMainWindow *parent, int unit)
{
	QString defaultText;
	char buf[32];

	if (unit == 0) {
	  defaultText = tr(
	    "S\"L0 - verse: \"\n"
	    "\n"
	    "S\".(C)Welcome .(C)to .(D)MIDI .(E)Player .(C)Pro! \"\n"
	    "\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n"
	    "U1 D3 D4 D5 G5B A5 /* D */\n"
	    "U1 E3 E4 E5 A5B H5 /* E */\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n");
	}

	snprintf(buf, sizeof(buf), "%c-scores", 'A' + unit);

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	/* all devices are input */

	devInputMask = 0;
	baseKey = MPP_DEFAULT_BASE_KEY;
	cmdKey = MPP_DEFAULT_CMD_KEY;
	delayNoise = 25;
	chordContrast = 128;
	chordNormalize = 1;

	/* Set parent */

	mainWindow = parent;

	/* Buttons */

	butScoreFileNew = new QPushButton(tr("New"));
	butScoreFileOpen = new QPushButton(tr("Open"));
	butScoreFileSave = new QPushButton(tr("Save"));
	butScoreFileSaveAs = new QPushButton(tr("Save As"));
	butScoreFilePrint = new QPushButton(tr("Print"));
	butScoreFileAlign = new QPushButton(tr("Align"));
	spnScoreFileAlign = new MppSpinBox();
	spnScoreFileAlign->setValue(F5);
	butScoreFileStepUp = new QPushButton(tr("Step Up"));
	butScoreFileStepDown = new QPushButton(tr("Step Down"));
	butScoreFileSetSharp = new QPushButton(tr("Set #"));
	butScoreFileSetFlat = new QPushButton(tr("Set b"));
	butScoreFileExport = new QPushButton(tr("To Lyrics"));

	gbScoreFile = new MppGroupBox(tr(buf));
	gbScoreFile->addWidget(butScoreFileNew, 0, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileOpen, 1, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSave, 2, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSaveAs, 3, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFilePrint, 4, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileAlign, 5, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileAlign, 5, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepUp, 6, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileStepDown, 7, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSetSharp, 8, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileSetFlat, 8, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileExport, 9, 0, 1, 2);

	connect(butScoreFileNew, SIGNAL(released()), this, SLOT(handleScoreFileNew()));
	connect(butScoreFileOpen, SIGNAL(released()), this, SLOT(handleScoreFileOpen()));
	connect(butScoreFileSave, SIGNAL(released()), this, SLOT(handleScoreFileSave()));
	connect(butScoreFileSaveAs, SIGNAL(released()), this, SLOT(handleScoreFileSaveAs()));
	connect(butScoreFilePrint, SIGNAL(released()), this, SLOT(handleScorePrint()));
	connect(butScoreFileAlign, SIGNAL(released()), this, SLOT(handleScoreFileAlign()));
	connect(butScoreFileStepUp, SIGNAL(released()), this, SLOT(handleScoreFileStepUp()));
	connect(butScoreFileStepDown, SIGNAL(released()), this, SLOT(handleScoreFileStepDown()));
	connect(butScoreFileSetSharp, SIGNAL(released()), this, SLOT(handleScoreFileSetSharp()));
	connect(butScoreFileSetFlat, SIGNAL(released()), this, SLOT(handleScoreFileSetFlat()));
	connect(butScoreFileExport, SIGNAL(released()), this, SLOT(handleScoreFileExport()));

	/* Widget */

	viewWidget.setContentsMargins(0,0,0,0);

	/* Editor */

	editWidget = new MppScoreTextEdit(this);

	editWidget->setFont(parent->editFont);
	editWidget->setPlainText(defaultText);
	editWidget->setCursorWidth(4);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);

	/* GridLayout */

	viewGrid = new QGridLayout(&viewWidget);
	viewGrid->setSpacing(0);
	viewGrid->setContentsMargins(1,1,1,1);

	viewScroll = new QScrollBar(Qt::Vertical);
	viewScroll->setValue(0);
	viewScroll->setMinimum(0);
	viewScroll->setMaximum(0);
	viewScroll->setPageStep(1);

	connect(viewScroll, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

	/* Visual */

	viewWidgetSub = new MppScoreView(this);

	viewGrid->addWidget(viewWidgetSub, 0, 0, 1, 1);
	viewGrid->addWidget(viewScroll, 0, 1, 1, 1);

	/* Initial compile */

	handleCompile(1);

	QPainter paint;

	picChord[0] = new QPicture();
	paint.begin(picChord[0]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(color_black, 1));
	paint.setBrush(QColor(color_black));
	paint.drawEllipse(QRect(0,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();

	picChord[1] = new QPicture();
	paint.begin(picChord[1]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(color_black, 1));
	paint.setBrush(QColor(color_black));
	paint.drawEllipse(QRect(MPP_VISUAL_C_MAX,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();
}

MppScoreMain :: ~MppScoreMain()
{
	handleScoreFileNew();
}

void
MppScoreMain :: parseMax(int *pmax, float value)
{
	value += MPP_VISUAL_MARGIN;

	if (value > 2048.0)
		value = 2048.0;

	if (value < 0.0)
		value = 0.0;

	if ((int)value > *pmax)
		*pmax = value;
}

void
MppScoreMain :: handleParseSub(QPrinter *pd, QPoint orig, float scale_f)
{
	QPainter paint;
	QPicture *pic;
	QString str;
	int x;
	int y;
	int z;
	int y_max;
	int x_max;
	int check_x;
	uint8_t draw_chord;
	uint8_t last_dot;
	uint16_t duration;

	float chord_x_last;
	float chord_x;
	float text_x;
	float adj_x;
	float scale_min;
	float spacing = 0;

	QFont fnt_a;
	QFont fnt_b;

	pic = NULL;

	fnt_a = mainWindow->defaultFont;
	fnt_a.setPixelSize(mainWindow->defaultFont.pixelSize());

	fnt_b = mainWindow->defaultFont;
	fnt_b.setPixelSize(mainWindow->defaultFont.pixelSize() + 4);

	maxScoresWidth = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {
		visual[x].x_off = 0;
	}

	if (pd != NULL) {
#ifndef QT_NO_PRINTER
		/* count number of lines */
		for (x = y_max = y = 0; x != MPP_MAX_LINES; x++) {

			if (pageNext[x] != 0) {
				if (y > y_max)
					y_max = y;
				y = 0;
			}

			if (visual[x].pstr == NULL)
				continue;
			str = QString::fromUtf8(visual[x].pstr);
			y++;
		}

		if (y > y_max)
			y_max = y;

		if (y_max != 0) {
			scale_min = ((float)(pd->height() - (2 * orig.y()))) / 
			    (((float)y_max) * visual_y_max);

			if (scale_min < 0)
				scale_f = 0.5;	/* dummy */
			else if (scale_min < scale_f)
				scale_f = scale_min;
		}

		paint.begin(pd);
		paint.translate(orig);
		paint.scale(scale_f, scale_f);
		paint.translate(QPoint(0, -visual_y_max));
#endif
	}

	y_max = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {

		if (pd == NULL) {

			if (visual[x].pic != NULL) {
				delete (visual[x].pic);
				visual[x].pic = NULL;
			}
		} else {
#ifndef QT_NO_PRINTER
			if (pageNext[x] != 0) {
				pd->newPage();
				while (y_max--)
					paint.translate(QPoint(0, -visual_y_max));
				y_max = 0;
			}
#endif
		}

		if (visual[x].pstr == NULL)
			continue;
		str = QString::fromUtf8(visual[x].pstr);

		if (pd == NULL) {
			pic = new QPicture();
			paint.begin(pic);
		} else {
#ifndef QT_NO_PRINTER
			paint.translate(QPoint(0,visual_y_max));
			y_max++;
#endif
		}

		draw_chord = 0;
		last_dot = 0;
		chord_x_last = 0;
		chord_x = MPP_VISUAL_MARGIN;
		text_x = MPP_VISUAL_MARGIN;
		check_x = 0;
		z = x;
		x_max = 0;

		for (y = 0; y != str.size(); y++) {

			if (draw_chord)
				paint.setFont(fnt_b);
			else
				paint.setFont(fnt_a);

			QString temp(str[y]);
			QRectF temp_size = 
			    paint.boundingRect(QRectF(0,0,0,0), temp);
			QRectF space_size = paint.boundingRect(
			    QRectF(0,0,0,0), QString("-"));

			if (temp_size.width() == 0.0)
				temp_size = space_size;

			if (str[y] == '(') {
				draw_chord = 1;
				continue;
			} else if (str[y] == ')') {
				draw_chord = 0;
				continue;
			} else if (str[y] == '.') {

				if (str[y+1] == '[') {
					if ((str[y+2] >= '0') && (str[y+2] <= '9')) {
						if ((str[y+3] >= '0') && (str[y+3] <= '9')) {
							duration = 9;
						} else {
							duration = str[y+2].digitValue();
						}
					} else {
						duration = 0;
					}
					while ((str[y+1] != ']') && (str[y+1] != 0))
						y++;

					if (str[y+1] != 0)
						y++;
				} else {
					duration = 0;
				}

				paint.setPen(QPen(color_black, 1));
				paint.setBrush(QColor(color_black));

				if (last_dot) {
					text_x += 2 * MPP_VISUAL_R_MAX;
				}

				if (str[y+1] != 0) {
					adj_x = paint.boundingRect(QRectF(0,0,0,0),
					    QString(str[y+1])).width();
					if (adj_x == 0) {
						paint.boundingRect(QRectF(0,0,0,0),
						   QString("-")).width();
					}
					adj_x = (adj_x - MPP_VISUAL_R_MAX) / 2.0;
				} else {
					adj_x = 0.0;
				}

				if (z < MPP_MAX_LINES) {

					uint32_t foff;

					visual[z].x_off = text_x + adj_x;
					visual[z].y_off = (visual_y_max / 3);

					parseMax(&maxScoresWidth, 
					    visual[z].x_off + MPP_VISUAL_R_MAX);

					parseMax(&x_max, 
					    visual[z].x_off + MPP_VISUAL_R_MAX);

					paint.drawEllipse(QRect(visual[z].x_off,
						visual[z].y_off,
						MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

					if (duration != 0) {

					  foff = 0;

					  if (duration > 5)
						duration = 5;

					  paint.drawLine(
					      visual[z].x_off + MPP_VISUAL_R_MAX,
					      visual[z].y_off + (MPP_VISUAL_R_MAX/2),
					      visual[z].x_off + MPP_VISUAL_R_MAX,
					      visual[z].y_off + (MPP_VISUAL_R_MAX/2) -
					      (3*MPP_VISUAL_R_MAX));

					  while (duration--) {
						paint.drawLine(
						    visual[z].x_off + MPP_VISUAL_R_MAX,
						    visual[z].y_off + (MPP_VISUAL_R_MAX/2) -
						    (3*MPP_VISUAL_R_MAX) + foff,
						    visual[z].x_off,
						    visual[z].y_off + MPP_VISUAL_R_MAX -
						    (3*MPP_VISUAL_R_MAX) + foff);

						foff += (MPP_VISUAL_R_MAX/2);
					  }
					}
				}

				last_dot = 1;
				z++;
				continue;
			} else if (str[y] == '\n' || str[y] == '\r') {
				continue;
			}

			paint.setPen(QPen(color_black, 1));
			paint.setBrush(QColor(color_black));

			if (draw_chord) {
				paint.drawText(QPointF(chord_x,
				    (visual_y_max / 3) - (MPP_VISUAL_C_MAX / 4)), temp);

				chord_x += temp_size.width();

				chord_x_last = chord_x + (2.0 * space_size.width());

				parseMax(&maxScoresWidth, chord_x);

				parseMax(&x_max, chord_x);

			} else {

				float temp_space;

				/*
				 * Make sure there is enough room for
				 * the chords:
				 */
				if (y >= check_x) {
					uint16_t t;
					uint16_t u;
					float sum_x;

					sum_x = text_x;
					u = 0;

					for (t = y; t != str.size(); t++) {
						QRectF check_size = 
						  paint.boundingRect(QRectF(0,0,0,0),
						  QString(str[t]));

						if (check_size.width() == 0.0)
							check_size = space_size;

						if (str[t] == '(')
							break;
						if (str[t] == '.')
							continue;
						if (str[t] == ' ' || str[t] == '-')
							u++;

						sum_x += check_size.width();
					}

					if ((u != 0) && (str[t] == '(') && (sum_x < chord_x_last))
						spacing = (chord_x_last - sum_x) / (2.0 * (float)u);
					else
						spacing = 0;

					check_x = t;
				}

				if (str[y] == '-' || str[y] == ' ')
					temp_space = spacing;
				else
					temp_space = 0;

				text_x += temp_space;

				paint.drawText(QPointF(text_x, visual_y_max -
				    (visual_y_max / 3)), temp);

				text_x += temp_space;
				text_x += temp_size.width();
				chord_x = text_x;
				last_dot = 0;

				parseMax(&maxScoresWidth, text_x);

				parseMax(&x_max, text_x);
			}
		}

		if (pd == NULL) {
			int w;
			int h;

			w = x_max;
			h = visual_y_max;

			paint.end();

			if (w && h) {
				visual[x].pic = new QPixmap(w, h);

				paint.begin(visual[x].pic);
				paint.setRenderHints(QPainter::Antialiasing, 1);
				paint.setPen(QPen(color_white, 0));
				paint.setBrush(QColor(color_white));
				paint.drawRect(QRect(0,0, w, h));
				paint.drawPicture(0,0, *pic);
				paint.end();
			}

			delete (pic);

			pic = NULL;
		}
	}

	if (pd != NULL)
		paint.end();
}

#define	MAX_LINE_BUF 128

QString
MppScoreMain :: doExport(void)
{
	QString out;
	const char *ptr;
	int x;
	int y;
	int z;
	int t;
	QString linebuf[2];
	QChar c;
	uint8_t draw_chord;
	uint8_t do_skip;
	uint8_t any_chord;

	for (x = 0; x != MPP_MAX_LINES; x++) {

		ptr = visual[x].pstr;
		if (ptr == NULL)
			continue;

		QString str = QString::fromUtf8(visual[x].pstr);

		linebuf[0] = QString();
		linebuf[1] = QString();

		draw_chord = 0;
		do_skip = 0;
		any_chord = 0;

		for (z = t = y = 0; y != str.size(); y++) {

			c = str[y];

			if (c == '(') {
				draw_chord = 1;
				continue;
			} else if (c == ')') {
				draw_chord = 0;
				if (z < MAX_LINE_BUF) {
					linebuf[0] += ' ';
					z++;
				}
				continue;
			} else if (c == '.' && str[y+1] == '[') {
				do_skip = 1;
				continue;
			} else if (do_skip) {
				if (c == ']')
					do_skip = 0;
				continue;
			} else if (c == '.') {
				continue;
			} else if (c == '\n' || c == '\r') {
				continue;
			}

			if (draw_chord) {
				any_chord = 1;

				if (z < MAX_LINE_BUF) {
					linebuf[0][z] = c;
					z++;
				}
			} else {
				if (t < MAX_LINE_BUF) {
					linebuf[1][t] = c;
					t++;
				}
			}

			while (z < t) {
				linebuf[0] += ' ';
				z++;
			}
		}

		if (any_chord != 0) {
			out += linebuf[0];
			out += "\n";
		}

		out += linebuf[1];
		out += "\n";
	}
	return (out);
}

void
MppScoreMain :: viewMousePressEvent(QMouseEvent *e)
{
	int yi;
	uint16_t max;

	yi = e->y() / visual_y_max;

	if ((yi < 0) || (yi >= MPP_MAX_LINES))
		return;

	mainWindow->handle_tab_changed(1);

	pthread_mutex_lock(&mainWindow->mtx);

	max = linesMax;

	if (mousePressPos[yi] < max) {
		currPos = mousePressPos[yi];
		lastPos = mousePressPos[yi];
		isPlayKeyLocked = 0;
	}

	mainWindow->handle_stop();

	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppScoreMain :: viewPaintEvent(QPaintEvent *event)
{
	QPainter paint(viewWidgetSub);
	uint16_t max;
	uint16_t pos;
	uint16_t opos;
	uint16_t scroll;
	uint16_t y_blocks;
	uint16_t y_div;
	uint16_t y_rem;
	uint16_t yc_div;
	uint16_t yc_rem;
	uint16_t yo_div;
	uint16_t yo_rem;
	uint16_t x;
	uint16_t y;
	uint16_t z;

	paint.fillRect(event->rect(), color_white);

	pthread_mutex_lock(&mainWindow->mtx);
	pos = currPos;
	opos = lastPos;
	max = linesMax;
	scroll = picScroll;
	pthread_mutex_unlock(&mainWindow->mtx);

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;
	y_div = 0;
	y_rem = 0;
	yc_div = 0;
	yc_rem = 0;
	yo_div = 0;
	yo_rem = 0;

	/* locate current scroll position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && ((y - 1) >= scroll)) {
			y_rem = (y - 1);
			break;
		}
	}

	/* locate current play position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && (x >= pos)) {
			yc_rem = (y - 1);
			break;
		}
	}

	/* locate last play position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && (x >= opos)) {
			yo_rem = (y - 1);
			break;
		}
	}

	/* compute scrollbar */

	y_div = y_rem / y_blocks;
	y_rem = y_rem % y_blocks;

	/* align current position with scroll bar */

	if (yc_rem < y_rem) {
		yc_rem = 0;
		yc_div = picMax;
	} else {
		yc_rem -= y_rem;
		yc_div = yc_rem / y_blocks;
		yc_rem = yc_rem % y_blocks;
	}

	/* align last position with scroll bar */

	if (yo_rem < y_rem) {
		yo_rem = 0;
		yo_div = picMax;
	} else {
		yo_rem -= y_rem;
		yo_div = yo_rem / y_blocks;
		yo_rem = yo_rem % y_blocks;
	}

	/* paint */

	for (x = y = z = 0; x != max; x++) {

		if (visual[x].pic != NULL) {
			if ((y >= y_rem) &&
			    (y_div == ((y - y_rem) / y_blocks))) {
				/* compute mouse press jump positions */
				if (z < MPP_MAX_LINES) {
					mousePressPos[z] = x;
					z++;
				}
				paint.drawPixmap(
				     QPoint(0, ((y - y_rem) % y_blocks) * visual_y_max),
				    *(visual[x].pic));
			}
			y++;
		}
	}

	/* fill out rest of mousePressPos[] */

	while ((z < MPP_MAX_LINES) && (z <= y_blocks)) {
			mousePressPos[z] = 65535;
			z++;
	}

	/* overlay (last) */

	if ((pos != opos) && (yo_div == y_div) &&
	    (visual[opos].x_off != 0)) {
		paint.setPen(QPen(color_green, 4));
		paint.setBrush(QColor(color_green));
		paint.drawEllipse(QRect(visual[opos].x_off,
		    visual[opos].y_off + (yo_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

	}

	/* overlay (current) */

	if ((yc_div == y_div) && (visual[pos].x_off != 0)) {
		paint.setPen(QPen(color_logo, 4));
		paint.setBrush(QColor(color_logo));
		paint.drawEllipse(QRect(visual[pos].x_off,
		    visual[pos].y_off + (yc_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}

	/* overlay (active chord) */

	if (keyMode == MM_PASS_NONE_CHORD) {
		int width = viewWidgetSub->width();
		int mask;

		pthread_mutex_lock(&mainWindow->mtx);
		mask = ((mainWindow->get_time_offset() % 1000) >= 500);
		pthread_mutex_unlock(&mainWindow->mtx);

		if (width >= (2 * MPP_VISUAL_C_MAX)) {
		  if (pressed_future != 1 || mask) {
			paint.drawPicture(QPoint(width -
			    (2 * MPP_VISUAL_C_MAX), 0),
			    *picChord[0]);
		  }
		  if (pressed_future != 0 || mask) {
			paint.drawPicture(QPoint(width -
			    (2 * MPP_VISUAL_C_MAX), 0),
			    *picChord[1]);
		  }
		}
	}
}

int
mergeCompare(const void *_a, const void *_b)
{
	const struct MppMergeEntry *a = (const struct MppMergeEntry *)_a;
	const struct MppMergeEntry *b = (const struct MppMergeEntry *)_b;

	return ((a->ticks == b->ticks) ? 0 : 
	    (a->ticks > b->ticks) ? 1 : -1);
}

void
MppScoreMain :: resetAutoMelody(void)
{
	memset(ps.am_keys, 0, sizeof(ps.am_keys));
	ps.am_steps = 0;
}

void
MppScoreMain :: computeAutoMelody(void)
{
	int c;
	int k;
	int n;
	int x;
	int y;
	int z;

	for (c = 0; c != 16; c++) {
		for (n = x = 0; x != MPP_MAX_SCORES; x++) {
			if (scores[ps.line][x].channel == c &&
			    scores[ps.line][x].dur != 0) {
				n++;
			}
		}
		if (n >= 3) {
			/* check for chord */
			resetAutoMelody();

			for (x = 0; x != MPP_MAX_SCORES; x++) {
				if (scores[ps.line][x].dur != 0)
					ps.am_keys[scores[ps.line][x].key % 12] = 1;
			}
			return;
		}
	}

	ps.am_steps++;

	for (c = 0; c != 16; c++) {
		for (y = n = x = 0; x != MPP_MAX_SCORES; x++) {
			if (scores[ps.line][x].channel == c &&
			    scores[ps.line][x].dur != 0) {
				if (n == 0)
					y = x;
				n++;
			}
		}

		if (ps.index >= MPP_MAX_SCORES)
			break;

		if (n != 1)
			continue;

		k = scores[ps.line][y].key;

		if (ps.am_keys[k % 12] == 0)
			continue;

		for (z = 0, x = 11; x != 0; x--) {
			if (ps.am_keys[(k + x) % 12] != 0) {
				z++;
				if (ps.am_mode == z)
					break;
			}
		}
		if (x == 0)
			continue;

		/* compute harmonic tone */

		k = k - 12 + x;
		if (k >= 128 || k < 0)
			continue;

		scores[ps.line][ps.index].key = k;
		scores[ps.line][ps.index].dur = 1;
		scores[ps.line][ps.index].channel = scores[ps.line][y].channel;

		/* check if we have to split the tone */

		for (x = 0; x != ps.am_steps; x++) {

			z = ps.line - x - 1;
			if (z < 0)
				break;

			for (y = 0; y != MPP_MAX_SCORES; y++) {

				int t;

				if (scores[z][y].dur == 0 ||
				    scores[z][y].key != k ||
				    scores[z][y].channel != c)
					continue;

				t = (2 * x) + 1;

				if (scores[z][y].dur > t) {
					scores[z][y].dur = t;
					break;
				}
			}
			if (y != MPP_MAX_SCORES)
				break;
		}
		ps.index++;
	}
}

void
MppScoreMain :: newLine(uint8_t force, uint8_t label, uint8_t new_page)
{
top:
	if (ps.line >= MPP_MAX_LINES)
		return;

	if (force == 0 &&
	    ps.index == 0 &&
	    ps.mindex == 0 &&
	    timer_ticks_pre[ps.line] == 0 &&
	    timer_ticks_post[ps.line] == 0) {
		return;
	}

	/* check if we should merge any label calls */

	if (ps.mindex != 0) {

		uint32_t x;
		uint32_t y;

		uint32_t temp_tick;
		uint32_t last_tick;
		uint32_t dur;
		uint32_t delta;

		qsort(merge, ps.mindex, sizeof(merge[0]), &mergeCompare);

		last_tick = 0;

		for (x = 0; x != (uint32_t)ps.mindex; x++) {

			if ((merge[x].ticks != last_tick) &&
			    (ps.line < MPP_MAX_LINES)) {

				delta = merge[x].ticks - last_tick;
				last_tick = merge[x].ticks;

				timer_ticks_pre[ps.line] = delta;
				timer_ticks_post[ps.line] = 0;
				jumpLabel[ps.line] = 0;
				pageNext[ps.line] = 0;
				realLine[ps.line] = ps.realLine;
				ps.line++;
				ps.index = 0;
			}

			if (merge[x].is_on == 0)
				continue;

			dur = 0;
			temp_tick = merge[x].ticks;

			/* compute relative duration */
			for (y = x + 1; y < (uint32_t)ps.mindex; y++) {
				if (temp_tick != merge[y].ticks) {
					temp_tick = merge[y].ticks;
					dur++;
				}
				/* look for next key event on the given channel */
				if (merge[y].key == merge[x].key &&
				    merge[y].channel == merge[x].channel) {
					break;
				}
			}

			if (dur)
				dur = (2 * dur) - 1;

			if ((ps.line < MPP_MAX_LINES) && (ps.index < MPP_MAX_SCORES)) {
				scores[ps.line][ps.index].key = merge[x].key & 127;
				scores[ps.line][ps.index].dur = dur & 255;
				scores[ps.line][ps.index].channel = merge[x].channel & 15;
				ps.index++;
			}
		}
		ps.mindex = 0;

 		if (ps.line < MPP_MAX_LINES) {
 			jumpLabel[ps.line] = MPP_JUMP_REL ;
			pageNext[ps.line] = 0;
			realLine[ps.line] = ps.realLine;
			ps.line++;
			ps.index = 0;
		}
 		goto top;
	}

	if (ps.line < MPP_MAX_LINES) {

		if (ps.am_mode != 0)
			computeAutoMelody();

		jumpLabel[ps.line] = label;
		pageNext[ps.line] = new_page;
		realLine[ps.line] = ps.realLine;
		ps.line++;
		ps.index = 0;
	}
}

void
MppScoreMain :: newVisual()
{
	if (ps.line >= MPP_MAX_LINES)
		goto done;

	bufData[ps.bufIndex] = 0;

	if (visual[ps.line].pstr != NULL)
		free(visual[ps.line].pstr);

	visual[ps.line].pstr = strdup(bufData);
done:
	ps.bufIndex = 0;
}

char
MppScoreMain :: getChar(uint32_t offset)
{
	char c;

	offset += ps.x;

	if (offset >= (uint32_t)ps.ba->size())
		return (0);

	c = (*(ps.ba))[offset];

	/* convert non-printable characters into space */
	if (c == 0)
		return (' ');
	return (c);
}

int32_t
MppScoreMain :: getIntValue(uint32_t offset)
{
	int32_t value;
	char c;
	char neg;

	c = getChar(offset);

	/* check sign, if any */
	if (c == '-') {
		neg = 1;
		parseAdv(1);
		c = getChar(offset);
	} else if (c == '+') {
		neg = 0;
		parseAdv(1);
		c = getChar(offset);
	} else {
		neg = 0;
	}

	value = 0;

	while (c >= '0' && c <= '9') {
		value *= 10;
		value += c - '0';
		parseAdv(1);
		c = getChar(offset);
	}

	if (neg)
		value = -value;

	return (value);
}

void
MppScoreMain :: parseAdv(uint8_t delta)
{
	char c;
	while (delta--) {
		if (ps.x >= 0) {
			ps.realCol++;
			c = getChar(0);
			if (c == '\n') {
				ps.realLine++;
				ps.realCol = 0;
			}
		}
		ps.x++;
	}
}

uint32_t
MppScoreMain :: resolveJump(uint32_t line)
{
	uint32_t jump;

	if (line >= MPP_MAX_LINES) {
		jump = 1;	/* jump to next */
	} else {
		uint8_t lbl;

		lbl = jumpLabel[line];
		if (lbl >= MPP_JUMP_REL)
			jump = line + 2 + (lbl - MPP_JUMP_REL);
		else if (lbl > 0 && lbl <= MPP_MAX_LABELS)
			jump = jumpTable[lbl - 1];
		else
			jump = 0;
	}
	/* clip */
	if (jump != 0 && jump > (uint32_t)linesMax)
		jump = 1;
	return (jump);
}

uint32_t
MppScoreMain :: resolveDuration(uint32_t line_start, uint32_t line_end, uint8_t dur)
{
	uint32_t duration = 0;

	while (line_start < line_end) {

		if (timer_ticks_pre[line_start] == 0 &&
		    timer_ticks_post[line_start] == 0)
			break;

		if (jumpLabel[line_start] != 0)
			break;

		if (dur < 2) {
			if (dur == 1) {
				duration += timer_ticks_pre[line_start];
				break;
			}
			break;
		}

		dur -= 2;
		duration += timer_ticks_pre[line_start];
		duration += timer_ticks_post[line_start];
		line_start++;
	}
	return (duration);
}

void
MppScoreMain :: handleParse(const QString &pstr)
{
	int z;
	int c;
	int y;
	int label;
	int channel;
	int command;
	int base_key;
	int duration;
	int flag;
	int timer;
	int transpose;
	int jmp_line;
	int curr_ticks;
	int key;
	int dur;
	int end_line;
	int post_inc;
	int line_off;

	/* cleanup all scores */

	for (z = 0; z != MPP_MAX_LINES; z++) {
		if (visual[z].pstr != NULL) {
			free(visual[z].pstr);
			visual[z].pstr = NULL;
		}
	}

	memset(&ps, 0, sizeof(ps));

	ps.x = -1;
	ps.ba = new QByteArray(pstr.toUtf8());

	memset(chord_info, 0, sizeof(chord_info));
	memset(scores, 0, sizeof(scores));
	memset(merge, 0, sizeof(merge));
	memset(jumpLabel, 0, sizeof(jumpLabel));
	memset(jumpTable, 0, sizeof(jumpTable));
	memset(pageNext, 0, sizeof(pageNext));
	memset(realLine, 0, sizeof(realLine));
	memset(playCommand, 0, sizeof(playCommand));
	memset(timer_ticks_pre, 0, sizeof(timer_ticks_pre));
	memset(timer_ticks_post, 0, sizeof(timer_ticks_post));
	active_channels = 1;
	num_base = 2;

	if (pstr.isNull() || pstr.isEmpty())
		goto done;

next_line:
	newLine(0,0,0);

	y = -1;
	channel = 0;
	duration = 1;
	transpose = 0;

next_char:
	parseAdv(1);
	y++;

	c = getChar(0);
	c = toupper(c);

	switch (c) {
	case 'C':
		if (y == 0) {
			base_key = C0;
			goto parse_score;
		}
		goto next_char;
	case 'D':
		if (y == 0) {
			base_key = D0;
			goto parse_score;
		}
		goto next_char;
	case 'E':
		if (y == 0) {
			base_key = E0;
			goto parse_score;
		}
		goto next_char;
	case 'F':
		if (y == 0) {
			base_key = F0;
			goto parse_score;
		}
		goto next_char;
	case 'G':
		if (y == 0) {
			base_key = G0;
			goto parse_score;
		}
		goto next_char;
	case 'A':
		if (y == 0) {
			base_key = A0;
			goto parse_score;
		}
		goto next_char;
	case 'B':
	case 'H':
		if (y == 0) {
			base_key = H0;
			goto parse_score;
		}
		goto next_char;
	case 'T':
		if (y == 0) {
			goto parse_channel;
		}
		goto next_char;
	case 'K':
		if (y == 0) {
			goto parse_command;
		}
		goto next_char;
	case 'L':
		if (y == 0) {
			goto parse_label;
		}
		goto next_char;
	case 'M':
		if (y == 0) {
			goto parse_inline;
		}
		goto next_char;
	case 'J':
		if (y == 0) {
			goto parse_jump;
		}
		goto next_char;
	case 'U':
		if (y == 0) {
			goto parse_duration;
		}
		goto next_char;
	case 'S':
		if (y == 0) {
			goto parse_string;
		}
		goto next_char;
	case 'W':
		if (y == 0) {
			goto parse_timer;
		}
		goto next_char;
	case 'X':
		if (y == 0) {
			goto parse_transpose;
		}
		goto next_char;
	case 0:
		goto done;
	case '\n':
		goto next_line;
	case ';':
		if (y == 0) {
			goto next_line;
		}
		goto next_char;
	case '/':
		/* check for comment */
		c = getChar(1);
		if (c == '*') {
			while (1) {
				c = getChar(2);
				if (c == '*') {
					c = getChar(3);
					if (c == '/') {
						/* end of comment */
						parseAdv(3);
						break;
					}
				}
				if (c == 0)
					goto done;
				parseAdv(1);
			}
			y = -1;
		}
		goto next_char;
	case ' ':
	case '\t':
		y = -1;
		goto next_char;
	default:
		goto next_char;
	}

parse_score:

	/* add octave number */
	base_key += 12 * getIntValue(1);

	c = getChar(1);
	if (c == 'B' || c == 'b') {
		base_key -= 1;
		parseAdv(1);
	}

	/* transpose, if any */
	base_key += transpose;

	if ((base_key >= 0) && (base_key <= 127) &&
	    (ps.line < MPP_MAX_LINES) && (ps.index < MPP_MAX_SCORES)) {

		uint8_t t_key;
		uint8_t t_dur;
		uint8_t t_chan;

		t_key = base_key & 127;
		t_dur = duration & 255;
		t_chan = channel & 15;

		/* check for duplicate keys */
		for (z = 0; z != ps.index; z++) {
			if (scores[ps.line][z].key != t_key)
				continue;
			if (scores[ps.line][z].channel != t_chan)
				continue;
			break;
		}

		/* insert new key into list */
		if (z == ps.index) {
			scores[ps.line][z].key = t_key;
			scores[ps.line][z].dur = t_dur;
			scores[ps.line][z].channel = t_chan;
			ps.index++;
		}
	}
	goto next_char;

parse_duration:

	duration = 2 * getIntValue(1);

	c = getChar(1);

	if (c == '.') {
		parseAdv(1);
	} else if (duration > 0) {
		duration --;
	}
	goto next_char;

parse_timer:

	timer = getIntValue(1);

	if (ps.line < MPP_MAX_LINES)
		timer_ticks_pre[ps.line] = timer;

	c = getChar(1);

	if (c == '.') {
		parseAdv(1);
		timer = getIntValue(1);
	}

	if (ps.line < MPP_MAX_LINES)
		timer_ticks_post[ps.line] = timer;

	goto next_char;

parse_channel:

	channel = getIntValue(1);

	if (channel < 16)
		active_channels |= (1 << channel);

	goto next_char;

parse_transpose:

	transpose = getIntValue(1);

	goto next_char;

parse_command:
	command = getIntValue(1);

	if (command == 5) {
		c = getChar(1);
		if (c == '.') {
			parseAdv(1);
			num_base = getIntValue(1);
			if (num_base > 12)
				num_base = 12;
		} else {
			num_base = 2;
		}
		goto next_char;

	} else if (command == 4) {
		c = getChar(1);
		if (c == '.') {
			parseAdv(1);
			ps.am_mode = getIntValue(1);
		} else {
			ps.am_mode = 0;	/* disabled */
		}
		goto next_char;

	} else if (command == 3) {
		int ref = 120;
		int per = 0;

		c = getChar(1);
		if (c == '.') {
			parseAdv(1);
			ref = getIntValue(1);
		}

		c = getChar(1);
		if (c == '.') {
			parseAdv(1);
			per = getIntValue(1);
		}

		/* range check */
		if (ref < 1)
			ref = 1;
		else if (ref > 6000)
			ref = 6000;

		if (per < 0)
			per = 0;
		else if (per > 60000)
			per = 60000;

		/* update BPM timer */

		pthread_mutex_lock(&mainWindow->mtx);
		mainWindow->dlg_bpm->ref = ref;
		mainWindow->dlg_bpm->period = per;
		mainWindow->dlg_bpm->handle_update();
		pthread_mutex_unlock(&mainWindow->mtx);

		goto next_char;
	}

	if (ps.line < MPP_MAX_LINES)
		playCommand[ps.line] = command;

	goto next_char;

parse_inline:

	/* handle macro label inlining, if any */

	label = getIntValue(1);

	if (label < 0 || label >= MPP_MAX_LABELS)
		goto next_char;

	jmp_line = jumpTable[label] - 1;
	if (jmp_line < 0)
		goto next_char;

	curr_ticks = 0;
	end_line = ps.line;

	if (end_line > MPP_MAX_LINES)
		end_line = MPP_MAX_LINES;

	while (jmp_line < end_line) {

		uint32_t x;

		if (jumpLabel[jmp_line] != 0)
			break;

		if (timer_ticks_pre[jmp_line] == 0 &&
		    timer_ticks_post[jmp_line] == 0)
			break;

		for (x = 0; x != MPP_MAX_SCORES; x++) {
			if (scores[jmp_line][x].dur == 0)
				continue;

			key = scores[jmp_line][x].key + transpose;
			if (key < 0 || key > 127)
				continue;

			dur = resolveDuration(jmp_line, end_line,
			    scores[jmp_line][x].dur);
			if (dur <= 0)
				continue;

			if (ps.mindex >= MPP_MAX_MERGE)
				break;

			merge[ps.mindex].ticks = curr_ticks;
			merge[ps.mindex].is_on = 1;
			merge[ps.mindex].key = key;
			merge[ps.mindex].channel = channel;
			ps.mindex++;

			merge[ps.mindex].ticks = curr_ticks + dur;
			merge[ps.mindex].is_on = 0;
			merge[ps.mindex].key = key;
			merge[ps.mindex].channel = channel;
			ps.mindex++;
		}

		curr_ticks += timer_ticks_pre[jmp_line] +
		    timer_ticks_post[jmp_line];

		jmp_line++;
	}

	goto next_char;

parse_label:

	label = getIntValue(1);

	if ((label >= 0) && (label < MPP_MAX_LABELS))
		jumpTable[label] = ps.line + 1;

	goto next_char;

parse_string:

	c = getChar(1);
	if (c != '\"')
		goto next_char;

	/* check if the current line already has a string */
	if (ps.line < MPP_MAX_LINES && visual[ps.line].pstr != NULL)
		newLine(1, MPP_JUMP_REL, 0);

	post_inc = 0;
	line_off = 0;

	while ((c = getChar(2)) != 0) {
		if (c == '\"') {
			newVisual();
			break;
		}
		if (c == '\r' || c == '\n') {
			/* drop character */
			parseAdv(1);
			continue;
		}
		if (c == '\n') {
			/* new line */
			parseAdv(1);
			continue;
		}
		if (c == '.' && getChar(3) == '(') {
			post_inc = 1;
		} else if (c == '.') {
			if (line_off < MPP_MAX_LINES)
				line_off++;
		} else if (c == '(') {
			if ((ps.line + line_off) < MPP_MAX_LINES) {
				chord_info[ps.line + line_off].start_col = ps.realCol + 2;
				chord_info[ps.line + line_off].chord_line = ps.realLine;
			}
		} else if (c == ')') {
			if ((ps.line + line_off) < MPP_MAX_LINES) {
				chord_info[ps.line + line_off].stop_col = ps.realCol + 2;
			}

			if (post_inc && (line_off < MPP_MAX_LINES))
				line_off++;

			post_inc = 0;
		}

		if (ps.bufIndex == (sizeof(bufData) - 1)) {
			/* wrap long line */
			newVisual();
		}
		bufData[ps.bufIndex] = c;
		ps.bufIndex++;
		parseAdv(1);
	}
	parseAdv(1);
	goto next_char;

parse_jump:
	flag = 0;

parse_jump_sub:

	c = getChar(1);
	if (c == 'R' || c == 'r') {
		c = getChar(2);
		parseAdv(1);
		flag |= 2;
		goto parse_jump_sub;
	}

	if (c == 'P' || c == 'p') {
		c = getChar(2);
		parseAdv(1);
		flag |= 1;
		goto parse_jump_sub;
	}

	if (c >= '0' && c <= '9') {
		label = getIntValue(1);

		if (label < 0 || label >= MPP_MAX_LABELS)
			goto next_char;

		/* check for relative jump */

		if (flag & 2)
			label += MPP_JUMP_REL - 1;

		resetAutoMelody();

	} else {
		/* no jump */

		label = MPP_JUMP_REL - 1;
	}

	newLine(1, label + 1, flag & 1);

	goto next_char;

done:
	/* check if the current line already has a string */
	if (ps.line < MPP_MAX_LINES && visual[ps.line].pstr != NULL)
		newLine(1, MPP_JUMP_REL, 0);

	newLine(0, 0, 0);

	if (ps.bufIndex != 0)
		newVisual();

	linesMax = ps.line;
	currPos = 0;
	lastPos = 0;

	isPlayKeyLocked = 0;

	handleParseSub(NULL, QPoint(0,0), 1.0);

	picMax = 0;
	for (z = 0; z != linesMax; z++) {
		if (visual[z].pic != NULL)
			picMax++;
	}

	if (picMax == 0) {
		viewScroll->setValue(0);
		viewScroll->setMaximum(0);
	} else {
		viewScroll->setValue(0);
		viewScroll->setMaximum(picMax - 1);
	}
	delete ps.ba;
}

void
MppScoreMain :: handleScoreFileNew()
{
	editWidget->setPlainText(QString());

	handleCompile();

	if (currScoreFileName != NULL) {
		delete (currScoreFileName);
		currScoreFileName = NULL;
	}

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);
}

void
MppScoreMain :: handleScoreFileOpenRaw(char *data, uint32_t len)
{
	handleScoreFileNew();

	editWidget->setPlainText(QString::fromUtf8(QByteArray(data, len)));

	handleCompile();

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);
}

int
MppScoreMain :: handleScoreFileOpenSub(QString fname)
{
	QString scores;

	handleScoreFileNew();

	currScoreFileName = new QString(fname);

	scores = MppReadFile(fname);

	editWidget->setPlainText(scores);

	handleCompile();

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);

	return (scores.isNull() || scores.isEmpty());
}

void
MppScoreMain :: handleScoreFileOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		MppHomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		MppHomeDirTxt = diag->directory().path();
		handleScoreFileOpenSub(diag->selectedFiles()[0]);
	}

	delete diag;
}

void
MppScoreMain :: handleScoreFileSave()
{
	if (currScoreFileName != NULL)
		MppWriteFile(*currScoreFileName, editWidget->toPlainText());
	else
		handleScoreFileSaveAs();
}

void
MppScoreMain :: handleScoreFileSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		MppHomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		MppHomeDirTxt = diag->directory().path();

		if (currScoreFileName != NULL)
			delete (currScoreFileName);

		currScoreFileName = new QString(diag->selectedFiles()[0]);

		if (currScoreFileName != NULL)
			handleScoreFileSave();

		mainWindow->handle_tab_changed(1);
	}

	delete diag;
}

int
MppScoreMain :: checkLabelJump(int pos)
{
	if ((pos < 0) || (pos >= 12) || (pos >= MPP_MAX_LABELS) ||
	    (jumpTable[pos] == 0))
		return (0);

	return (1);
}

int
MppScoreMain :: checkHalfPassThru(int key)
{
	static const uint8_t is_black[12] = {0,1,0,1,0,0,1,0,1,0,1,0};

	return ((key >= mid_next_key(baseKey, -1)) &&
	    (key <= mid_next_key(baseKey, +1)) &&
	    (is_black[((uint8_t)key) % 12U] ==
	     is_black[((uint8_t)baseKey) % 12U]));
}

/* must be called locked */
void
MppScoreMain :: handleLabelJump(int pos)
{
	if (checkLabelJump(pos) == 0)
		return;

	lastPos = currPos = jumpTable[pos] - 1;
	isPlayKeyLocked = 0;

	mainWindow->cursorUpdate = 1;

	mainWindow->handle_stop(1);
}

void
MppScoreMain :: handleChordsLoad(void)
{
	uint16_t pos;
	uint8_t x;
	uint8_t y;
	uint8_t ns;
	uint8_t nb;
	uint8_t nk;
	uint8_t chan;
	uint8_t score[12];
	uint8_t base[12];
	uint8_t key[12];

	for (x = 0; x != 128; x++) {
		pos = resolveJump(currPos);
		if (pos != 0) {
			/* set new position */
			currPos = pos - 1;
		} else {
			break;
		}
	}

	memset(score_future, 0, sizeof(score_future));

	nb = 0;
	nk = 0;
	ns = 0;
	chan = 0;

	for (y = 0; y != MPP_MAX_SCORES; y++) {
		if (scores[currPos][y].dur != 0 && ns < 12) {
			score[ns] = scores[currPos][y].key;
			chan = scores[currPos][y].channel;
			ns++;
		}
	}

	if (ns == 0)
		return;

	mid_sort(score, ns);

	for (x = 0; x != ns; x++) {
		if (x < num_base)
			base[nb++] = score[x];
		else
			key[nk++] = score[x];
	}

	if (nb != 0) {
		score_future[2].dur = 1;
		score_future[2].key = base[0];
		score_future[2].channel = chan;
		mid_trans(base, nb, 1);
		score_future[4].dur = 1;
		score_future[4].key = base[0];
		score_future[4].channel = chan;
		mid_trans(base, nb, 1);
		score_future[1].dur = 1;
		score_future[1].key = base[0];
		score_future[1].channel = chan;
		mid_trans(base, nb, 1);
		score_future[3].dur = 1;
		score_future[3].key = base[0];
		score_future[3].channel = chan;
		mid_trans(base, nb, 1);
	}

	if (nk != 0) {
		score_future[5].dur = 1;
		score_future[5].key = key[0];
		score_future[5].channel = chan;
		mid_trans(key, nk, 1);
		score_future[7].dur = 1;
		score_future[7].key = key[0];
		score_future[7].channel = chan;
		mid_trans(key, nk, 1);
		score_future[9].dur = 1;
		score_future[9].key = key[0];
		score_future[9].channel = chan;
		mid_trans(key, nk, 1);
		score_future[11].dur = 1;
		score_future[11].key = key[0];
		score_future[11].channel = chan;
		mid_trans(key, nk, 1);
		score_future[6].dur = 1;
		score_future[6].key = key[0];
		score_future[6].channel = chan;
		mid_trans(key, nk, 1);
		score_future[8].dur = 1;
		score_future[8].key = key[0];
		score_future[8].channel = chan;
		mid_trans(key, nk, 1);
		score_future[10].dur = 1;
		score_future[10].key = key[0];
		score_future[10].channel = chan;
		mid_trans(key, nk, 1);
	}

	lastPos = currPos;
	currPos++;

	if (currPos >= linesMax)
		currPos = 0;

	mainWindow->cursorUpdate = 1;
}

/* must be called locked */
uint8_t
MppScoreMain :: handleKeyRemovePast(struct MppScoreEntry *pn, uint32_t key_delay)
{
	int x;
	uint8_t chan;
	uint8_t retval = 0;

	chan = (synthChannel + pn->channel) & 0xF;

	for (x = 0; x != 24; x++) {
		if (score_past[x].dur != 0 &&
		    score_past[x].key == pn->key &&
		    score_past[x].channel == chan) {

			mainWindow->output_key(chan, pn->key, 0, key_delay, 0);

			/* kill past score */
			score_past[x].dur = 0;

			/* tell caller about leftover key presses */
			retval = 1;
		}
	}
	return (retval);
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressChord(int in_key, int vel, uint32_t key_delay)
{
	struct MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off >= 1 && off < 12) {

		if (pressed_future == 0 || lastPos == currPos) {
			pressed_future = 1;
			handleChordsLoad();
		}

		pn = &score_future[off];

		/* check for silent key */
		if (pn->dur == 0)
			return;

		/* remove key if already pressed */
		if (handleKeyRemovePast(pn, key_delay))
			key_delay++;

		score_past[off].channel = (synthChannel + pn->channel) & 0xF;
		score_past[off].key = pn->key;
		score_past[off].dur = 1;

		mainWindow->output_key(score_past[off].channel, score_past[off].key,
		    vel, key_delay, 0);

	} else if (off >= 13 && off < 24) {

		if (pressed_future == 1 || lastPos == currPos) {
			pressed_future = 0;
			handleChordsLoad();
		}

		pn = &score_future[off - 12];

		/* check for silent key */
		if (pn->dur == 0)
			return;

		/* remove key if already pressed */
		if (handleKeyRemovePast(pn, key_delay))
			key_delay++;

		score_past[off].channel = (synthChannel + pn->channel) & 0xF;
		score_past[off].key = pn->key;
		score_past[off].dur = 1;

		mainWindow->output_key(score_past[off].channel, score_past[off].key,
		    vel, key_delay, 0);
	} else {
		/* silent keys */
		return;
	}

	if (mainWindow->currScoreMain() == this)
		mainWindow->do_update_bpm();

	mainWindow->cursorUpdate = 1;
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressureChord(int in_key, int vel, uint32_t key_delay)
{
	struct MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off == 0 || off == 12)
		return;

	if (off >= 0 && off < 24) {

		pn = &score_pressure[off];

		if (pn->dur != 0) {
			outputKeyPressure(pn->channel, pn->key, vel);
		}
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyReleaseChord(int in_key, uint32_t key_delay)
{
	struct MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off == 0 || off == 12)
		return;

	if (off >= 0 && off < 24) {

		pn = &score_past[off];

		if (pn->dur != 0) {

			/* store score for pressure */
			score_pressure[off] = score_past[off];

			mainWindow->output_key(pn->channel, pn->key, 0, key_delay, 0);

			/* only release once */
			pn->dur = 0;
		}
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyPress(int in_key, int vel, uint32_t key_delay)
{
	struct MppScoreEntry *pn;
	uint32_t timeout = 0;
	uint32_t t_pre;
	uint32_t t_post;
	int out_key;
	int vel_old;
	int vel_other;
	uint16_t pos;
	uint8_t chan;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t delay;

	vel_old = vel;

 repeat:
	for (x = 0; x != 128; x++) {
		pos = resolveJump(currPos);
		if (pos != 0) {
			/* avoid infinite loops */
			if (timeout != 0)
				goto done;
			/* check for real jump */
			if ((pos - 2) != currPos)
				isPlayKeyLocked = 0;

			/* set new position */
			currPos = pos - 1;
		} else {
			break;
		}
	}

	switch (playCommand[currPos]) {
	case MPP_CMD_LOCK:
		whatPlayKeyLocked = in_key;
		isPlayKeyLocked = 1;
		break;
	case MPP_CMD_UNLOCK:
		isPlayKeyLocked = 0;
		break;
	default:
		break;
	}

	if (isPlayKeyLocked != 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	t_pre = timer_ticks_pre[currPos];
	t_post = timer_ticks_post[currPos];

	if (timer_was_active && t_pre == 0 && t_post == 0)
		goto done;

	timer_was_active = 0;

	decrementDuration();

	last_key = in_key;
	last_vel = vel;

	pn = &scores[currPos][0];

	for (x = y = 0; x != MPP_MAX_SCORES; x++, pn++) {
		if (pn->dur != 0)
			y++;
	}

	if (y >= 6) {
		z = y - 1;
		y = 3;
	} else if (y >= 3) {
		z = y - 1;
		y = 2;
	} else if (y >= 1) {
		y = 1;
		z = 0;
	} else {
		z = 0;
		y = 0;
	}

	if (y == 0) {
		/* fallthrough */
		vel_other = 0;
	} else {
		if (chordNormalize == 0)
			y = 1;
		vel_other = (vel_old * chordContrast) / (y * 128);
		if (vel_other > 127)
			vel_other = 127;
		else if (vel_other < 0)
			vel_other = 0;
	}

	pn = &scores[currPos][0];

	for (x = 0; x != MPP_MAX_SCORES; x++, pn++) {

		if (pn->dur != 0) {
			if (z == 0) {
				vel = vel_other;
			} else {
				vel = vel_old;
				z--;
			}

			out_key = (int)pn->key + (int)in_key - (int)baseKey;

			if (out_key < 0 || out_key > 127)
				continue;

			chan = (synthChannel + pn->channel) & 0xF;

			delay = mainWindow->noise8(delayNoise);

			if (setPressedKey(chan, out_key, pn->dur, delay))
				continue;

			mainWindow->output_key(chan, out_key, vel,
			    key_delay + timeout + delay, 0);
		}
	}

	lastPos = currPos;
	currPos++;

	if (currPos >= linesMax) {
		currPos = 0;
		isPlayKeyLocked = 0;
	}

	if (t_pre || t_post) {
		timeout += t_pre;
		decrementDuration(timeout);
		timeout += t_post;
		timer_was_active = 1;

		/* avoid infinite loops */
		if (currPos != 0)
			goto repeat;
	}

done:
	mainWindow->cursorUpdate = 1;

	if (mainWindow->currScoreMain() == this)
		mainWindow->do_update_bpm();
}

/* must be called locked */
void
MppScoreMain :: decrementDuration(uint32_t timeout)
{
	uint8_t out_key;
	uint8_t chan;
	uint8_t delay;
	uint8_t x;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if ((pressedKeys[x] & 0xFF) == 1) {

			out_key = (pressedKeys[x] >> 8) & 0xFF;
			chan = (pressedKeys[x] >> 16) & 0xFF;
			delay = (pressedKeys[x] >> 24) & 0xFF;

			/* clear entry */
			pressedKeys[x] = 0;

			mainWindow->output_key(chan, out_key, 0,
			    timeout + delay, 0);
		}

		if (pressedKeys[x] != 0)
			pressedKeys[x] --;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyRelease(int in_key, uint32_t key_delay)
{
	if (isPlayKeyLocked != 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	if (timer_was_active) {
		timer_was_active = 0;
		return;
	}

	decrementDuration(key_delay);

	lastPos = currPos;
}

void
MppScoreMain :: handleScorePrint(void)
{
#ifndef QT_NO_PRINTER
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg;
	QPoint orig;
	qreal scale_f;
	QString temp;

	/* make sure everything is up-to-date */

	handleCompile();

#ifdef __APPLE__
	printer.setOutputFormat(QPrinter::NativeFormat);
#else
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setFontEmbeddingEnabled(true);
	printer.setFullPage(true);
	printer.setResolution(600);

	if (currScoreFileName != NULL) {
		temp = *currScoreFileName;
		temp.replace(QString(".txt"), QString(".pdf"),
		     Qt::CaseInsensitive);

		printer.setOutputFileName(temp);
	} else {
		printer.setOutputFileName(QString("NewSong.pdf"));
	}

	printer.setColorMode(QPrinter::Color);
#endif

	dlg = new QPrintDialog(&printer, mainWindow);

	if(dlg->exec() == QDialog::Accepted) {

		orig = QPoint(printer.logicalDpiX() * 0.5,
			      printer.logicalDpiY() * 0.5);

		scale_f = ((qreal)printer.logicalDpiY()) / (qreal)visual_y_max;

		handleParseSub(&printer, orig, scale_f);
	}

	delete dlg;
#endif
}

int
MppScoreMain :: setPressedKey(int chan, int out_key, int dur, int delay)
{
	uint32_t temp;
	uint8_t y;

	dur &= 0xFF;
	out_key &= 0xFF;
	chan &= 0xFF;
	delay &= 0xFF;

	temp = dur | (out_key << 8) | (chan << 16) | (delay << 24);

	if (dur == 0) {
		/* release key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key information matches */
				/* clear key */
				pressedKeys[y] = 0;
			}
		}
		return (0);
	} else {
		/* pre-press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key already set */
				return (1);
			}
		}

		/* press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (pressedKeys[y] != 0)
				continue;	/* key in use */

			pressedKeys[y] = temp;	/* set key */

			return (0);
		}
		return (1);
	}
}

int
MppScoreMain :: handleCompile(int force)
{
	QString temp;

	temp = editWidget->toPlainText();

	if (temp != editText || force != 0) {
		editText = temp;

		visual_y_max = MPP_VISUAL_C_MAX +
		  (3 * mainWindow->defaultFont.pixelSize());

		pthread_mutex_lock(&mainWindow->mtx);

		handleParse(editText);

		pthread_mutex_unlock(&mainWindow->mtx);
#if 0
		viewWidgetSub->setMinimumWidth(maxScoresWidth);
#endif
		return (1);
	}
	return (0);
}

void
MppScoreMain :: watchdog()
{
	uint16_t x;
	uint16_t y;
	uint16_t max;
	uint16_t pos;
	uint16_t y_blocks;

	QTextCursor cursor(editWidget->textCursor());

	QTextEdit::ExtraSelection format;

	/* Highlight the next line to be played */

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, 1);

	format.cursor = cursor;
	format.format.setForeground(color_black);
	format.format.setBackground(color_grey);

	QList<QTextEdit::ExtraSelection> extras;
	extras << format;

	editWidget->setExtraSelections(extras);

	/* Compute scrollbar */

	pthread_mutex_lock(&mainWindow->mtx);
	pos = currPos;
	max = linesMax;
	pthread_mutex_unlock(&mainWindow->mtx);

	/* Compute alignment factor */

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;

	/* Compute picture index */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if (x >= pos) {
			if (y != 0)
				break;
		}
	}
	if (y == 0 || y > picMax)
		viewScroll->setValue(0);
	else
		viewScroll->setValue((y - 1) - ((y - 1) % y_blocks));
}

void
MppScoreMain :: handleScrollChanged(int value)
{
	pthread_mutex_lock(&mainWindow->mtx);
	picScroll = value;
	pthread_mutex_unlock(&mainWindow->mtx);

	viewWidgetSub->repaint();
}

void
MppScoreMain :: handleAlign(uint8_t *ptr, int nak, int limit)
{
	int x;
	int y;
	int temp;
	int min;
	int map[12];

	/* sort scores by value */

	mid_sort(ptr, nak);

	for (x = 0; x != 12; x++)
		map[x] = limit;

	for (x = nak - 1; x != -1; x--) {

		temp = ptr[x];
		y = temp % 12;

		while (temp < map[y])
			temp += 12;
		while (temp >= map[y])
			temp -= 12;

		if (temp < 0) {
			min = 0;
			ptr[x] = 255;
		} else {
			min = temp;
			ptr[x] = temp;
			map[y] = temp;
		}
	}
}

QString
MppScoreMain :: handleTextTranspose(const QString &str, int level, int sharp, int align)
{
	QByteArray out;

	char c;
	uint8_t score[MPP_MAX_SCORES];
	int score_index[MPP_MAX_SCORES];
	int nak = 0;
	int is_comment = 0;
	int skip = 0;
	int key;
	int y;

	memset(&ps, 0, sizeof(ps));

	ps.x = 0;
	ps.ba = new QByteArray(str.toUtf8());

top:
	while ((c = getChar(0)) != 0) {

		switch (c) {
		case 'A':
			if (skip || is_comment)
				break;
			key = A0;
			goto parse_score;
		case 'H':
			if (skip || is_comment)
				break;
		case 'B':
			if (skip || is_comment)
				break;
			key = H0;
			goto parse_score;
		case 'C':
			if (skip || is_comment)
				break;
			key = C0;
			goto parse_score;
		case 'D':
			if (skip || is_comment)
				break;
			key = D0;
			goto parse_score;
		case 'E':
			if (skip || is_comment)
				break;
			key = E0;
			goto parse_score;
		case 'F':
			if (skip || is_comment)
				break;
			key = F0;
			goto parse_score;
		case 'G':
			if (skip || is_comment)
				break;
			key = G0;
			goto parse_score;
		case 'S':
			if (skip || is_comment)
				break;
			if (getChar(1) == '\"') {
				out += "S\"";
				ps.x += 2;
				goto parse_string;
			}
			skip = 1;
			break;
		case '\n':
			if (nak != 0) {
				int x;
				int off;
				handleAlign(score, nak, align);
				for (x = off = 0; x != nak; x++) {
					if (score[x] > 127)
						continue;
					QString temp(mid_key_str[score[x]]);
					out.insert(score_index[x] + off, temp);
					off += temp.size();
				}
				nak = 0;
			}
		case ' ':
		case '\t':
			skip = 0;
			break;
		case '/':
			if (getChar(1) == '*') {
				skip = 0;
				is_comment++;
			}
			break;
		case '*':
			if (getChar(1) == '/') {
				skip = 0;
				is_comment--;
			}
			break;
		default:
			skip = 1;
			break;
		}
		out += (char)c;		
		ps.x++;
	}
	if (nak != 0) {
		int x;
		int off;
		handleAlign(score, nak, align);
		for (x = off = 0; x != nak; x++) {
			if (score[x] > 127)
				continue;
			QString temp(mid_key_str[score[x]]);
			out.insert(score_index[x] + off, temp);
			off += temp.size();
		}
		nak = 0;
	}
	delete ps.ba;
	return (QString::fromUtf8(out));

parse_score:

	parseAdv(1);

	/* add octave number */
	key += 12 * getIntValue(0);

	c = getChar(0);
	if (c == 'B' || c == 'b') {
		key--;
		parseAdv(1);
	}

	key += level;
	if (key >= 0 && key <= 127) {
		if (align > -1) {
			if (nak < MPP_MAX_SCORES) {
				score[nak] = key;
				score_index[nak] = out.size();
				nak++;
			}
		} else {
			out += mid_key_str[key];
		}
	}

	skip = 1;
	goto top;

parse_string:

	while (1) {
		int is_dot;

		c = getChar(0);
		if (c == '\"' || c == 0)
			goto top;

		if (c == '.' && getChar(1) == '(') {
			ps.x += 2;
			is_dot = 1;
		} else if (c == '(') {
			ps.x++;
			is_dot = 0;
		} else {
			out += (char)c;
			ps.x++;
			continue;
		}

		for (y = 0; ; y++) {
			c = getChar(y);

			/* check for end of string */
			if (c == '\"' || c == 0)
				goto top;
			if (c == ')') 
				break;
		}
		if (is_dot == 0 && getChar(y + 1) == '.') {
			is_dot = 1;
			y++;
		}

		if (is_dot == 0) {
			out += '(';
			continue;
		}

		/* compute end of chord information */
		y = ps.x + y + 1;
		break;
	}

	switch (getChar(0)) {
	case 'A':
		key = A0;
		break;
	case 'H':
	case 'B':
		key = H0;
		break;
	case 'C':
		key = C0;
		break;
	case 'D':
		key = D0;
		break;
	case 'E':
		key = E0;
		break;
	case 'F':
		key = F0;
		break;
	case 'G':
		key = G0;
		break;
	case ')':
		out += ".()";
		ps.x = y;
		goto parse_string;
	default:
		out += ".(?)";
		ps.x = y;
		goto parse_string;
	}

	ps.x++;

	c = getChar(0);

	if (c == '#') {
		key++;
		ps.x++;
	} else if (c == 'b') {
		key--;
		ps.x++;
	}

	key = (key + level) % 12;
	if (key < 0)
		key += 12;

	out += ".(";
	out += MppBaseKeyToString(key, sharp);

	while (1) {
		c = getChar(0);
		out += (char)c;
		ps.x++;

		if (c == ')')
			goto parse_string_rem;
		else if (c == '/')
			break;
	}

	switch (getChar(0)) {
	case 'A':
		key = A0;
		break;
	case 'H':
	case 'B':
		key = H0;
		break;
	case 'C':
		key = C0;
		break;
	case 'D':
		key = D0;
		break;
	case 'E':
		key = E0;
		break;
	case 'F':
		key = F0;
		break;
	case 'G':
		key = G0;
		break;
	default:
		out += "?)";
		ps.x = y;
		goto parse_string;
	}

	ps.x++;

	c = getChar(0);

	if (c == '#') {
		key++;
		ps.x++;
	} else if (c == 'b') {
		key--;
		ps.x++;
	}

	key = (key + level) % 12;
	if (key < 0)
		key += 12;

	out += MppBaseKeyToString(key, sharp);

parse_string_rem:
	while (ps.x < y) {
		c = getChar(0);
		if (c != '.')
			out += (char)c;
		ps.x++;
	}
	goto parse_string;
}

void
MppScoreMain :: handleScoreFileAlign(void)
{
	int align = spnScoreFileAlign->value();

	editWidget->setPlainText(
	    handleTextTranspose(
	        editWidget->toPlainText(), 0, 0, align)
	    );

	handleCompile();
}

void
MppScoreMain :: handleScoreFileStepUp(void)
{
	editWidget->setPlainText(
	    handleTextTranspose(
	        editWidget->toPlainText(), 1, 0, -1)
	    );

	handleCompile();
}

void
MppScoreMain :: handleScoreFileStepDown(void)
{
	editWidget->setPlainText(
	    handleTextTranspose(
	        editWidget->toPlainText(), -1, 0, -1)
	    );

	handleCompile();
}

void
MppScoreMain :: handleScoreFileSetSharp(void)
{
	editWidget->setPlainText(
	    handleTextTranspose(
	        editWidget->toPlainText(), 0, 1, -1)
	    );

	handleCompile();
}

void
MppScoreMain :: handleScoreFileSetFlat(void)
{
	editWidget->setPlainText(
	    handleTextTranspose(
	        editWidget->toPlainText(), 0, 0, -1)
	    );

	handleCompile();
}

void
MppScoreMain :: handleScoreFileExport(void)
{
	QTextCursor cursor(mainWindow->tab_import->editWidget->textCursor());

	cursor.beginEditBlock();
	cursor.insertText(doExport());
	cursor.endEditBlock();

	mainWindow->handle_make_tab_visible(mainWindow->tab_import->editWidget);
}

uint8_t
MppScoreMain :: isValidChordInfo(uint32_t line)
{
	if (line >= MPP_MAX_LINES)
		return (0);

	return (chord_info[line].stop_col > chord_info[line].start_col);
}


uint8_t
MppScoreMain :: handleEditLine(void)
{
	char *ptr = NULL;
	uint32_t row;
	uint16_t x;
	uint8_t retval = 1;
	int len;
	int update_chord;

	QTextCursor cursor(editWidget->textCursor());

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);

	row = cursor.blockNumber();

	for (x = 0; x != linesMax; x++) {
		if (realLine[x] == row) {

			QTextCursor chord;

			MppDecode dlg(mainWindow, 1);

			if (isValidChordInfo(x)) {

				chord = cursor;
				chord.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
				chord.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, chord_info[x].chord_line);
				chord.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, chord_info[x].start_col);
				chord.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1 + chord_info[x].stop_col - chord_info[x].start_col);

				ptr = MppQStringToAscii(chord.selectedText());
			}
			if (ptr)
				len = strlen(ptr);
			else
				len = 0;

			if (isValidChordInfo(x) && ptr[0] == '(' && ptr[len-1] == ')') {
				ptr[len-1] = 0;
				update_chord = 1;
				if (dlg.parseScoreChord(scores[x], ptr + 1, 1)) {
					break;
				}
			} else {
				update_chord = 0;
				if (dlg.parseScoreChord(scores[x], "", 1)) {
					break;
				}
			}

			if (dlg.exec() == QDialog::Accepted) {

				cursor.beginEditBlock();
				cursor.removeSelectedText();
				cursor.insertText(mainWindow->led_config_insert->text());
				cursor.insertText(dlg.getText());
				cursor.endEditBlock();

				if (update_chord) {
					chord.beginEditBlock();
					chord.removeSelectedText();
					chord.insertText(QString("("));
					chord.insertText(dlg.lin_edit->text().trimmed());
					chord.insertText(QString(")"));
					chord.endEditBlock();
				}

				retval = 0;
			}
			break;
		}
	}
	free(ptr);

	return (retval);
}

void
MppScoreMain :: handleReplace(void)
{
	QTextCursor cursor(editWidget->textCursor());

	MppReplace dlg(mainWindow, cursor.selectedText(),
	    cursor.selectedText());

	if (dlg.exec() == QDialog::Accepted) {

		editWidget->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

		while (editWidget->find(dlg.match)) {

			cursor = editWidget->textCursor();

			cursor.beginEditBlock();
			cursor.removeSelectedText();
			cursor.insertText(dlg.replace);
			cursor.endEditBlock();
		}
	}
}

/* must be called locked */
void
MppScoreMain :: outputControl(uint8_t ctrl, uint8_t val)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = synthChannel;

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (ctrl == 0x40 && mw->mutePedal[x] != 0)
					continue;
				if (mw->muteAllControl[x] != 0)
					continue;
				if (mw->check_synth(x, chan, 0) == 0)
					continue;
				mid_control(d, ctrl, val);
			}
			if (mw->check_record(chan, 0))
				mid_control(d, ctrl, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}

	if (ctrl == 0x40)
		mw->tab_loop->add_pedal(val);
}

/* must be called locked */
void
MppScoreMain :: outputKeyPressure(uint8_t chan, uint8_t key, uint8_t pressure)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t buf[4];

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	buf[0] = 0xA0;
	buf[1] = key & 0x7F;
	buf[2] = pressure & 0x7F;
	buf[3] = 0;

	/* the pressure event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_add_raw(d, buf, 3, 0);
			}
			if (mw->check_record(chan, 0))
				mid_add_raw(d, buf, 3, 0);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}

/* must be called locked */
void
MppScoreMain :: outputChanPressure(uint8_t chan, uint8_t pressure)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t buf[4];

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	buf[0] = 0xD0;
	buf[1] = pressure & 0x7F;
	buf[2] = 0;
	buf[3] = 0;

	/* the pressure event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_add_raw(d, buf, 2, 0);
			}
			if (mw->check_record(chan, 0))
				mid_add_raw(d, buf, 2, 0);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}

/* must be called locked */
void
MppScoreMain :: outputPitch(uint16_t val)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = synthChannel;

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->muteAllControl[x] != 0)
					continue;
				if (mw->check_synth(x, chan, 0) == 0)
					continue;
				mid_pitch_bend(d, val);
			}
			if (mw->check_record(chan, 0))
				mid_pitch_bend(d, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}
