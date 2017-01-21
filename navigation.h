#ifndef __NAVIGATION__H__
#define __NAVIGATION__H__

#include <vector>

using namespace std;

class Navigation {
public:
	Navigation() : _cur(0), _tab(0), _max_length(0) {}

	virtual void set_view(const vector<size_t>& lines,
			      const vector<string>& data) {
		_lines = lines;
		_max_length = data[data.size() / 2].length();
	}

	virtual void line_start() {
		_tab = 0;
	}

	virtual void line_end() {
		_tab = (_max_length / 30) * 30 - 60;
		if (_tab > _max_length) _tab = 0;
	}

	virtual void start() {
		_cur = 0;
	}

	virtual void end() {
		_cur = POS_END;
	}

	virtual void left() {
		if (_tab >= 30) _tab -= 30;
		assert(!(_tab % 30));
	}

	virtual void right() {
		_tab += 30;
	}

	virtual size_t tab() {
		return _tab;
	}

	virtual void up() {
		set(safe_up(midpos() - 1));
	}

	virtual void down() {
		set(safe_down(midpos() + 1));
	}

	virtual void page_up() {
		set(safe_up(0));
	}

	virtual void page_down() {
		set(safe_down(_lines.size() - 1));
	}

	virtual size_t cur() const {
		return _cur;
	}

	virtual bool near_end(size_t size, size_t radius) const {
		//if (cur() == POS_END) return true;
		if (size - radius < cur()) return true;
		return false;
	}

protected:
	virtual size_t safe_up(size_t pos) {
		while (safe(pos) && _lines[pos] == -1) ++pos;
		if (!safe(pos)) return -1;
		return _lines[pos];
	}

	virtual size_t safe_down(size_t pos) {
		if (pos == POS_END) return _lines.size() - 1;
		while (safe(pos) && _lines[pos] == -1) --pos;
		if (!safe(pos)) return -1;
		return _lines[pos];
	}

	virtual bool safe(size_t pos) {
		return (pos < _lines.size());
	}

	virtual size_t midpos() {
		return (_lines.size() - 1) / 2;
	}

	virtual void set(size_t pos) {
		if (pos == -1) return;
		_cur = pos;
	}

	vector<size_t> _lines;
	size_t _cur;
	size_t _tab;
	size_t _max_length;
	const size_t POS_END = (size_t) -1;
};

#endif  // __FORMAT_STRING__H__
