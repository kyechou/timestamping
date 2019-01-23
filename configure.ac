##
## Package name, version, bug-report, package tarname, URL
##
AC_INIT([timestamping], [0], [kychou2@illinois.edu], [timestamping], [])


##
## Check distribution
##
AS_IF([test -f /etc/os-release], [. /etc/os-release; OS=$(echo $NAME | cut -f 1 -d ' ')],
      [type lsb_release >/dev/null 2>&1], [OS=$(lsb_release -si)],
      [test -f /etc/lsb-release], [. /etc/lsb-release; OS=$DISTRIB_ID],
      [test -f /etc/arch-release], [OS="Arch"],
      [test -f /etc/debian_version], [OS="Debian"],
      [test -f /etc/SuSe-release], [OS="openSUSE"],
      [test -f /etc/fedora-release], [OS="Fedora"],
      [test -f /etc/redhat-release], [OS="CentOS"],
      [type uname >/dev/null 2>&1], [OS="$(uname -s)"],
      [AC_ERROR([unable to determine the distribution])])


##
## Determine license install path
##
AS_IF([test "$OS" = "Debian" -o "$OS" = "Ubuntu"],
      [AC_SUBST([licensedir], [\${docdir}])],
      [AC_SUBST([licensedir], [\${datarootdir}/licenses/\${PACKAGE_TARNAME}])])


##
## Config files and directories
##
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([src/server.cpp])
AC_CONFIG_AUX_DIR([build-aux])


##
## Preset default flags
##
: ${CXXFLAGS="-O3"}
: ${LDFLAGS=""}
: ${LIBS=""}
: ${TESTCXXFLAGS="-O3"}
: ${TESTLDFLAGS=""}
: ${TESTLIBS=""}


##
## Language environments
##
AC_LANG(C++)


##
## Programs
##
AC_PROG_CXX


###
### Configure arguments
###
AC_ARG_ENABLE([coverage],
        AS_HELP_STRING([--enable-coverage], [Enable coverage testing using gcov]))
AC_ARG_ENABLE([asan],
        AS_HELP_STRING([--enable-asan], [Enable AddressSanitizer, LeakSanitizer,
                        and UndefinedBehaviorSanitizer]))
AC_ARG_ENABLE([tsan],
        AS_HELP_STRING([--enable-tsan], [Enable ThreadSanitizer]))
AS_IF([test "$enable_asan" = "yes" -a "$enable_tsan" = "yes"], [
       AC_ERROR([ASan and TSan cannot be used at the same time])
])


###
### Predefined functions
###
define([ADD_CXXFLAG], [
        AS_IF([test -z "$CXXFLAGS"], [CXXFLAGS="$1"],
              [AS_IF([test -n "${CXXFLAGS##*$1*}"], [CXXFLAGS="$CXXFLAGS $1"])])
        CPPFLAGS="$CXXFLAGS"])
define([ADD_LDFLAG], [
        AS_IF([test -z "$LDFLAGS"], [LDFLAGS="$1"],
              [AS_IF([test -n "${LDFLAGS##*$1*}"], [LDFLAGS="$LDFLAGS $1"])])])
define([ADD_LIB], [
        AS_IF([test -z "$LIBS"], [LIBS="$1"],
              [AS_IF([test -n "${LIBS##*$1*}"], [LIBS="$LIBS $1"])])])
define([ADD_TESTCXXFLAG], [
        AS_IF([test -z "$TESTCXXFLAGS"], [TESTCXXFLAGS="$1"],
              [AS_IF([test -n "${TESTCXXFLAGS##*$1*}"], [TESTCXXFLAGS="$TESTCXXFLAGS $1"])])])
define([ADD_TESTLDFLAG], [
        AS_IF([test -z "$TESTLDFLAGS"], [TESTLDFLAGS="$1"],
              [AS_IF([test -n "${TESTLDFLAGS##*$1*}"], [TESTLDFLAGS="$TESTLDFLAGS $1"])])])
define([ADD_TESTLIB], [
        AS_IF([test -z "$TESTLIBS"], [TESTLIBS="$1"],
              [AS_IF([test -n "${TESTLIBS##*$1*}"], [TESTLIBS="$TESTLIBS $1"])])])
define([SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG],[
        AC_MSG_CHECKING([if compiler flag $1 works])
        save_CXXFLAGS="$CXXFLAGS"
        ADD_CXXFLAG($1)
        pushdef(prog, [int main(void) { return 0; }])
        flag_test=0
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([prog])], [flag_test=1],[flag_test=0])
        popdef([prog])
        if test $flag_test = 1
        then
                AC_MSG_RESULT(yes)
                ADD_TESTCXXFLAG($1)
        else
                AC_MSG_RESULT(no)
                CXXFLAGS="$save_CXXFLAGS"
        fi
])
define([SAFE_ADD_LDFLAG_AND_TESTLDFLAG],[
        AC_MSG_CHECKING([if linker flag $1 works])
        save_LDFLAGS="$LDFLAGS"
        ADD_LDFLAG($1)
        pushdef(prog, [int main(void) { return 0; }])
        flag_test=0
        AC_LINK_IFELSE([AC_LANG_SOURCE([prog])], [flag_test=1],[flag_test=0])
        popdef([prog])
        if test $flag_test = 1
        then
                AC_MSG_RESULT(yes)
                ADD_TESTLDFLAG($1)
        else
                AC_MSG_RESULT(no)
                LDFLAGS="$save_LDFLAGS"
        fi
])
define([CHECK_COMPILER_FLAGS], [
        AC_MSG_CHECKING([if compiler flags "$1" work])
        save_CXXFLAGS="$CXXFLAGS"
        CXXFLAGS="$1"
        pushdef(prog, [int main(void) { return 0; }])
        flag_test=0
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([prog])], [flag_test=1],[flag_test=0])
        popdef([prog])
        CXXFLAGS="$save_CXXFLAGS"
        if test $flag_test = 1
        then
                AC_MSG_RESULT(yes)
        else
                AC_MSG_RESULT(no)
                AC_ERROR([failed to compile with flags "$1"])
        fi
])
define([CHECK_LINKER_FLAGS], [
        AC_MSG_CHECKING([if linker flags "$1" work])
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$1"
        pushdef(prog, [int main(void) { return 0; }])
        flag_test=0
        AC_LINK_IFELSE([AC_LANG_SOURCE([prog])], [flag_test=1], [flag_test=0])
        popdef([prog])
        LDFLAGS="$save_LDFLAGS"
        if test $flag_test = 1
        then
                AC_MSG_RESULT(yes)
        else
                AC_MSG_RESULT(no)
                AC_ERROR([failed to link with flags "$1"])
        fi
])
define([CHECK_LIB], [
        save_LIBS="$LIBS"
        AC_CHECK_LIB($1, $2, $3, $4)
        LIBS="$save_LIBS"
])


##
## Compiler and linker flags
##
AC_LANG_WERROR
#SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG([-Wall])
#SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG([-Wextra])
#SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG([-Werror])
SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG([-std=c++14])
ADD_CXXFLAG([-I${srcdir}/src $(pkg-config --cflags protobuf grpc)])
ADD_LDFLAG([-Wl,-s])
ADD_TESTCXXFLAG([-I${srcdir}/src])
ADD_TESTLDFLAG([-L.])
AS_IF([test "$enable_coverage" = "yes"], [
        CXXFLAGS="$(echo $CXXFLAGS | sed 's/-O[[0-9]]/-O0/g')"
        TESTCXXFLAGS="$(echo $TESTCXXFLAGS | sed 's/-O[[0-9]]/-O0/g')"
        ADD_CXXFLAG(-g)
        ADD_CXXFLAG(--coverage)
        ADD_TESTCXXFLAG(-g)
        ADD_TESTCXXFLAG(--coverage)
        ADD_LIB(--coverage)
        ADD_TESTLIB(--coverage)
        AS_IF([test -z "$GCOV"], [      # if GCOV is not defined or null
               AS_IF([test -z "${CXX##*g++*}"], [GCOV="$(which gcov)"],
                     [test -z "${CXX##*clang*}"], [GCOV="$(which llvm-cov) gcov"],
                     [AC_ERROR([unsupported compiler: $CXX])])],
              [test -n "${GCOV##/*}"], [        # else if GCOV is not an abs path
               GCOV_PATH="$(which $(echo $GCOV | cut -f 1 -d ' ') 2>/dev/null)"
               AS_IF([test -z "$GCOV_PATH"],
                     [AC_ERROR(["$(echo $GCOV | cut -f 1 -d ' ')" file not found])])
               GCOV="$(echo $GCOV | sed "s|[[^ ]]\+|$GCOV_PATH|")"])
        AC_MSG_CHECKING([for $(echo $GCOV | cut -f 1 -d ' ')])
        AS_IF([test -x "$(echo $GCOV | cut -f 1 -d ' ')"], [AC_MSG_RESULT(yes)], [
               AC_MSG_RESULT(no)
               AC_ERROR([$(echo $GCOV | cut -f 1 -d ' ') cannot be executed])])
        CXX_VER="$($CXX --version | grep -oE '[[0-9]]\.[[0-9]]\.[[0-9]]' | head -n1)"
        GCOV_VER="$($GCOV --version | grep -oE '[[0-9]]\.[[0-9]]\.[[0-9]]' | head -n1)"
        AS_IF([test "$CXX_VER" != "$GCOV_VER"],
              [AC_ERROR([$GCOV $GCOV_VER and $CXX $CXX_VER version mismatch])])
        AC_SUBST([GCOV], [$GCOV])
])
AS_IF([test "$enable_asan" = "yes"], [
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fsanitize=address)
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fsanitize=pointer-compare)
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fsanitize=pointer-subtract)
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fsanitize=leak)
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fno-omit-frame-pointer)
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fno-common)
        SAFE_ADD_LDFLAG_AND_TESTLDFLAG(-fsanitize=address)
        SAFE_ADD_LDFLAG_AND_TESTLDFLAG(-fsanitize=pointer-compare)
        SAFE_ADD_LDFLAG_AND_TESTLDFLAG(-fsanitize=pointer-subtract)
        SAFE_ADD_LDFLAG_AND_TESTLDFLAG(-fsanitize=leak)
])
AS_IF([test "$enable_tsan" = "yes"], [
        SAFE_ADD_CXXFLAG_AND_TESTCXXFLAG(-fsanitize=thread)
        SAFE_ADD_LDFLAG_AND_TESTLDFLAG(-fsanitize=thread)
])


##
## Check flags usability
##
CHECK_COMPILER_FLAGS([$CXXFLAGS])
CHECK_LINKER_FLAGS([$LDFLAGS])
CHECK_COMPILER_FLAGS([$TESTCXXFLAGS])
CHECK_LINKER_FLAGS([$TESTLDFLAGS])


##
## Libraries
##
AC_CHECK_HEADERS(grpcpp/grpcpp.h, [], [AC_ERROR([grpc is not installed])])
ADD_LIB($(pkg-config --libs protobuf grpc++ grpc uuid))
ADD_LIB(-lgrpc++_reflection)

#AC_CHECK_HEADERS(pthread.h, [], [AC_ERROR([pthread is not available])])
#CHECK_LIB(pthread, pthread_create, [], [AC_ERROR([pthread does not work])])
#ADD_CXXFLAG(-pthread)
#ADD_LIB(-pthread)
#AC_DEFINE([PCRE2_CODE_UNIT_WIDTH], [8])
#AC_CHECK_HEADERS(pcre2.h, [], [AC_ERROR([pcre2 is not installed])])
#CHECK_LIB(pcre2-8, pcre2_compile_8, [], [AC_ERROR([pcre2 does not work])])
#ADD_LIB(-lpcre2-8)


##
## Libraries for testing
##
#SORYU_CXXFLAGS="$CXXFLAGS"
#SORYU_LDFLAGS="$LDFLAGS"
#SORYU_LIBS="$LIBS"
#CXXFLAGS="$TESTCXXFLAGS"
#LDFLAGS="$TESTLDFLAGS"
#LIBS="$TESTLIBS"
#AC_CHECK_HEADERS(criterion/criterion.h, [], [AC_ERROR([criterion is not available])])
#CHECK_LIB(criterion, criterion_run_all_tests, [], [AC_ERROR([criterion does not work])])
#ADD_LIB(-lcriterion)
#AC_CHECK_HEADERS(soryu.h, [], [AC_ERROR([soryu is not available])])
#ADD_LIB(-lsoryu)
#TESTCXXFLAGS="$CXXFLAGS"
#TESTLDFLAGS="$LDFLAGS"
#TESTLIBS="$LIBS"
#CXXFLAGS="$SORYU_CXXFLAGS"
#LDFLAGS="$SORYU_LDFLAGS"
#LIBS="$SORYU_LIBS"


echo >& AS_MESSAGE_FD
AC_SUBST([TESTCXXFLAGS], [$TESTCXXFLAGS])
AC_SUBST([TESTLDFLAGS], [$TESTLDFLAGS])
AC_SUBST([TESTLIBS], [$TESTLIBS])
AC_OUTPUT
# vim: set ts=8 sw=8 et:
