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
	 LogLines(istream& in) : _in(in.rdbuf()), _exit(false) {
		 _reader_thread.reset(new thread(&LogLines::reader, this));
	 }

	 virtual ~LogLines() {
		 _exit = true;
		 _reader_thread->join();
	 }

	 virtual void add_line(const string& line) {
		 unique_lock<mutex> ul(_m);

		 _lines.push_back(line);
	 }

	 virtual void lock() {
		assert(_ul.get() == nullptr);
		_ul.reset(new unique_lock<mutex>(_m));
	 }

	 virtual void unlock() {
		assert(_ul.get() != nullptr);
		_ul.reset(nullptr);
	 }

	 virtual void match(const string& keyword,
			    set<size_t>* lines) const {
		 for (size_t i = 0; i < _lines.size(); ++i) {
			if (match(keyword, _lines[i])) {
				lines->insert(i);
			}
		 }
	 }

	 virtual void get_lines(const vector<size_t>& lines,
				vector<string>* output) const {
		 for (const auto &x : lines) {
			 stringstream ss;
			 if (x != (size_t) -1)
				ss << x << "\t";
			 ss << '\t';
			 if (x == -1) {
				 ss << "~";
			 } else {
				 ss << _lines[x];
			 }
			 output->push_back(ss.str());
		 }
	 }

	 virtual size_t length() const {
		 return _lines.size();
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
		while (!_exit && _in.good()) {
			string line;
			getline(_in, line);
			add_line(line);
		}
	}

	unique_ptr<thread> _reader_thread;
	istream _in;
	unique_ptr<unique_lock<mutex>> _ul;
	vector<string> _lines;
	mutex _m;
	bool _exit;
};

#endif  // __LOGLINES__H__
