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
		_start = 2;
		freopen("/dev/tty", "rw", stdin);
		initscr();
		assert(has_colors());
		start_color();
		init_pair(0, COLOR_WHITE, COLOR_BLACK);
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(5, COLOR_CYAN, COLOR_BLACK);
		init_pair(6, COLOR_YELLOW, COLOR_BLACK);
		init_pair(10, COLOR_BLACK, COLOR_WHITE);
		init_pair(11, COLOR_BLACK, COLOR_RED);
		init_pair(12, COLOR_BLACK, COLOR_BLUE);
		init_pair(13, COLOR_BLACK, COLOR_GREEN);
		init_pair(14, COLOR_BLACK, COLOR_MAGENTA);
		init_pair(15, COLOR_BLACK, COLOR_CYAN);
		init_pair(16, COLOR_BLACK, COLOR_MAGENTA);
		init_pair(100, COLOR_BLACK, COLOR_WHITE);
		init_pair(101, COLOR_RED, COLOR_WHITE);
		init_pair(102, COLOR_BLUE, COLOR_WHITE);
		init_pair(103, COLOR_GREEN, COLOR_WHITE);
		init_pair(104, COLOR_MAGENTA, COLOR_WHITE);
		init_pair(105, COLOR_CYAN, COLOR_WHITE);
		init_pair(106, COLOR_YELLOW, COLOR_WHITE);
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

			/* to see key names uncomment
			_ll->add_line(keyname(ch));
			*/

			process(ch);
		}
		refresh.join();
	}

protected:
	virtual void redraw() {
		int force = 50;
		while (!_exit) {
			this_thread::sleep_for(chrono::milliseconds(100));
			if (should_redraw()) {
				draw();
			}
			if (!(force--)) {
				draw();
				force = 50;
			}

		}
	}

	virtual bool should_redraw() {
		unique_lock<mutex> ul(_m);
		if (_start) {
			--_start;
			_ll->dirty();
			return true;
		}
		if (_ll->really_dirty()) {
			return true;
		}
		// BUG: redraw if near end of filter
		//      this causes fast input to not redraw
		//
		if (_navi.near_end(_N, RADIUS) && _ll->dirty()) {
			return true;
		}
		return false;
	}

	virtual void draw() {
		unique_lock<mutex> ul(_m);
		vector<FormatString> lines;
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
			ss << "[COMMAND]                    \t"
			   << _fr->mode_string() << "  "
			   << _fr->length() << " lines.  "
			   << "   keywords: ";
		} else if (_state == TYPE_MATCH) {
			ss << "> ";
			ss << _fr->current_keyword();
		} else if (_state == TYPE_COMMENT) {
			ss << "[enter comment] ";
			ss << _comment;
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
		if (ch == KEY_LEFT) _navi.left();
		if (ch == KEY_RIGHT) _navi.right();
		if (ch == KEY_PPAGE) _navi.page_up();
		if (ch == KEY_NPAGE) _navi.page_down();
		if (ch == KEY_HOME) _navi.start();
		if (ch == KEY_END) _navi.end();
		if (ch == KEY_SHOME) _navi.line_start();
		if (ch == KEY_SEND) _navi.line_end();
		if (ch == KEY_SRIGHT) _navi.goto_pos(_fr->find_next_match(_navi));
		if (ch == KEY_SLEFT) _navi.goto_pos(_fr->find_prev_match(_navi));

		if (_state == TYPE_MATCH) {
			if (ch == 27) {
				finish_type();
				pop_keyword();
			}
			if (ch == KEY_ENTER) finish_type();
			if (ch == '\n') finish_type();
			if (ch == KEY_BACKSPACE) pop_char();
			if (ch > 0 && ch < 256 && isprint(ch)) push_char(ch);
		} else if (_state == TYPE_COMMENT) {
			if (ch == 27) {
				finish_comment(false);
			}
			if (ch == KEY_ENTER) finish_comment(true);
			if (ch == '\n') finish_comment(true);
			if (ch == KEY_BACKSPACE) pop_char();
			if (ch > 0 && ch < 256 && isprint(ch)) push_char(ch);
		} else if (_state == COMMAND) {
			if (ch == '\t') toggle_mode();
			if (ch == KEY_BACKSPACE) pop_keyword();
			if (ch == 'q') _exit = true;
			if (ch == '/') start_match(true);
			if (ch == 'G') _navi.end();
			if (ch == 'T') _navi.start();
			if (ch == '#') start_comment();
			if (ch == '!') insert_dash_line();
			if (ch == '-') add_dash_line();
			if (ch == 'p') pin_line();
			if (ch == 'S') _ll->save();
			if (ch == 's') _ll->save_line(_navi.cur());
			if (ch == '\\') start_match(false);
			if (ch == '%') permafilter();
		}
	}

	virtual void pin_line() {
		if (_navi.at_end()) return;
		_ll->new_pin(_navi.cur());
	}

	virtual void toggle_mode() {
		_fr->toggle_mode();
	}

	virtual void permafilter() {
		_ll->permafilter();

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
		} else if (_state == TYPE_COMMENT) {
			_comment += ch;
		}
	}

	virtual void add_dash_line() {
		_ll->insert_line(
		    "---------------------------------------------",
		    _ll->length());
	}

	virtual void insert_dash_line() {
		if (_navi.at_end()) {
			add_dash_line();
			return;
		}
		size_t pos = _navi.cur();
		_ll->insert_line(
		    "---------------------------------------------",
		    pos);
	}

	virtual void start_comment() {
		_state = TYPE_COMMENT;
	}

	virtual void finish_comment(bool save) {
		_state = COMMAND;
		if (save) {
			string data = "# \t" + _comment;
			if (_navi.at_end()) {
				_ll->insert_line(data,
						 _ll->length());
			} else {
				_ll->insert_line(data,
						 _navi.cur());
			}
		}
		_comment = "";
	}

	virtual void pop_char() {
		if (_state == TYPE_MATCH) {
			if (!_fr->pop_char())
				finish_type();
		}
		if (_state == TYPE_COMMENT) {
			if (!_comment.empty()) {
				_comment = _comment.substr(
				    0, _comment.length() - 1);
			}
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

	virtual void start_match(bool not_inverted) {
		_state = TYPE_MATCH;
		_fr->start_match(not_inverted);
	}

	int _colour;
	map<string, int> _keyword_to_colour;
	bool _exit;
	int _state;
	int _start;
	const int COMMAND = 0;
	const int TYPE_MATCH = 1;
	const int TYPE_COMMENT = 2;
	const int PAGE = 20;
	const size_t RADIUS = 20;
	const size_t STATUS_LINE = 2 * RADIUS + 3;
	size_t _N;
	string _comment;
	LogLines* _ll;
	unique_ptr<FilterRunner> _fr;
	Navigation _navi;
	mutex _m;
};

#endif  //  __INTERFACE__H__
