#####################################################################
# Build type and directories
#####################################################################
BUILD_TYPE     ?= RelWithDebInfo
BUILD_DIR      ?= ${CURDIR}/build_dir/${BUILD_TYPE}
INSTALL_DIR    ?= ${CURDIR}/install_dir/${BUILD_TYPE}

#####################################################################
# Build tools: cmake, ctest, and lcov
#####################################################################
CMAKE          := cmake
CTEST          := ctest
LCOV           := lcov
GENHTML        := genhtml

#####################################################################
# CMake configuration
#####################################################################

ifdef ComSpec
GENERATOR      := "NMake Makefiles"
RM             := rmdir /S /Q
FixPath        = $(subst /,\,$1)
MKDIR          := mkdir
CC             := cl
CXX            := cl
else
GENERATOR      := "Ninja"
RM             := rm -f -r
FixPath        = $1
MKDIR          := mkdir -p
CC             ?= gcc
CXX            ?= g++
endif

CMAKE_CONFIG_ARGS   := -Wdev \
	-G ${GENERATOR} \
	-DCMAKE_C_COMPILER=${CC} \
	-DCMAKE_CXX_COMPILER=${CXX} \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
	-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT} \
	${CURDIR}

#####################################################################
# Rules
#####################################################################
.PHONY: distclean

all install test clean: ${BUILD_DIR}/CMakeCache.txt
	${CMAKE} --build ${BUILD_DIR} --target $@

${BUILD_DIR}/CMakeCache.txt:
	${MAKE} configure

coverage: test
	cd ${BUILD_DIR} && ${CTEST} -T coverage
	${LCOV} --directory ${BUILD_DIR} --capture \
		--output-file ${BUILD_DIR}/coverage.info --base-directory ${BUILD_DIR}
	${GENHTML} --title "Coverage Report" ${BUILD_DIR}/coverage.info \
		--output-directory ${BUILD_DIR}/coverage_report

memcheck: all
	cd ${BUILD_DIR} && ${CTEST} -T memcheck

${BUILD_DIR} ${INSTALL_DIR}:
	${MKDIR} $(call FixPath,$@)

configure: | ${BUILD_DIR} ${INSTALL_DIR}
	cd ${BUILD_DIR} && ${CMAKE} ${CMAKE_CONFIG_ARGS}

distclean:
	${RM} $(call FixPath,${CURDIR}/build_dir)
	${RM} $(call FixPath,${CURDIR}/install_dir)
