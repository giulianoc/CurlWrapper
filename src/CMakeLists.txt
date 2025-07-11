
# Copyright (C) Giuliano Catrambone (giulianocatrambone@gmail.com)

# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later 
# version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Commercial use other than under the terms of the GNU General Public
# License is allowed only after express negotiation of conditions
# with the authors.

SET (SOURCES
	Compressor.cpp
	Convert.cpp
	CurlWrapper.cpp
)

SET (HEADERS
	Compressor.h
	Convert.h
	CurlWrapper.h
)
include_directories("${SPDLOG_INCLUDE_DIR}")
include_directories("${NLOHMANN_INCLUDE_DIR}")
include_directories("${JSONUTILS_INCLUDE_DIR}")

add_compile_options(-DCOMPRESSOR)

if(APPLE)
	link_directories("${JSONUTILS_LIB_DIR}")
endif()

add_library (CurlWrapper SHARED ${SOURCES} ${HEADERS})

if(APPLE)
  target_link_libraries(CurlWrapper JSONUtils)
	target_link_libraries(CurlWrapper curl)
	target_link_libraries(CurlWrapper z)
endif()

if(ZORAC)
	install (TARGETS CurlWrapper DESTINATION services/cms-import)
	install (TARGETS CurlWrapper DESTINATION services/cms-pusher)
	install (TARGETS CurlWrapper DESTINATION services/cms-getter)
else()
	install (TARGETS CurlWrapper DESTINATION lib)
	install (FILES ${HEADERS} DESTINATION include)
endif()

