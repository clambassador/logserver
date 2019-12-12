#ifndef __LOG_LINES__H__
#define __LOG_LINES__H__

#include <algorithm>
#include <cassert>
#include <memory>
#include <mutex>
#include <thread>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class LogLines {
public:
	 LogLines(istream& in) : _in(in.rdbuf()), _exit(false), _dirty(false) {
		 _reader_thread.reset(new thread(&LogLines::reader, this));
	 }

	 LogLines(int fd) : _in(cin.rdbuf()),  _fd(fd), _exit(false),
			    _dirty(false) {
		 _reader_thread.reset(new thread(&LogLines::reader, this));
	 }

	 LogLines(const string& filename)
			: _in(cin.rdbuf()), _filename(filename),
			  _exit(false), _dirty(false) {
		 _reader_thread.reset(new thread(&LogLines::reader, this));
	 }

	 virtual ~LogLines() {
		 _exit = true;
		 _reader_thread->join();
	 }

	 virtual size_t add_line(const string& line) {
		 unique_lock<mutex> ul(_m);

		 _lines.push_back(line);
		 _dirty = true;
		 return _lines.size() - 1;
	 }

	 virtual size_t insert_line(const string& line,
				  int pos) {
		 unique_lock<mutex> ul(_m);

		 _lines.insert(_lines.begin() + pos, line);
		 _dirty = true;
		 _really_dirty = true;
		 new_pin_locked(pos);
		 return pos;
	 }

	 virtual void lock() const {
		assert(_ul.get() == nullptr);
		_ul.reset(new unique_lock<mutex>(_m));
	 }

	 virtual bool is_locked() const {
		return (_ul.get() != nullptr);
	 }

	 virtual void unlock() const {
		assert(_ul.get() != nullptr);
		_ul.reset(nullptr);
	 }

	 virtual size_t match_locked(const string& keyword,
				     bool not_inverted,
				     set<size_t>* lines) const {
		 return match_locked(keyword, not_inverted, lines, 0);
	 }

	 virtual size_t match_locked(const string& keyword,
				     bool not_inverted,
				     set<size_t>* lines,
				     size_t start) const {
		 if (lines->empty()) {
			 for (size_t i = start; i < _lines.size(); ++i) {
				if (!not_inverted ^ match(keyword, _lines[i])) {
					lines->insert(i);
				}
			 }
		 } else {
			 set<size_t> result;
			 for (auto &x : *lines) {
				 if (!not_inverted ^ match(keyword, _lines[x])) {
					result.insert(x);
				 }
			 }
			 lines->swap(result);
		 }

		 return _lines.size();
	 }

	 virtual void get_lines_locked(const vector<size_t>& lines,
				       vector<string>* output) const {
		 for (const auto &x : lines) {
			 stringstream ss;
			 if (x != (size_t) -1)
				ss << " " << x << "\t";
			 ss << '\t';
			 if (x == -1) {
				 ss << " ~";
			 } else {
				 ss << _lines[x];
			 }
			 output->push_back(ss.str());
		 }
	 }

	 virtual void recent_result_locked(const vector<size_t>& lines) {
		 _cur_view = lines;
	 }

	 virtual void permafilter() {
		 unique_lock<mutex> ul(_m);

		 vector<string> contents;
		 for (const auto &x : _cur_view) {
			 contents.push_back(_lines[x]);
		 }
		 _lines = contents;
		 _dirty = true;
		 _really_dirty = true;
	 }

	 virtual size_t length() const {
		 unique_lock<mutex> ul(_m);
		 return length_locked();
	 }

	 virtual size_t length_locked() const {
		 return _lines.size();
	 }

	 virtual bool really_dirty() {
		 unique_lock<mutex> ul(_m);
		 if (_really_dirty) {
			 _dirty = false;
			 _really_dirty = false;
			 return true;
		 }
		 return false;
	 }

	 virtual bool dirty() {
		 unique_lock<mutex> ul(_m);
		 if (_dirty) {
			 _dirty = false;
			 return true;
		 }
		 return false;
	}

	virtual size_t find(size_t cur, size_t tab, const string& keyword) const {
		unique_lock<mutex> ul(_m);
		return _lines[cur].find(keyword, tab);		
	}

	virtual size_t rfind(size_t cur, size_t tab, const string& keyword) const {
		unique_lock<mutex> ul(_m);
		return _lines[cur].rfind(keyword, tab);		
	}

	virtual void save() const {
		 unique_lock<mutex> ul(_m);
		 ofstream fout("/tmp/logserver_save");
		 for (const auto &x : _lines) {
			 fout << x;
		 }
	}

	virtual void save_line(size_t line) const {
		 unique_lock<mutex> ul(_m);
		 ofstream fout("/tmp/logserver_save");
		 fout << _lines[line];
	}

	virtual void new_pin(size_t pos) {
		unique_lock<mutex> ul(_m);

		new_pin_locked(pos);
	}

	virtual void new_pin_locked(size_t pos) {
		set<size_t> result;
		for (auto &x: _pins) {
			if (x < pos) result.insert(x);
			else result.insert(x + 1);
		}
		result.insert(pos);
		_pins.swap(result);
	}

	virtual const set<size_t>& pins_locked() const {
		return _pins;
	}

protected:
	virtual bool match(const string& keyword,
			   const string& line) const {
		if (keyword == "") return true;
		auto it = search(line.begin(), line.end(),
				 keyword.begin(), keyword.end(),
				 [](char c1, char c2) {
					return tolower(c1) == tolower(c2)
					  || tolower(c1) == c2;
				 });
		return (it != line.end());
	}

	virtual void reader() {
		if (_fd > 0) {
			stringstream ss;
			while (!_exit) {
				char c;
				int r = ::read(_fd, &c, 1);
				if (r <= 0) break;
				ss << c;
				if (c == '\n') {
					string line = ss.str();
					add_line(line);
					ss.str(string());
				}
			}
		} else if (!_filename.empty()) {
			/* TODO: reopen, fstat, etc. */
			assert(0);
		} else {
			while (!_exit && _in.good()) {
				string line;
				getline(_in, line);
				add_line(line);
			}
		}
	}

	unique_ptr<thread> _reader_thread;
	istream _in;
	string _filename;
	int _fd = 0;
	mutable unique_ptr<unique_lock<mutex>> _ul;
	vector<string> _lines;
	vector<size_t> _cur_view;
	mutable mutex _m;
	bool _exit;
	bool _dirty;
	bool _really_dirty;
	set<size_t> _pins;
};

#endif  // __LOGLINES__H__
