#ifndef __LINE_FILTER_COMPOSE__H__
#define __LINE_FILTER_COMPOSE__H__

#include <memory>

#include "abstract_line_filter.h"

using namespace std;

class LineFilterCompose : public AbstractLineFilter {
public:
	LineFilterCompose(AbstractLineFilter* next) {
		_next = next;
	}
	virtual ~LineFilterCompose() {}
	virtual void filter_lines(LineFilterResult *lfr) {
		_next->filter_lines(lfr);
	}

protected:
	AbstractLineFilter* _next;
};

#endif  // __LINE_FILTER_KEYWORD__H__
