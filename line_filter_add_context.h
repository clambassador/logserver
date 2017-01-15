#ifndef __LINE_FILTER_ADD_CONTEXT__H__
#define __LINE_FILTER_ADD_CONTEXT__H__

#include <set>

#include "abstract_line_filter.h"
#include "line_filter_compose.h"

class LineFilterAddContext : public LineFilterCompose {
public:
	LineFilterAddContext(AbstractLineFilter* next)
		: LineFilterCompose(next) {}
	virtual ~LineFilterAddContext() {}
	virtual void filter_lines(LineFilterResult *lfr) {
		lfr->add_context();
		_next->filter_lines(lfr);
	}

protected:
};

#endif  // __LINE_FILTER_ADD_CONTEXT__H__
