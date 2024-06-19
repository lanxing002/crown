#
# Copyright (c) 2012-2024 Daniele Bartolini et al.
# SPDX-License-Identifier: MIT
#

OS=windows
EXE_PREFIX=
EXE_SUFFIX=.exe
ARG_PREFIX=/
MKDIR=mkdir

GENIE=scripts/genie/bin/$(OS)/genie
MAKE_JOBS=1

build/windows64/bin/luajit.exe:
	cd "3rdparty/luajit/src" && .\\msvcbuild.bat
	-@install -m775 -D 3rdparty/luajit/src/luajit.exe $@
	-@install -m664 -D 3rdparty/luajit/src/lua51.dll build/windows64/bin/lua51.dll
	-@install -m664 -D 3rdparty/luajit/src/lua51.lib build/windows64/bin/lua51.lib
	-@cp -r 3rdparty/luajit/src/jit build/windows64/bin


build/windows64/bin/texturec.exe: \
	build/projects/windows
	devenv.com 3rdparty/bimg/.build/projects/vs2019/bimg.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project texturec.vcxproj
	-@install -m775 -D 3rdparty/bimg/.build/win64_vs2019/bin/texturecRelease.exe $@
build/windows64/bin/shaderc.exe: \
	build/projects/windows
	devenv.com 3rdparty/bgfx/.build/projects/vs2019/bgfx.sln $(ARG_PREFIX)Build "Release|x64" $(ARG_PREFIX)Project shaderc.vcxproj
	-@install -m775 -D 3rdparty/bgfx/.build/win64_vs2019/bin/shadercRelease.exe $@


build/projects/vs2019:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools vs2019
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools vs2019
	$(GENIE) --gfxapi=d3d11 --with-tools --no-level-editor vs2019
windows-debug64:          \
	build/projects/vs2019 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "debug|x64" $(ARG_PREFIX)Project crown
windows-development64:    \
	build/projects/vs2019 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "development|x64" $(ARG_PREFIX)Project crown
windows-release64:        \
	build/projects/vs2019 \
	build/windows64/bin/luajit.exe
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "release|x64" $(ARG_PREFIX)Project crown


tools-windows-debug64:               \
	build/windows64/bin/texturec.exe \
	build/windows64/bin/shaderc.exe  \
	windows-debug64
tools-windows-release64:             \
	build/windows64/bin/texturec.exe \
	build/windows64/bin/shaderc.exe  \
	windows-development64

.PHONY: docs
docs:
	"$(MAKE)" -C docs/ html

SAMPLES_PLATFORM=$(OS)

.PHONY: 00-empty
00-empty: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 01-physics
01-physics: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 02-animation
02-animation: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)
.PHONY: 03-joypad
03-joypad: $(OS)-development64 tools-$(OS)-release64
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --source-dir $(realpath samples/$@) --map-source-dir core $(realpath samples) --compile --platform $(SAMPLES_PLATFORM)

.PHONY: samples
samples: 00-empty 01-physics 02-animation 03-joypad

.PHONY: run-00-empty
run-00-empty: 00-empty
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-01-physics
run-01-physics: 01-physics
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-02-animation
run-02-animation: 02-animation
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))
.PHONY: run-03-joypad
run-03-joypad: 03-joypad
	cd build/$(OS)64/bin && $(EXE_PREFIX)crown-development$(EXE_SUFFIX) --data-dir $(realpath samples/$<_$(OS))

.PHONY: clean-samples
clean-samples:
	-@rm -rf samples/00-empty_*
	-@rm -rf samples/01-physics_*
	-@rm -rf samples/02-animation_*
	-@rm -rf samples/03-joypad_*

.PHONY: codespell
codespell:
	@codespell docs src tools \
		--ignore-words=scripts/codespell-dictionary.txt \
		--skip "*.ttf.h,*.png,docs/_themes,tools/level_editor/resources/theme/Adwaita" \
		-q4 # 4: omit warnings about automatic fixes that were disabled in the dictionary.

.PHONY: cppcheck
cppcheck:
	@cppcheck src \
		--includes-file=scripts/cppcheck/includes.txt \
		--suppressions-list=scripts/cppcheck/suppressions.txt \
		--enable=all \
		--quiet \
		--force # Check all configurations.

.PHONY: format-sources
format-sources:
	@scripts/uncrustify/format-all.sh -j $(MAKE_JOBS) --enable-tools

.PHONY: create-meson-build
create-meson-build:
	$(GENIE) create-meson-build

.PHONY: clean
clean: clean-samples
	@echo Cleaning...
ifeq ($(OS), linux)
	-@"$(MAKE)" -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf 3rdparty/bimg/.build
	-@rm -rf build
