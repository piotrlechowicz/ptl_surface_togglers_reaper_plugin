#include "surface_togglers_utils.h"

namespace surface_togglers
{

std::string eraseSubString(std::string mainStr, const std::string& toErase, std::string::size_type position)
{
	std::string::size_type pos = mainStr.find(toErase, position);
	if (pos != std::string::npos)
	{
		mainStr.erase(pos, toErase.length());
	}
	return mainStr;
}

}
