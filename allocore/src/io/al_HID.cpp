#include "allocore/io/al_HID.hpp"
#include "hidapi/hidapi.h"
#include <stdio.h>
namespace al{

class HID::Impl{
public:
	Impl()
	:	mHandle(NULL)
	{
		if(refCount() == 0) hid_init();//init();
		refCount()++;
	}
	
	~Impl(){
		close();
		refCount()--;
		if(refCount() == 0) hid_exit();
	}

	bool open(unsigned short vid, unsigned short pid, const wchar_t *ser){
		mHandle = hid_open(vid, pid, ser);
		if(opened()){
			nonblocking(true);
			return true;
		}
		return false;
	}

	void close(){
		if(opened()) hid_close(mHandle);
	}

	bool nonblocking(bool v){
		return 0 == hid_set_nonblocking(mHandle, v);
	}

	int read(unsigned char * data, size_t length){
		return hid_read(mHandle, data, length);
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
		printf("Path:               %s\n", d.path);		
		printf("Vendor, product ID: %#.4x, %#.4x\n", d.vendor_id, d.product_id);
		printf("Manufacturer:       %ls\n", d.manufacturer_string);
		printf("Product:            %ls\n", d.product_string);
		printf("Serial number:      %ls\n", d.serial_number);
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

	static int& refCount(){
		static int count = 0;
		return count;
	}
	
	//static bool init(){
	//	static bool inited = false;
	//	if(!inited){
	//		if(0 == hid_init())
	//			inited = true;
	//	}
	//	return inited;
	//}
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

void HID::close(){
	mImpl->close();
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
