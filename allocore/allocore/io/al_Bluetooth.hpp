#ifndef INC_AL_BLUETOOTH_HPP
#define INC_AL_BLUETOOTH_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Bluetooth interface

	Author(s):
	Lance Putnam, 2014, putnam.lance@gmail.com
*/

#include <cstdio>
#include <memory> // unique_ptr
#include <string>
#include <vector>

namespace al{

/// \addtogroup allocore
/// @{

/// Bluetooth connection
class Bluetooth{
public:

	enum{
		SERVICE_MAJOR_MASK		= ((1<<11) - 1)<<13,
		DEVICE_MAJOR_MASK		= ((1<< 5) - 1)<< 8,
		DEVICE_MINOR_MASK		= ((1<< 6) - 1)<< 2
	};

	enum ServiceMajor{
		LIMITED_DISCOVERABLE	= 1<<13,
		POSITIONING				= 1<<16,
		NETWORKING				= 1<<17,
		RENDERING				= 1<<18,
		CAPTURING				= 1<<19,
		OBJECT_TRANSFER			= 1<<20,
		AUDIO					= 1<<21,
		TELEPHONY				= 1<<22,
		INFORMATION				= 1<<23
	};
	
	enum DeviceMajor{
		MISC					=  0<<8,
		COMPUTER				=  1<<8,
		PHONE					=  2<<8,
		NETWORK_ACCESS_POINT	=  3<<8,
		AUDIO_VIDEO				=  4<<8,
		PERIPHERAL				=  5<<8,
		IMAGING					=  6<<8,
		WEARABLE				=  7<<8,
		TOY						=  8<<8,
		HEALTH					=  9<<8,
		UNCATEGORIZED			= 31<<8
	};

	Bluetooth();
	Bluetooth(const Bluetooth& other);
	Bluetooth(const std::string& addr);
	
	~Bluetooth();


	/// Get channel
	int channel() const { return mChannel; }

	/// Returns whether a connection is open
	bool opened() const;


	/// Create an RFCOMM connection with a remote device

	/// This call will not return until the connection is opened or there is a
	/// failure.
	/// @param[in] addr		remote device address, e.g., "68:86:e7:02:fa:99"
	/// @param[in] chan		channel in [1,30] or -1 for automatic
	bool openRFCOMM(const std::string& addr, int chan=-1);
	
	/// Close connection

	/// This call will not return until the connection is closed or there is a
	/// failure.
	bool close();


	/// Send data (non-blocking)
	bool send(const std::vector<unsigned char>& buf);

	/// Send data (non-blocking)
	bool send(const unsigned char * buf, unsigned len);
	
	/// Receive data (non-blocking)
	bool recv(std::vector<unsigned char>& buf);


	/// Check if Bluetooth is available
	static bool available();

	/// Print information about detected devices
	static void printDevices();


	//bool find(Bluetooth& bt, const std::string& address);
	//static int find(std::vector<Bluetooth>& matches, const std::string& address);

	class Impl;

private:
	std::unique_ptr<Impl> mImpl;

	std::string mName;
	std::string mAddr;
	unsigned mClass; // class of device
	int mChannel;

	bool open(const std::string& addr);
	void print(FILE * fp=stdout);
};

/// @} // end allocore group

} // al::
#endif
