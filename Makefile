#####################################################################
# CMake configuration
#####################################################################

ifdef ComSpec
GENERATOR      := "Microsoft Visual Studio 2019"
FixPath         = $(subst /,\,$1)
RM              = if exist $1 rmdir /S /Q $1
MKDIR           = if not exist $1 mkdir $1
else
GENERATOR      := "Unix Makefiles"
FixPath         = $1
RM              = rm -f -r $1
MKDIR           = mkdir -p $1
endif

#####################################################################
# Build type and directories
#####################################################################
BUILD_TYPE     ?= RelWithDebInfo
BUILD_DIR       = $(call FixPath,${CURDIR}/build_dir/${BUILD_TYPE})
INSTALL_DIR     = $(call FixPath,${CURDIR}/install_dir/${BUILD_TYPE})
CACHE           = $(call FixPath,${BUILD_DIR}/CMakeCache.txt)

#####################################################################
# Build tools: cmake, ctest, and lcov
#####################################################################
CMAKE          := cmake
CTEST          := ctest
LCOV           := lcov
GENHTML        := genhtml


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

all install test clean: ${CACHE}
	${CMAKE} --build ${BUILD_DIR} --target $@ -j 4

${CACHE}:
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
	$(call MKDIR,$@)

configure: | ${BUILD_DIR} ${INSTALL_DIR}
	cd ${BUILD_DIR} && ${CMAKE} ${CMAKE_CONFIG_ARGS}

distclean:
	$(call RM,$(call FixPath,${CURDIR}/build_dir))
	$(call RM,$(call FixPath,${CURDIR}/install_dir))
