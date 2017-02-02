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

	 virtual void lock() {
		assert(_ul.get() == nullptr);
		_ul.reset(new unique_lock<mutex>(_m));
	 }

	 virtual void unlock() {
		assert(_ul.get() != nullptr);
		_ul.reset(nullptr);
	 }

	 virtual size_t match(const string& keyword,
			      set<size_t>* lines) const {
		 return match(keyword, lines, 0);
	 }
	 virtual size_t match(const string& keyword,
			      set<size_t>* lines,
			      size_t start) const {
		 for (size_t i = start; i < _lines.size(); ++i) {
			if (match(keyword, _lines[i])) {
				lines->insert(i);
			}
		 }
		 return _lines.size();
	 }

	 virtual void get_lines(const vector<size_t>& lines,
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

	 virtual size_t length() const {
		 return _lines.size();
	 }

	 virtual bool dirty() {
		 unique_lock<mutex> ul(_m);
		 if (_dirty) {
			 _dirty = false;
			 return true;
		 }
		 return false;
	 }

protected:

	virtual bool match(const string& keyword,
			   const string& line) const {
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
	unique_ptr<unique_lock<mutex>> _ul;
	vector<string> _lines;
	mutex _m;
	bool _exit;
	bool _dirty;
};

#endif  // __LOGLINES__H__
