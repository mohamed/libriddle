# Author: Mohamed A. Bamakhrama <mohamed@alumni.tum.de>
# Licensed under 3-clause BSD license.
#
#.rst:
# FindGMP
# --------
#
# Find GNU Multiprecision (GMP) library.
#
#
# Input Variables
# ^^^^^^^^^^^^^^^
#
# The user may set ``GMP_ROOT`` to the path where GMP is installed
#
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   GMP_INCLUDE_DIR    - where to find gmp.h, etc.
#   GMP_LIBRARIES      - List of C libraries when using gmp.
#   GMPXX_LIBRARIES    - List of C++ libraries when using gmp.
#   GMP_FOUND          - True if gmp found.
#

find_path(GMP_INCLUDE_DIR
    NAMES
        gmp.h
    HINTS
        ${GMP_ROOT}
    )
mark_as_advanced(GMP_INCLUDE_DIR)

# Find the library
find_library(GMP_LIBRARIES NAMES gmp libgmp
    PATHS
        ${GMP_ROOT}
    DOC "GMP library"
    )
mark_as_advanced(GMP_LIBRARIES)

#find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx
#    PATHS
#        ${GMP_ROOT}
#    DOC "GMPXX library"
#    )
#mark_as_advanced(GMPXX_LIBRARIES)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GMP
    DEFAULT_MSG
    GMP_LIBRARIES GMP_INCLUDE_DIR
)
