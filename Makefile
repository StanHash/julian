
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17 -O3

BUILDDIR = .build

SOURCES := \
  julian.cc \
  csv.cc \
  6502.cc \
  disasm.cc \
  anal.cc \
  print.cc \
  args.cc

OBJECTS := $(addprefix $(BUILDDIR)/,$(SOURCES:.cc=.o))

julian: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

$(BUILDDIR)/%.d: %.cc
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@ -c -MM -MP -MT $@ -MT $(BUILDDIR)/$*.o

$(BUILDDIR)/%.o: %.cc $(BUILDDIR)/%.d
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@ -c

clean:
	rm -r $(BUILDDIR)
	rm -f julian

.PHONY: clean

-include $(wildcard $(BUILDDIR)/*.d)
.PRECIOUS: $(BUILDDIR)/%.d
