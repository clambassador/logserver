#include <ncurses.h>

#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <csignal>

#include "pp_log_lines.h"
#include "log_lines.h"
#include "interface.h"

using namespace std;

int main(int argc, char** argv) {
	unique_ptr<LogLines> ll;
	ifstream fin;
	pid_t pid = 0;

	if (argc == 2) {
		fin.open(argv[1]);
		ll.reset(new PPLogLines(fin));
	} else {
		int pipe_ends[2];
		pipe(pipe_ends);
		pid = fork();
		if (pid == -1) throw string("fork(): failed");
		if (pid == 0) {
			close(pipe_ends[0]);
			string line;
			setpgid(0, 0);
			signal(SIGTERM, exit);
			while (true) {
				int c = fgetc(stdin);
				if (c == EOF) {
					::close(pipe_ends[1]);
					return 0;
				}
				char ch = (char) c;
				int r = write(pipe_ends[1], &ch, 1);
				if (r != 1) {
					::close(pipe_ends[1]);
					return 0;
				}
			}
		}
		close(pipe_ends[1]);
		ll.reset(new PPLogLines(pipe_ends[0]));
	}
	Interface interface(ll.get());
	interface.run();
	if (pid > 0) kill(pid, SIGABRT);
	return 0;
}
