#ifndef __FORMAT_STRING__H__
#define __FORMAT_STRING__H__

#include <algorithm>
#include <string>

using namespace std;

class FormatString : public string {
public:
	FormatString() : string("") {}

	virtual void init(const string& val) {
		assign(val);
		for (size_t i = 0; i < length(); ++i) {
			_fmt.push_back(0);
		}
	}

	virtual void highlight() {
		for (size_t i = 0; i < length(); ++i) {
			_fmt[i] += 100;
		}
	}

	virtual void colour_function() {
		for (size_t i = 1; i < length(); ++i) {
			if (at(i) == ')' && at(i - 1) == '(') {
				size_t j;
				for (j = i - 2; j < length(); --j) {
					if (white_at(j)) break;
				}
				if (j >= length()) j = 0;
				mark(FUNCTION_COLOUR, j, i + 1 - j);
			}
		}
	}
	virtual void set_start(size_t pos) {
		assert(_fmt.size() == length());
		if (length() < pos) {
			_fmt.clear();
			assign("");
			return;
		}

		for (size_t i = pos; i < _fmt.size(); ++i) {
			_fmt[i - pos] = _fmt[i];
		}
		_fmt.resize(_fmt.size() - pos);
		assign(substr(pos));
		assert(_fmt.size() == length());
	}

	virtual void mark(int code, const string& keyword) {
		string lowerkeyword, lowerline, line;
		line.assign(static_cast<string>(*this));
		lowerkeyword.resize(keyword.length());
		lowerline.resize(length());
		transform(keyword.begin(), keyword.end(),
			  lowerkeyword.begin(), ::tolower);
		transform(line.begin(), line.end(),
			  lowerline.begin(), ::tolower);

		mark(code, lowerkeyword, line);
		mark(code, lowerkeyword, lowerline);
	}

	virtual void mark(int code, const string& keyword,
			  const string& line) {
		size_t start_pos = 0;
		while (npos !=
		       (start_pos = line.find(keyword, start_pos))) {
			mark(code, start_pos, keyword.length());
			++start_pos;
		}
	}

	virtual int code(size_t pos) const {
		return _fmt[pos];
	}

	virtual void add(const string& suffix, int code) {
		append(suffix);
		for (size_t i = 0; i < suffix.length(); ++i) {
			_fmt.push_back(code);
		}
	}

protected:
	virtual bool white_at(size_t i) const {
		assert(i < length());
		if (at(i) == ' ' || at(i) == '\t' || at(i) == '\n') return true;
		return false;
	}

	virtual void mark(int code, size_t pos, size_t len) {
		for (size_t i = pos; i < pos + len; ++i) {
			if (_fmt[i] == 0)
				_fmt[i] = code;
		}
	}

	const int FUNCTION_COLOUR = 4;

	vector<int> _fmt;
};

#endif  // __FORMAT_STRING__H__
