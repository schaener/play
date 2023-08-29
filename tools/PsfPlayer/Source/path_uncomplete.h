#ifndef _PATH_UNCOMPLETE_H_
#define _PATH_UNCOMPLETE_H_

#include "filesystem_def.h"
#include <stdexcept>

//Taken from boost's trac
//https://svn.boost.org/trac/boost/ticket/1976
//Anonymous poster
fs::path naive_uncomplete(const fs::path& path, const fs::path& base)
{
	if(path.has_root_path())
	{
		if(path.root_path() != base.root_path())
		{
			return path;
		}
		else
		{
			return naive_uncomplete(path.relative_path(), base.relative_path());
		}
	}
	else
	{
		if(base.has_root_path())
		{
			throw std::runtime_error("cannot uncomplete a path relative path from a rooted base");
		}
		else
		{
			typedef fs::path::const_iterator path_iterator;
			path_iterator path_it = path.begin();
			path_iterator base_it = base.begin();
			while(path_it != path.end() && base_it != base.end())
			{
				if(*path_it != *base_it) break;
				++path_it;
				++base_it;
			}
			fs::path result;
			for(; base_it != base.end(); ++base_it)
			{
				result /= "..";
			}
			for(; path_it != path.end(); ++path_it)
			{
				result /= *path_it;
			}
			return result;
		}
	}
}

#endif
