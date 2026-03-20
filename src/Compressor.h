
/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <zlib.h>

class Compressor
{

  public:
	/** Compress a STL string using zlib with given compression level and return
	 * the binary data. */
	static std::string compress_string(const std::string_view &toBeCompressed, int compressionlevel = Z_DEFAULT_COMPRESSION);
	// int compressionlevel = Z_BEST_COMPRESSION);

	static std::string decompress_string(const std::vector<uint8_t> &compressed);
};
