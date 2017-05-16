/*MIT License

Copyright (c) 2016 MTA SZTAKI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include "ApeSystem.h"

int main (int argc, char** argv)
{
	std::stringstream configDir;
	if (argc > 1)
	{
		std::string participantType = argv[1];
		if (participantType == "host")
			configDir << APE_SOURCE_DIR << "\\samples\\linkageDesignerVR\\configs\\host";
		else if (participantType == "guest")
			configDir << APE_SOURCE_DIR << "\\samples\\linkageDesignerVR\\configs\\guest";
		else if (participantType == "local")
			configDir << APE_SOURCE_DIR << "\\samples\\linkageDesignerVR\\configs\\local";
		else if (participantType == "local_cave")
			configDir << APE_SOURCE_DIR << "\\samples\\linkageDesignerVR\\configs\\local_cave";
		else if (participantType == "host_cave")
			configDir << APE_SOURCE_DIR << "\\samples\\linkageDesignerVR\\configs\\host_cave";
	}
	else
	{
		std::cout << "usage: host | guest | local | local_cave | host_cave " << std::endl;
		return 0;
	}
	Ape::System::Start(configDir.str(), true);
	Ape::System::Stop();
	return 0;
}
