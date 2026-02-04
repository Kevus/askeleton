Please set the enviromental variable ASKELETON_HOME to the source path.

Requirements (Ubuntu/Debian):
- clang-15
- llvm-15
- llvm-config-15
- libclang-15-dev
- (optional) g++-12 / gcc-12
- (optional) libboost-dev

Build (LLVM/Clang 15):
```
make
```

Optional sanitizer build (AddressSanitizer):
```
make clean
make CXXFLAGS='-std=c++20 -Wall -Wextra -Wcast-qual -Wwrite-strings -Wno-unused-parameter -Wdelete-non-virtual-dtor -fPIC -ffunction-sections -fdata-sections -fsanitize=address -fno-omit-frame-pointer' \
    DEBUG_FLAGS='' OPTIMIZATION_FLAGS='-O1' \
    CLANGLIBS="$(llvm-config-15 --ldflags --system-libs --libs | tr '\n' ' ') -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite -lclangRewriteFrontend -lclangSupport -fsanitize=address"
```

Quick sanity check:
```
scripts/sanity_types.sh
```

SUT showcase (example):
```
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp
```

USAGE: askeleton [options] <source0> [... <sourceN>]

OPTIONS:

ASkeleTon - Test harness generation for C/C++ programs:

  -extra-arg=<string>        - Additional argument to append to the compiler command line
  -extra-arg-before=<string> - Additional argument to prepend to the compiler command line
  -p=<string>                - Build path

Generic Options:

  -help                      - Display available options (-help-hidden for more)
  -help-list                 - Display list of available options (-help-list-hidden for more)
  -version                   - Display the version of this program

-p <build-path> is used to read a compile command database.

	For example, it can be a CMake build directory in which a file named
	compile_commands.json exists (use -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	CMake option to get this output). When no build path is specified,
	a search for compile_commands.json will be attempted through all
	parent paths of the first input file . See:
	http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html for an
	example of setting up Clang Tooling on a source tree.

<source0> ... specify the paths of source files. These paths are
	looked up in the compile command database. If the path of a file is
	absolute, it needs to point into CMake's source tree. If the path is
	relative, the current working directory needs to be in the CMake
	source tree and the file must be in a subdirectory of the current
	working directory. "./" prefixes in the relative files will be
	automatically removed, but the rest of a relative path must be a
	suffix of a path in the compile command database.


If you are working with C++ headers use the option -xc++ at the end.
Author: Kevin J. Valle-Gomez (kevin.valle@uca.es)
Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
