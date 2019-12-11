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
			  bool not_inverted,
			  AbstractLineFilter* next)
		: LineFilterCompose(next), _ll(log_lines),
		  _not_inverted(not_inverted) {
		reset();
	}
	virtual ~LineFilterKeyword() {}
	virtual void set_keyword(const string& keyword) {
		_keyword = keyword;
		reset();
	}
	virtual void add_keyletter(const char& ch) {
		_keyword += ch;
		if (!_not_inverted) reset();
		else filter();
	}

	virtual void pop_keyletter() {
		if (!_keyword.length()) return;
		_keyword = _keyword.substr(0, _keyword.length() - 1);
		reset();
	}

	virtual string get_keyword() const {
		if (!_not_inverted) return "!" + _keyword;
		return _keyword;
	}

	virtual void filter_lines(LineFilterResult *lfr) {
		update();
		lfr->intersect(_lines);
		LineFilterCompose::filter_lines(lfr);
	}

protected:
	virtual void update() {
		assert(_ll.is_locked());
		_start = _ll.match_locked(_keyword, _not_inverted, &_lines, _start);
	}

	virtual void reset() {
		_lines.clear();
		filter();
	}

	virtual void filter() {
		assert(!_ll.is_locked());
		_ll.lock();
		_start = _ll.match_locked(_keyword, _not_inverted, &_lines);
		_ll.unlock();
	}
	string _keyword;
	set<size_t> _lines;
	const LogLines& _ll;
	size_t _start;
	bool _not_inverted;
};

#endif  // __LINE_FILTER_KEYWORD__H__
