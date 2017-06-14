//-*-Mode: C++;-*-
#ifndef _INFLOUtil_h_
#define _INFLOUtil_h_

#ifdef _WIN32
	#ifdef _INFLODLL_DEF
		#define INFLOEXPORT __declspec(dllexport)
	#else
		#define INFLOEXPORT __declspec(dllimport)
	#endif
#else
	#define INFLOEXPORT 
#endif

#endif
