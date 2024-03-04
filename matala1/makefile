# List of subdirectories containing code and Makefile
SUBDIRS := Q1 Q2 Q3 Q4

# Default target
all: $(SUBDIRS)

# Recursively execute make in each subdirectory
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)

# Clean up generated files
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
