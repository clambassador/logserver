#ifndef __FILTER_RUNNER__H__
#define __FILTER_RUNNER__H__

#include "abstract_line_filter.h"
#include "format_string.h"
#include "log_lines.h"
#include "line_filter_keyword.h"
#include "line_filter_add_context.h"
#include "line_filter_result.h"
#include "line_filter_terminus.h"
#include "navigation.h"

class FilterRunner {
public:
	FilterRunner(LogLines* ll) : _ll(ll) {
		_keywords.push_back(nullptr);
		_keywords.back().reset(new LineFilterTerminus());
		_add_context.push_back(nullptr);
		_add_context.back().reset(new LineFilterTerminus());
		_current_keyword = nullptr;
	}

	virtual ~FilterRunner() {}

	virtual void render(vector<FormatString>* output,
			    Navigation *navi,
			    size_t radius) {
		_ll->lock();

		LineFilterResult lfr(_ll->length());
		if (_filter_keywords != FILTER_NONE) {
			if (_filter_keywords == FILTER_OR) {
				lfr.set_mode_disjunction();
			}
			_keywords.front()->filter_lines(&lfr);
			_add_context.front()->filter_lines(&lfr);
		}
		lfr.insert(_pins);

		vector<size_t> lines;
		vector<string> data;

		lfr.build_display(&lines, navi->cur(), radius);
		_ll->get_lines(lines, &data);
		navi->set_view(lines, data);

		set_formatting(lines, data, output);
		set_line_position(output, navi->tab());

		_ll->unlock();
	}

	virtual void finish_match() {
		_keyword_vals.push_back(_current_keyword->get_keyword());
		_current_keyword = nullptr;
	}

	virtual void push_char(char ch) {
		assert(_current_keyword);
		_current_keyword->add_keyletter(ch);
	}

	virtual bool pop_char() {
		assert(_current_keyword);
		if (_current_keyword->get_keyword().empty()) return false;
		_current_keyword->pop_keyletter();
		return true;
	}

	virtual void pop_keyword() {
		if (_keyword_vals.empty()) return;
		_keyword_vals.pop_back();
		_keywords.pop_front();
	}

	virtual void start_match() {
		_current_keyword = new LineFilterKeyword(
		    *_ll, _keywords.front().get());

		_keywords.push_front(nullptr);
		_keywords.front().reset(_current_keyword);
		if (_filter_keywords == FILTER_NONE)
			set_mode_disjunction();
	}

	virtual string current_keyword() {
		assert(_current_keyword);
		return _current_keyword->get_keyword();
	}

	virtual void toggle_mode() {
		++_filter_keywords;
		_filter_keywords %= 3;
	}

	virtual void set_mode_none() {
		_filter_keywords = FILTER_NONE;
	}

	virtual void set_mode_conjunction() {
		_filter_keywords = FILTER_AND;
	}

	virtual void set_mode_disjunction() {
		_filter_keywords = FILTER_OR;
	}

	virtual void pin(size_t pos) {
		_pins.insert(pos);
	}

	virtual string mode_string() const {
		if (_filter_keywords == FILTER_NONE)
			return "ALL";
		if (_filter_keywords == FILTER_AND)
			return "AND";
		if (_filter_keywords == FILTER_OR)
			return " OR";
		return "NO SUCH MODE";
	}

protected:
	virtual void set_line_position(vector<FormatString>* output, size_t tab) {
		for (auto &x : *output) {
			x.set_start(tab);
		}
	}

	virtual void set_formatting(const vector<size_t>& pos,
				    const vector<string>& data,
				    vector<FormatString>* output) {
		assert(pos.size() == data.size());
		for (size_t i = 0; i < pos.size(); ++i) {
			FormatString fs;
			fs.init(data[i]);
			set_formatting(pos[i], data[i], &fs);
			if (i == (pos.size() - 1) / 2) {
				fs.highlight();
			}
			fs.colour_function();
			output->push_back(fs);
		}
	}

	virtual void set_formatting(size_t pos, const string& str,
				    FormatString *fs) {
		fs->assign(str);
		int code = 1;
		if (_current_keyword) {
			_keyword_vals.push_back(_current_keyword->get_keyword());
		}
		for (auto &x : _keyword_vals) {
			fs->mark(code++, x);
		}
		if (_current_keyword) {
			_keyword_vals.pop_back();
		}
	}

	LineFilterKeyword *_current_keyword;
	list<unique_ptr<AbstractLineFilter>> _keywords;
	vector<string> _keyword_vals;
	list<unique_ptr<AbstractLineFilter>> _add_context;
	set<size_t> _pins;
	int _filter_keywords = 0;
	LogLines* _ll;

	const int FILTER_AND = 0;
	const int FILTER_OR = 1;
	const int FILTER_NONE = 2;
};

#endif  // __FILTER_RUNNER__H__
