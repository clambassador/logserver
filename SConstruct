for i in range(0, 10):
    print ""
common = Split("""
	       """)
mains = dict()
mains['logserver.cc'] = 'logserver'

libs = Split("""ncurses
	     pthread
	     """)
env = Environment(CXX="ccache clang++ -I. -pg", CPPFLAGS="-D_FILE_OFFSET_BITS=64 -Wall -g -pg --std=c++11 -pthread", LIBS=libs, CPPPATH=".")
env['ENV']['TERM'] = 'xterm'

Decider('MD5')
for i in mains:
	env.Program(source = [i] + common, target = mains[i])
