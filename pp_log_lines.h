#ifndef __PP_LOG_LINES__H__
#define __PP_LOG_LINES__H__

#include <algorithm>
#include <cassert>
#include <memory>
#include <mutex>
#include <thread>
#include <sstream>
#include <string>
#include <vector>

#include "log_lines.h"

using namespace std;

class PPLogLines : public LogLines {
public:
	 PPLogLines(istream& in) : LogLines(in) {}
	 PPLogLines(int fd) : LogLines(fd) {}
	 PPLogLines(const string& filename) : LogLines(filename) {}

	 virtual ~PPLogLines() {
	 }

	 virtual size_t add_line(const string& line) {
		 unique_lock<mutex> ul(_m);

		 pretty_print_lines(line, &_lines);

		 _dirty = true;
		 return _lines.size() - 1;
	 }

	 virtual size_t insert_line(const string& line,
				  int pos) {
		 unique_lock<mutex> ul(_m);

		 vector<string> to_add;
		 pretty_print_lines(line, &to_add);
		 for (const auto &x : to_add) {
			 _lines.insert(_lines.begin() + (pos++), x);
		 }
		 --pos;
		 _dirty = true;
		 _really_dirty = true;
		 new_pin_locked(pos);
		 return pos;
	 }

protected:
	virtual void pretty_print_lines(const string& line,
				        vector<string>* out) const {
		if (line.length() < 150) {
			out->push_back(line);
			return;
		}
		size_t pos = 0, old_pos = 0;
		while (true) {
			old_pos = pos;
			pos = line.find("&", old_pos + 1);
			if (pos == string::npos) {
				out->push_back(line.substr(old_pos));
				break;
			}
			out->push_back(line.substr(old_pos, pos - old_pos));
		}
	}
};

#endif  // __PP_LOGLINES__H__
