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

build/projects/vs2019:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools vs2019
	$(GENIE) --file=3rdparty/bimg/scripts/genie.lua --with-tools vs2019
	$(GENIE) --gfxapi=d3d11 --with-tools vs2019

windows-debug64:          \
	build/projects/vs2019 \
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "debug|x64" $(ARG_PREFIX)Project crown
windows-development64:    \
	build/projects/vs2019 \
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "development|x64" $(ARG_PREFIX)Project crown
windows-release64:        \
	build/projects/vs2019 \
	devenv.com build/projects/vs2019/crown.sln $(ARG_PREFIX)Build "release|x64" $(ARG_PREFIX)Project crown

ifeq ($(OS), linux)
	-@"$(MAKE)" -R -C 3rdparty/luajit/src clean -s
endif
	-@rm -rf 3rdparty/bgfx/.build
	-@rm -rf 3rdparty/bimg/.build
	-@rm -rf build
