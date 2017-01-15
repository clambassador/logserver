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
		update();
	}
	virtual ~LineFilterKeyword() {}
	virtual void set_keyword(const string& keyword) {
		_keyword = keyword;
		update();
	}
	virtual void add_keyletter(const char& ch) {
		_keyword += ch;
		update();
	}

	virtual void pop_keyletter() {
		if (!_keyword.length()) return;
		_keyword = _keyword.substr(0, _keyword.length() - 1);
		update();
	}

	virtual string get_keyword() {
		return _keyword;
	}

	virtual void filter_lines(LineFilterResult *lfr) {
		lfr->intersect(_lines);
		LineFilterCompose::filter_lines(lfr);
	}

protected:

	virtual void update() {
		_lines.clear();
		_log_lines.match(_keyword, &_lines);
	}
	string _keyword;
	set<size_t> _lines;
	const LogLines& _log_lines;

};

#endif  // __LINE_FILTER_KEYWORD__H__
