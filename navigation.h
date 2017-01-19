#ifndef __NAVIGATION__H__
#define __NAVIGATION__H__

#include <vector>

using namespace std;

class Navigation {
public:
	Navigation() : _cur(0) {}

	virtual void set_view(const vector<size_t>& lines) {
		_lines = lines;
	}

	virtual void start() {
		_cur = 0;
	}

	virtual void end() {
		_cur = POS_END;
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

	virtual size_t cur() {
		return _cur;
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
	const size_t POS_END = (size_t) -1;
};

#endif  // __FORMAT_STRING__H__
