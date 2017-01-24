#ifndef __LINE_FILTER_RESULT__H__
#define __LINE_FILTER_RESULT__H__

class LineFilterResult {
public:
	LineFilterResult(size_t total) {
		for (size_t i = 0; i < total; ++i) {
			_lines.insert(_lines.end(), i);
		}
		_total = total;
		_is_conjunction = true;
	}

	~LineFilterResult() {
	}

	virtual void insert(const set<size_t>& to_add) {
		for (auto &x : to_add) _lines.insert(x);
	}

	virtual void intersect(const set<size_t>& to_restrict) {
		if (!_is_conjunction) return insert(to_restrict);

		set<size_t> result;
		for (auto &x : to_restrict) {
			if (_lines.count(x)) result.insert(x);
		}
		_lines = result;
	}

	virtual void add_context() {
		set<size_t> to_add;
		for (auto &x : _lines) {
			if (x > 0) to_add.insert(x - 1);
			if (x < _total - 1) to_add.insert(x + 1);
		}
		insert(to_add);
	}

	virtual void set_mode_disjunction() {
		_is_conjunction = false;
		clear();
	}

	virtual void clear() {
		_lines.clear();
	}

	virtual void build_display(vector<size_t>* output,
				   size_t center,
				   size_t radius) {
		auto x = _lines.lower_bound(center);
		size_t i;
		for (i = 0; i < radius; ++i, --x) {
			if (x == _lines.begin()) break;
		}
		for (int blanks = 0; blanks < radius - i; ++blanks) {
			output->push_back(-1);
		}
		for (size_t j = 0; j < i; ++j, ++x) {
			output->push_back(*x);
		}
		if (x == _lines.end()) output->push_back(-1);
		else output->push_back(*x++);
		for (size_t i = 0; i < radius; ++i) {
			if (x == _lines.end()) output->push_back(-1);
			else output->push_back(*x++);
		}
	}

protected:
	set<size_t> _lines;
	size_t _total;
	bool _is_conjunction;
};

#endif  // __LINE_FILTER_RESULT__H__
