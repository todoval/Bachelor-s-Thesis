CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(json_fetcher)
include(ExternalProject)

set(JSON_PREFIX ${CMAKE_BINARY_DIR}/externals/json)

ExternalProject_Add(
	nlohmann_json
	PREFIX				${JSON_PREFIX}
	GIT_REPOSITORY		https://github.com/nlohmann/json.git
	GIT_TAG				ee4028b8e4cf6a85002a296a461534e2380f0d57
	CONFIGURE_COMMAND   ""
	BUILD_COMMAND       ""
	INSTALL_COMMAND		""
)

set(JSON_INCLUDE_DIRS ${JSON_PREFIX}/src/nlohmann_json/single_include/nlohmann)