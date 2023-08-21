#include "zCompiler.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <streambuf>
#include <set>
#include <map>
#include <string>

using namespace Zilla::Compiler;
using std::istreambuf_iterator;

inline static const std::map<std::string, enCompileFlags> s_flags =
{
	{"-lua", enCompileFlags::CF_LUA_COMPILE},
	{"-autorun", enCompileFlags::CF_AUTORUN},
	{"-token", enCompileFlags::CF_TOKEN_FILE}
};

int main(int argc, char* argv[])
try
{
	const char * outputName;
	uint8_t flags = 0;

	if(argc < 2)
		throw std::invalid_argument("No arguments passed to compiler. Ending.\n");

	try
	{
		for(int i = 2; i < argc; i++) // Sets flags passed as argument
			flags |= s_flags.at(std::string(argv[i]));
	}
	catch(const std::exception& e)
	{
		std::cout << "Invalid flags! Ending.\n";
		return 1;
	}

	std::ifstream file(argv[1]);

	compile({istreambuf_iterator<char>(file), istreambuf_iterator<char>()}, flags);
}
catch(compiler_exception& e)
{
	e.print();
}