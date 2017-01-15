#ifndef __LINE_FILTER_TERMINUS__H__
#define __LINE_FILTER_TERMINUS__H__

#include "abstract_line_filter.h"

class LineFilterTerminus : public AbstractLineFilter {
public:
	LineFilterTerminus() {}
	virtual ~LineFilterTerminus() {}
	virtual void filter_lines(LineFilterResult *lfr) {
		return;
	}
protected:
};

#endif  // __LINE_FILTER_TERMINUS__H__
