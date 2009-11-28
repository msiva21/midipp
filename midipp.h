/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_H_
#define	_MIDIPP_H_

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QTextCursor>
#include <QTimer>
#include <QKeyEvent>

#include <umidi20.h>

#define	MPP_MAX_LINES	1024
#define	MPP_MAX_NOTES	16
#define	MPP_MAX_LABELS	32
#define	MPP_MAX_QUEUE	32

struct MppNote {
	uint8_t key;
	uint8_t dur;
	uint8_t channel;
};

struct MppSoftc {
	struct MppNote ScNotes[MPP_MAX_LINES][MPP_MAX_NOTES];
	uint16_t ScJumpNext[MPP_MAX_LINES];
	uint16_t ScJumpTable[MPP_MAX_LABELS];
	uint16_t ScLinesMax;
	uint16_t ScCurrPos;
	uint8_t ScPressed[128];
	uint8_t ScInputEvents[MPP_MAX_QUEUE];
	uint8_t ScNumInputEvents;
	uint8_t ScCmdKey;

	uint8_t is_record_off;
	uint8_t is_pass_thru_off;
};

class MppMainWindow : public QWidget
{
	Q_OBJECT;

 public:
	MppMainWindow(QWidget *parent = 0);
	~MppMainWindow();

	void MidiInit(void);
	void MidiUnInit(void);

	struct MppSoftc main_sc;

	QGridLayout *main_gl;
	QTabWidget *main_tw;

	QTimer *watchdog;

	/* editor */

	QTextEdit *main_edit;

	/* tab <file> */

	QWidget *tab_file_wg;
	QGridLayout *tab_file_gl;

	QPushButton *but_new_file;
	QPushButton *but_open_file;
	QPushButton *but_save_file;
	QPushButton *but_save_file_as;
	QPushButton *but_print_file;
	QPushButton *but_configure;
	QPushButton *but_quit;

	/* tab <edit> */

	QWidget *tab_edit_wg;
	QGridLayout *tab_edit_gl;

	QLabel	*lbl_volume;
	QSpinBox *spn_volume;

	QLabel	*lbl_play_key;
	QSpinBox *spn_play_key;

	QLabel	*lbl_cmd_key;
	QSpinBox *spn_cmd_key;

	QPushButton *but_jump_0;
	QPushButton *but_jump_1;
	QPushButton *but_jump_2;
	QPushButton *but_jump_3;
	QPushButton *but_pass_thru;
	QPushButton *but_compile;
	QPushButton *but_record;
	QPushButton *but_play;

	QString *CurrFilename;

	/* MIDI stuff */
	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track;

	pthread_mutex_t mtx;

 public slots:
	void handle_quit();
	void handle_play_key_changed(int);
	void handle_cmd_key_changed(int);
	void handle_jump_0();
	void handle_jump_1();
	void handle_jump_2();
	void handle_jump_3();
	void handle_pass_thru();
	void handle_compile();
	void handle_record();
	void handle_play_press();
	void handle_play_release();
	void handle_watchdog();

 protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif	/* _MIDIPP_H_ */
