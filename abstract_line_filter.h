#ifndef __ABSTRACT_LINE_FILTER__H__
#define __ABSTRACT_LINE_FILTER__H__

class LineFilterResult;

class AbstractLineFilter {
public:
	virtual ~AbstractLineFilter() {}
	virtual void filter_lines(LineFilterResult *lfr) = 0;
};

#endif  // __ABSTRACT_LINE_FILTER__H__
