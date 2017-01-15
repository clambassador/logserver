#ifndef __LINE_FILTER_KEYWORD__H__
#define __LINE_FILTER_KEYWORD__H__

#include <set>
#include <string>

#include "abstract_line_filter.h"
#include "line_filter_compose.h"
#include "line_filter_keyword.h"
#include "line_filter_result.h"

using namespace std;

class LineFilterKeyword : public LineFilterCompose {
public:
	LineFilterKeyword(const LogLines& log_lines,
			  AbstractLineFilter* next)
		: LineFilterCompose(next), _log_lines(log_lines) {
		reset();
	}
	virtual ~LineFilterKeyword() {}
	virtual void set_keyword(const string& keyword) {
		_keyword = keyword;
		reset();
	}
	virtual void add_keyletter(const char& ch) {
		_keyword += ch;
		reset();
	}

	virtual void pop_keyletter() {
		if (!_keyword.length()) return;
		_keyword = _keyword.substr(0, _keyword.length() - 1);
		reset();
	}

	virtual string get_keyword() {
		return _keyword;
	}

	virtual void filter_lines(LineFilterResult *lfr) {
		update();
		lfr->intersect(_lines);
		LineFilterCompose::filter_lines(lfr);
	}

protected:
	virtual void update() {
		_start = _log_lines.match(_keyword, &_lines, _start);
	}

	virtual void reset() {
		_lines.clear();
		_start = _log_lines.match(_keyword, &_lines);
	}
	string _keyword;
	set<size_t> _lines;
	const LogLines& _log_lines;
	size_t _start;

};

#endif  // __LINE_FILTER_KEYWORD__H__
