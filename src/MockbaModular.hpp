#ifndef MOCKBAMODULAR_HPP
#define MOCKBAMODULAR_HPP

#ifdef bazinga
	#define BGCOLOR loadBack()
#else
	#define BGCOLOR "res/Empty_light.svg"
#endif

void saveBack(const char* Back);
const char* loadBack();

#endif