set(SOURCES
	MatrixIO.cpp
	MatrixIO.hpp
	OBJReader.cpp
	OBJReader.hpp
	OBJWriter.cpp
	OBJWriter.hpp
)

# OpenUSD mesh I/O (optional): lossless USD reader/writer over OBJData.
if(POLYFEM_WITH_USD)
	list(APPEND SOURCES
		USDPlugins.hpp
		USDReader.cpp
		USDReader.hpp
		USDWriter.cpp
		USDWriter.hpp
	)
	# These two TUs include third-party OpenUSD headers, which do not satisfy
	# PolyFEM's strict -Werror set (e.g. -Werror=non-virtual-dtor on Sdf value
	# types). Don't treat warnings from USD headers as errors for these files.
	set_source_files_properties(USDReader.cpp USDWriter.cpp
		PROPERTIES COMPILE_OPTIONS "-Wno-error")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCES})
target_sources(polyfem PRIVATE ${SOURCES})
