FLAGS=-g -std=c++1z -Wall -Wextra
LEST_FLAGS=-Dlest_FEATURE_COLOURISE=1 -Dlest_FEATURE_AUTO_REGISTER=1
INCLUDE_FLAGS=-isystem./include/lest

.PHONY: default

default: test-either

test-either: test_either.cpp either.hpp either.ipp
	$(CXX) $(FLAGS) $(INCLUDE_FLAGS) $(LEST_FLAGS) test_either.cpp -o $@

.PHONY: test
test: test-either
	./test-either -p --order=random

clean:
	@rm -f test-either
