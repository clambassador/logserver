#ifndef __NAVIGATION__H__
#define __NAVIGATION__H__

#include <vector>

using namespace std;

class Navigation {
public:
	Navigation() {}

	virtual void set_view(const vector<size_t>& lines) {
		_lines = lines;
	}

	virtual size_t up() {
		return safe_up(midpos() - 1);
	}

	virtual size_t down() {
		return safe_down(midpos() + 1);
	}

	virtual size_t page_up() {
		return safe_up(0);
	}

	virtual size_t page_down() {
		return safe_down(_lines.size() - 1);
	}

	virtual size_t cur() {
		if (safe(midpos())) return _lines[midpos()];
		return -1;
	}

protected:
	virtual size_t safe_up(size_t pos) {
		while (safe(pos) && _lines[pos] == -1) ++pos;
		if (!safe(pos)) return -1;
		return _lines[pos];
	}

	virtual size_t safe_down(size_t pos) {
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

	vector<size_t> _lines;
};

#endif  // __FORMAT_STRING__H__
