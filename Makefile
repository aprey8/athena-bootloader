# you can set this to 1 to see all commands that are being run
export VERBOSE := 0

ifeq ($(VERBOSE),1)
export Q :=
export VERBOSE := 1
else
export Q := @
export VERBOSE := 0
endif

BUILDRESULTS?=buildresults
CONFIGURED_BUILD_DEP = $(BUILDRESULTS)/build.ninja

.PHONY: all
all: makerelease

# Runs whenever the build has not been configured successfully
$(CONFIGURED_BUILD_DEP):
	$(Q) meson $(BUILDRESULTS)

.PHONY: makerelease
makerelease:
	$(Q)cd avr/bootloaders/athena/src/; ./makerelease

.PHONY: clean
clean:
	$(Q)make -C avr/bootloaders/athena/src/ clean

.PHONY: package
package:
	$(Q)tools/package.sh

.PHONY: dist
dist: $(CONFIGURED_BUILD_DEP)
	$(Q) ninja -C $(BUILDRESULTS) dist

.PHONY: format
format:
	$(Q)tools/format/clang-format-all.sh

.PHONY : format-diff
format-diff :
	$(Q)tools/format/clang-format-git-diff.sh

.PHONY : format-check
format-check :
	$(Q)tools/format/clang-format-patch.sh
