#include "allocore/io/al_HID.hpp"
#include "hidapi/hidapi.h"
#include <stdio.h>
namespace al{

struct HIDSetup{
	HIDSetup(){
		hid_init();
	}
	~HIDSetup(){
		hid_exit();
	}
} hidSetup;


class HID::Impl{
public:
	Impl()
	:	mHandle(NULL), mTimeout(0)
	{}
	
	~Impl(){
		close();
	}

	/*	
	"I'm not sure the actual criteria which determines whether shared mode is 
	required. Keyboards and mice cannot be opened at all in Windows using the 
	HID interface, and thus can't be opened using HIDAPI.

	On Linux, it depends on the implementation you use. For Linux/hidraw, 
	devices can be opened multiple times. On Linux/libusb, they can only be 
	opened once. Actually, opening it a second time will steal the device from
	the first instance. On mac, it allows multiple opens."
	
		signal11, https://github.com/signal11/hidapi/issues/23
	*/
	bool open(unsigned short vid, unsigned short pid, const wchar_t *ser){
		mHandle = hid_open(vid, pid, ser);
		//printf("%p\n", mHandle);
		if(opened()){
			nonblocking(true);
			return true;
		}
		return false;
	}

	bool open(const char * path){
		mHandle = hid_open_path(path);
		if(opened()){
			nonblocking(true);
			return true;
		}
		return false;
	}

	void close(){
		if(opened()) hid_close(mHandle);
	}

	void timeout(int msec){
		mTimeout = msec;
	}

	bool nonblocking(bool v){
		return 0 == hid_set_nonblocking(mHandle, v);
	}

	int read(unsigned char * data, size_t length){
		if(mTimeout == 0){		
			return hid_read(mHandle, data, length);
		}
		else{
			return hid_read_timeout(mHandle, data, length, mTimeout);
		}
	}
	
	bool opened() const {
		return NULL != mHandle; }

	#define RETURN_STRING(hid_func)\
		if(opened()){\
			wchar_t wstr[128];\
			if(0 == hid_func(mHandle, wstr, sizeof(wstr))){\
				return wstr;\
			}\
		}\
		return L""

	std::wstring manufacturer() const {		
		RETURN_STRING(hid_get_manufacturer_string);
	}

	std::wstring product() const {
		RETURN_STRING(hid_get_product_string);
	}

	std::wstring serialNumber() const {
		RETURN_STRING(hid_get_serial_number_string);
	}
	
	#undef RETURN_STRING

	static void print(hid_device_info& d){
		printf("Product:            %ls\n", d.product_string);
		printf("Manufacturer:       %ls\n", d.manufacturer_string);
		printf("Vendor, product ID: %#.4x, %#.4x\n", d.vendor_id, d.product_id);		
		printf("Serial number:      %ls\n", d.serial_number);
		printf("Path:               %s\n", d.path);
		//#ifndef AL_LINUX
		printf("Usage, page:        %#x, %#x\n", d.usage, d.usage_page);
		//#endif
	}

	static void printDevices(unsigned short vID, unsigned short pID){
		struct hid_device_info * begin = hid_enumerate(vID, pID);
		struct hid_device_info * d = begin;
		
		if(NULL != begin){
			while(NULL != d){
				print(*d);
				printf("\n");
				d = d->next;
			}
			hid_free_enumeration(begin);
		}
	}

private:
	hid_device * mHandle;
	int mTimeout;
};



HID::HID()
:	mImpl(new Impl)
{}

HID::~HID(){
	delete mImpl;
}

bool HID::open(unsigned short vid, unsigned short pid, const wchar_t *ser){
	return mImpl->open(vid, pid, ser);
}

bool HID::open(const char * path){
	return mImpl->open(path);
}

void HID::close(){
	mImpl->close();
}

void HID::timeout(int msec){
	mImpl->timeout(msec);
}

int HID::read(unsigned char * data, size_t length){
	return mImpl->read(data, length);
}

bool HID::opened() const {
	return mImpl->opened();
}

std::wstring HID::manufacturer() const {
	return mImpl->manufacturer();
}

std::wstring HID::product() const {
	return mImpl->product();
}

std::wstring HID::serialNumber() const {
	return mImpl->serialNumber();
}

void HID::printDevices(unsigned short vID, unsigned short pID){
	return Impl::printDevices(vID, pID);
}

} // al::
