#include <ncurses.h>

#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "log_lines.h"
#include "interface.h"

using namespace std;

int main(int argc, char** argv) {
	unique_ptr<LogLines> ll;
	ifstream fin;
	if (argc == 2) {
		fin.open(argv[1]);
		ll.reset(new LogLines(fin));
	} else {
		ll.reset(new LogLines(cin));
	}
	sleep(1);
	Interface interface(ll.get());
	interface.run();
	return 0;
}
