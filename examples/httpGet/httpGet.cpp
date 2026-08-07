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

#include "CurlWrapper.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int iArgc, char *pArgv[])

{

	if (iArgc != 8)
	{
		std::cout << "wrong args number: " << iArgc << ", usage: " << pArgv[0] << " <url>"
			 << std::endl;

		return 1;
	}

	std::string url(pArgv[1]);

	CurlWrapper::GetInputParameters inputParameters {
		.url = url
	};
	CurlWrapper::OutputParameters outputParameters;

	std::cout << "httpGet: " << ": " << CurlWrapper::httpGet(inputParameters, outputParameters) << endl;

	return 0;
}
