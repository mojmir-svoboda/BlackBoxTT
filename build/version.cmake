# VERSION_COMPUTE
# ---------------
#
# Deduce automatically the version number.
# This mechanism makes sure that version number is always up-to-date and
# coherent (i.e. strictly increasing as commits are made).
#
# There is two cases:
# - the software comes from a release (stable version). In this case, the
# software is retrieved through a tarball which does not contain the `.git'
# directory. Hence, there is no way to search in the Git history to generate
# the version number.
# In this case, a 'bbversion.h' file is put at the top-directory of the source
# tree which contains the project version.
#
# - the softwares comes from git (possibly unstable version).
# 'git describe' is used to retrieve the version number
# (see 'man git-describe'). This tool generates a version number from the git
# history. The version number follows this pattern:
#
# TAG[-N-SHA1][-dirty]
#
# TAG: last matching tag (i.e. last signed tag starting with v, i.e. v0.1)
# N: number of commits since the last maching tag
# SHA1: sha1 of the current commit
# -dirty: added if the workig directory is dirty (there is some uncommitted
# changes).
#
# For stable releases, i.e. the current commit is a matching tag, -N-SHA1 is
# omitted. If the HEAD is on the signed tag v0.1, the version number will be
# 0.1.
#
# If the HEAD is two commits after v0.5 and the last commit is 034f6d...
# The version number will be:
# - 0.5-2-034f if there is no uncommitted changes,
# - 0.5-2-034f-dirty if there is some uncommitted changes.
#

set(PROJECT_STABLE False)

# Check if a version is embedded in the project.
if(EXISTS ${CMAKE_SOURCE_DIR}/bbversion.h)
	# Yes, use it. This is a stable version.
	FILE(COPY ${CMAKE_SOURCE_DIR}/bbversion.h DESTINATION ${CMAKE_BINARY_DIR})
	SET(PROJECT_STABLE True)
else(EXISTS ${CMAKE_SOURCE_DIR}/bbversion.h)
	# No, there is no 'bbversion.h' file. Deduce the version from git.

	# Search for git.
	find_program(GIT git)
	if(NOT GIT)
		message("Warning: git not in system path!")
		set(PROJECT_VERSION UNKNOWN)
	endif()

	if (WIN32)
		find_program(GIT_EXECUTABLE
			NAMES "git.exe"
			HINTS "C:/Program Files (x86)/git/bin/"
			DOC "git x86 command line client")

		if (NOT GIT_FOUND)
			find_program(GIT_EXECUTABLE
				NAMES "git.exe"
				HINTS "C:/Program Files/git/bin/"
				DOC "git x64 command line client")
		endif (NOT GIT_FOUND)

		message("git: ${GIT_EXECUTABLE}")
	endif (WIN32)


	# Run describe: search for *signed* tags starting with v, from the HEAD and
	# display only the first four characters of the commit id.
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --abbrev=4 HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_DESCRIBE_RESULT
		OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
		ERROR_VARIABLE GIT_DESCRIBE_ERROR
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

	# Run diff-index to check whether the tree is clean or not.
	execute_process(
		COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_DIFF_INDEX_RESULT
		OUTPUT_VARIABLE GIT_DIFF_INDEX_OUTPUT
		ERROR_VARIABLE GIT_DIFF_INDEX_ERROR
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

	# Check if the tree is clean.
	if(NOT GIT_DIFF_INDEX_RESULT AND NOT GIT_DIFF_INDEX_OUTPUT)
		set(PROJECT_DIRTY False)
	else()
		set(PROJECT_DIRTY True)
	endif()

	# Check if git describe worked and store the returned version number.
	if(GIT_DESCRIBE_RESULT)
		message(
			"Warning: failed to compute the version number,"
			" 'git describe' failed:\n"
			"\t" ${GIT_DESCRIBE_ERROR})
		set(PROJECT_VERSION UNKNOWN)
	else()
		# Get rid of the tag prefix to generate the final version.
		string(REGEX REPLACE "^v" "" PROJECT_VERSION "${GIT_DESCRIBE_OUTPUT}")
		if(NOT PROJECT_VERSION)
			message(
				"Warning: failed to compute the version number,"
				"'git describe' returned an empty string.")
			set(PROJECT_VERSION UNKNOWN)
		endif()

		# If there is a dash in the version number, it is an unstable release,
		# otherwise it is a stable release.
		# I.e. 1.0, 2, 0.1.3 are stable but 0.2.4-1-dg43 is unstable.
		string(REGEX MATCH "-" PROJECT_STABLE "${PROJECT_VERSION}")
		if(NOT PROJECT_STABLE STREQUAL -)
			set(PROJECT_STABLE True)
		else()
			set(PROJECT_STABLE False)
		endif()
	endif()
	string(TIMESTAMP PROJECT_RELDATE "%Y-%m-%d")

	# Append dirty if the project is dirty.
	if(PROJECT_DIRTY)
		set(PROJECT_VERSION "${PROJECT_VERSION}-dirty")
	endif()
	file(WRITE bbversion.h "#define BB_VERSION \"${PROJECT_VERSION}\"\n")
	file(APPEND bbversion.h "#define BB_RELDATE \"${PROJECT_RELDATE}\"\n")
	file(APPEND bbversion.h "#define BB_VERSIONW L\"${PROJECT_VERSION}\"\n")
	file(APPEND bbversion.h "#define BB_RELDATEW L\"${PROJECT_RELDATE}\"\n")

	# FIXME: fill BB_NUMVERSION corectly (used for blackbox/resource.rc)
	file(APPEND bbversion.h "#define BB_NUMVERSION 1,20,0\n")
	file(APPEND bbversion.h "#define BBAPPNAME \"BlackboxTT\"\n")
	file(APPEND bbversion.h "#define BBAPPVERSION \"BlackboxTT ${PROJECT_VERSION}\"\n")
	file(APPEND bbversion.h "#define BBAPPNAMEW L\"BlackboxTT\"\n")
	file(APPEND bbversion.h "#define BBAPPVERSIONW L\"BlackboxTT ${PROJECT_VERSION}\"\n")

	file(WRITE ${CMAKE_SOURCE_DIR}/build.ver "${PROJECT_VERSION}\n")
	message("bb version : ${PROJECT_VERSION}")

endif(EXISTS ${CMAKE_SOURCE_DIR}/bbversion.h)
