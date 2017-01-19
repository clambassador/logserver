#ifndef __INTERFACE__H__
#define __INTERFACE__H__

#include <ncurses.h>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "filter_runner.h"
#include "log_lines.h"

using namespace std;

class Interface {
public:
	Interface(LogLines* ll) : _exit(false), _state(0) {
		_ll = ll;
		_fr.reset(new FilterRunner(_ll));
		_state = COMMAND;
		_colour = 1;
		freopen("/dev/tty", "rw", stdin);
		initscr();
		assert(has_colors());
		start_color();
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
		keypad(stdscr, true);
		noecho();
		cbreak();
	}

	virtual ~Interface() {
		_exit = true;
		endwin();
	}

	virtual void run() {
		string s;
		thread refresh(&Interface::redraw, this);
		while (!_exit) {
			draw();
			_N = _ll->length();

			int ch;
			ch = getch();

			process(ch);
		}
		refresh.join();
	}

protected:
	virtual void redraw() {
		while (!_exit) {
			this_thread::sleep_for(chrono::milliseconds(100));
			if (_ll->dirty()) {
				draw();
			}
		}
	}

	virtual void draw() {
		unique_lock<mutex> ul(_m);
		vector<FormatString> lines;
		lines.clear();
		_fr->render(&lines, &_navi, RADIUS);
		clear();
		for (size_t i = 0; i < lines.size(); ++i) {
			draw(i + 1, 0, lines[i]);
		}
		draw_status_bar();
		refresh();
	}

	virtual void draw(size_t y, size_t x, const FormatString& data) {
		for (int i = 0; i < data.length(); ++i) {
			int code = data.code(i);
			attron(COLOR_PAIR(code % 100));
			if (code >= 100) {
				attron(A_BOLD);
			}
			mvprintw(y, x + i, data.substr(i, 1).c_str());
			if (code >= 100) {
				attroff(A_BOLD);
			}
			attroff(COLOR_PAIR(code % 100));
		}
	}

	virtual void draw_status_bar() {
		stringstream ss;
		FormatString fs;
		if (_state == COMMAND) {
			ss << "[COMMAND]                    \tkeywords: ";
		} else if (_state == TYPE_MATCH) {
			ss << "> ";
			ss << _fr->current_keyword();
		}
		fs.add(ss.str(), 0);
		ss.str(string());
		for (auto &x : _keyword_to_colour) {
			ss << "[" << x.first <<  " " << x.second << "] ";
			fs.add(ss.str(), x.second);
			ss.str(string());
		}
		draw(STATUS_LINE, 0, fs);
	}

	virtual void process(int ch) {
		unique_lock<mutex> ul(_m);
		//printf("  state: %d key %d  ", _state, ch);
		if (ch == KEY_UP) _navi.up();
		if (ch == KEY_DOWN) _navi.down();
		if (ch == KEY_PPAGE) _navi.page_up();
		if (ch == KEY_NPAGE) _navi.page_down();
		if (ch == KEY_HOME) _navi.start();
		if (ch == KEY_END) _navi.end();

		if (_state == TYPE_MATCH) {
			if (ch == 27) {
				finish_type();
				pop_keyword();
			}
			if (ch == KEY_ENTER) finish_type();
			if (ch == '\n') finish_type();
			if (ch == KEY_BACKSPACE) pop_char();
			if (ch > 0 && ch < 256 && isprint(ch)) push_char(ch);
		} else if (_state == COMMAND) {
			if (ch == '\t') toggle_mode();
			if (ch == KEY_BACKSPACE) pop_keyword();
			if (ch == 'q') _exit = true;
			if (ch == '/') start_match();
			if (ch == 'G') _navi.end();
			if (ch == 'T') _navi.start();
			//if (ch == '#') select();
			//if (ch == '!') start_comment();
		}
	}

	virtual void toggle_mode() {
		_fr->toggle_mode();
	}

	virtual void finish_type() {
		if (_state == TYPE_MATCH) {
			string keyword = _fr->current_keyword();
			_fr->finish_match();
			_keyword_to_colour[keyword] = next_colour();
			_state = COMMAND;

			if (keyword.empty()) pop_keyword();
		}
	}

	virtual int next_colour() {
		return _colour++;
	}

	virtual int prev_colour() {
		return --_colour;
	}

	virtual void push_char(char ch) {
		if (_state == TYPE_MATCH) {
			_fr->push_char(ch);
		}
	}

	virtual void pop_char() {
		if (_state == TYPE_MATCH) {
			if (!_fr->pop_char())
				finish_type();
		}
	}

	virtual void pop_keyword() {
		_fr->pop_keyword();
		int colour = prev_colour();
		string keyword;
		for (auto &x : _keyword_to_colour) {
			if (x.second == colour) {
				keyword = x.first;
				break;
			}
		}
		_keyword_to_colour.erase(keyword);
	}

	virtual void start_match() {
		_state = TYPE_MATCH;
		_fr->start_match();
	}

	int _colour;
	map<string, int> _keyword_to_colour;
	bool _exit;
	int _state;
	const int COMMAND = 0;
	const int TYPE_MATCH = 1;
	const int PAGE = 20;
	const size_t RADIUS = 20;
	const size_t STATUS_LINE = 2 * RADIUS + 3;
	size_t _N;
	LogLines* _ll;
	unique_ptr<FilterRunner> _fr;
	Navigation _navi;
	mutex _m;
};

#endif  //  __INTERFACE__H__
