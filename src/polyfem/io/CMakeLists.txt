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
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCES})
target_sources(polyfem PRIVATE ${SOURCES})
