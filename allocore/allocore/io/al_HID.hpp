#ifndef INC_AL_HID_HPP
#define INC_AL_HID_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Human interface device

	Author(s):
	Lance Putnam, 2013, putnam.lance@gmail.com
*/

#include <wchar.h>
#include <string>
#include "allocore/system/al_Pimpl.hpp"

namespace al{

/// Accesses a human interface device (HID)
///
/// @ingroup allocore
class HID{
public:

	struct Info{
		unsigned short vendorID;
		unsigned short productID;
		const wchar_t * serialNumber;
	};

	HID();

	/// Open a HID device

	/// @param[in] vendorID		The Vendor ID (VID) of the device to open.
	/// @param[in] productID	The Product ID (PID) of the device to open.
	/// @param[in] serialNum	The Serial Number of the device to open (optionally NULL).
	/// \returns whether the device was opened.
	bool open(
		unsigned short vendorID,
		unsigned short productID,
		const wchar_t *serialNumber = NULL
	);

	/// Open a HID device
	bool open(const Info& info);

	/// Open a HID device

	/// @param[in] path			A platform specific path to the device,
	///							e.g., /dev/hidraw0
	bool open(const char * path);


	/// Close HID device
	void close();


	/// Set read timeout, in milliseconds

	/// @param[in] msec			Read timeout, in milliseconds or
	///							-1 for a blocking read.
	void timeout(int msec);


	/// Read an input report from the HID device

	/// The first byte will contain the report number if the device uses
	/// numbered reports.
	///
	/// @param[in] data		A buffer to put the read data into.
	/// @param[in] length	The number of bytes to read. For devices with
	///						multiple reports, make sure to read an extra byte
	///						for the report number.
	/// \returns the actual number of bytes read and -1 on error.
	int read(unsigned char * data, size_t length);

	/// Read all input reports in queue

	/// @param[in] onReport	Function called for each report.
	///						Signature is void(unsigned char *).
	/// \returns the actual number of bytes read and -1 on error.
	template <unsigned Nbuf, class OnReport>
	int readAll(OnReport onReport){
		unsigned char buf[Nbuf];
		int bytesRead = 0;
		int num;
		while((num = read(buf, sizeof(buf))) > 0){
			onReport(buf);
			bytesRead += num;
		}
		return num>=0 ? bytesRead : -1;
	}

	template <class OnReport>
	int readAll(OnReport onReport){ return readAll<64>(onReport); }


	/// Get whether the device has been opened
	bool opened() const;
	operator bool() const { return opened(); }

	/// Get manufacturer string
	std::wstring manufacturer() const;

	/// Get product string
	std::wstring product() const;

	/// Get serial number string
	std::wstring serialNumber() const;


	/// Find HID whose product name matches search term
	static Info find(const char * searchTerm);
	static Info find(const wchar_t * searchTerm);

	static void printDevices(unsigned short vendorID=0, unsigned short productID=0);

private:
	class Impl;
	Pimpl<Impl> mImpl;
};

} // al::
#endif
