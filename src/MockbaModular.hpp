#ifndef MOCKBAMODULAR_HPP
#define MOCKBAMODULAR_HPP

using simd::float_4;

#define DETUNE 7.f

#ifdef _WIN32
	#ifndef ARCH_WIN
		#define ARCH_WIN                  // Just to make Visual Studio happy
	#endif
#endif

#ifdef ARCH_WIN
	#include <winsock2.h>                 // Needed for all Winsock stuff
	#define NOSOCKET INVALID_SOCKET
	#define NOBIND SOCKET_ERROR
#else
	#include <sys/select.h>               // Needed for sockets stuff
	#include <sys/socket.h>               // Needed for sockets stuff
	#include <arpa/inet.h>                // Needed for sockets stuff
	#include <fcntl.h>                    // Needed for sockets stuff
	#include <unistd.h>
	#define NOSOCKET -1
	#define NOBIND -1
#endif

#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef M_2PI
	#define M_2PI    6.28318530717958647692
#endif

template <typename T>
T expCurve(T x) {
	return (3 + x * (-13 + 5 * x)) / (3 + 2 * x);
}

#define BGCOLOR loadBack()

void saveBack(const std::string& Back);
std::string loadBack();
float randomFloat();

#endif
