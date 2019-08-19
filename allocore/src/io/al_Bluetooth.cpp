#include "allocore/io/al_Bluetooth.hpp"
#include <stdio.h>
#include <vector>

#define OS_LINUX	(defined(__linux__))
#define OS_WINDOWS	(defined(_WIN32) || defined(__WIN32__) || defined(_WIN64))

#if OS_LINUX

#include <unistd.h> // close
#include <sys/ioctl.h>
#include <sys/socket.h> // send, recv
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

/*
Some handy command-line tools for testing:

List detected bluetooth devices:
$ hcitool scan
$ hcitool inq

List services available on a particular device:
$ sdptool browse XX:XX:XX:XX:XX:XX

*/

struct al::Bluetooth::Impl{
	
	int mSocket;
	struct sockaddr_rc mSockAddr;


	Impl()
	:	mSocket(-1)
	{
		memset(&mSockAddr, 0, sizeof(mSockAddr));
	}


	void setDevice(const std::string& addr){
		mSockAddr.rc_family = AF_BLUETOOTH;
		str2ba(addr.c_str(), &mSockAddr.rc_bdaddr);
	}

	bool recv(std::vector<unsigned char>& buf){
		unsigned char readbuf[128];
		int read;
		int totalRead = 0;

		/* Normally, read and recv block until a message arrives on the socket.
		We can pass in MSG_DONTWAIT to tell recv to return immediately if there
		are zero bytes of data to be read.
		*/

		// Put the socket in non-blocking mode for BOTH send and recv:
		/*if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK) < 0) {
			// handle error
		}*/

		while((read = ::recv(mSocket, readbuf, sizeof(readbuf), MSG_DONTWAIT)) > 0){
			//printf("read %d bytes\n", read);
			totalRead += read;
			for(int i=0; i<read; ++i){
				buf.push_back(readbuf[i]);
			}
		}
		return bool(totalRead);

		/*while((read = ::read(mSocket, readbuf, sizeof(readbuf))) != 0){
			if(read > 0){
				for(int i=0; i<read; ++i){
					buf.push_back(readbuf[i]);
				}
			}
			else{
				return false;
			}
		}
		return true;*/
	}
	
	bool send(const unsigned char * buf, unsigned len){

		unsigned offset = 0;

		while(offset != len){
			int written = ::send(mSocket, buf + offset, len - offset, 0);
			if(written == -1){
				return false;
			}	
			else{
				offset += written;
			}
		}
		return true;
	}
	
	bool opened() const {
		return mSocket >= 0;
	}

	int openRFCOMM(int chan){

		// Create a reliable, stream-based connection (like TCP)
		mSocket = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		if(mSocket < 0){
			perror("Can't open RFCOMM socket");
			return false;
		}

		if(chan > 0){
			mSockAddr.rc_channel = chan;
		}
		else{ // choose channel automatically...
			// TODO: determine via SDP
			mSockAddr.rc_channel = 1;
		}

		if(::connect(mSocket, (struct sockaddr *) &mSockAddr, sizeof(mSockAddr)) < 0) {
			perror("Can't connect RFCOMM socket");
			close();
			return -1;
		}

		return mSockAddr.rc_channel;
	}


	bool open(){

		return false;
	}
	
	bool close(){
		if(mSocket > 0){
			::close(mSocket);
			mSocket = -1;
			return true;
		}
		return false;
	}

	
	static void getDevices(std::vector<al::Bluetooth>& devs){

		/*
		See cmd_inq and cmd_scan in tools/hcitool.c.
		*/

		/* From bluetooth/hci.h:
		typedef struct {
			bdaddr_t	bdaddr;
			uint8_t		pscan_rep_mode;
			uint8_t		pscan_period_mode;
			uint8_t		pscan_mode;
			uint8_t		dev_class[3];
			uint16_t	clock_offset;
		} __attribute__ ((packed)) inquiry_info;
		*/

		/*static const char *inq_help =
			"Usage:\n"
			"\tinq [--length=N] maximum inquiry duration in 1.28 s units\n"
			"\t    [--numrsp=N] specify maximum number of inquiry responses\n"
			"\t    [--iac=lap]  specify the inquiry access code\n"
			"\t    [--flush]    flush the inquiry cache\n";*/

		// First determine if there is a Bluetooth controller
		int dev_id = hci_get_route(NULL);
		if (dev_id < 0) {
			perror("Bluetooth not available");
			return;
		}

		inquiry_info *info = NULL;
		uint8_t lap[3] = { 0x33, 0x8b, 0x9e };
		int num_rsp = 16; 	// max number responses
		int length = 8; 	// max inquiry response time, in 1.28 s
		int flags = 0;
		char addr[18], name[249];

		num_rsp = hci_inquiry(dev_id, length, num_rsp, lap, &info, flags);
		if (num_rsp < 0) {
			perror("Bluetooth HCI inquiry failed.");
			return;
		}

		int dd = hci_open_dev(dev_id);
		if (dd < 0) {
			perror("Bluetooth HCI device open failed");
			bt_free(info);
			return;
		}

		Bluetooth bt;

		for (int i = 0; i < num_rsp; i++) {

			ba2str(&info[i].bdaddr, addr);

			if(hci_read_remote_name_with_clock_offset(dd,
					&info[i].bdaddr,
					info[i].pscan_rep_mode,
					info[i].clock_offset | 0x8000,
					sizeof(name), name, 100000) 
			< 0){
				strcpy(name, "n/a");
			}

			bt.mName = std::string(name);
			bt.mAddr = std::string(addr);
			bt.mClass= (unsigned(info[i].dev_class[2])<<16) | (unsigned(info[i].dev_class[1])<<8) | info[i].dev_class[0];
			devs.push_back(bt);
		}

		/**/

		bt_free(info);
	}


	static bool available(){
		return hci_get_route(NULL) >= 0;
	}

	static void printDevices(){
		std::vector<al::Bluetooth> devs;
		getDevices(devs);
		for(unsigned i=0; i<devs.size(); ++i) devs[i].print();
	}
};


#elif OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <stdlib.h> // For wcstombs
#include <winsock2.h>
//#define NO_GUID_DEFS
#include <initguid.h> // makes DEFINE_GUID macro _define_ variables
#include <ws2bth.h> // must be included after winsock2.h
//#include "bluetoothWindows/ws2bth.h"
#pragma comment (lib, "Ws2_32.lib")
#include <BluetoothAPIs.h> // must be included after ws2bth.h
#pragma comment (lib, "Bthprops.lib")
/*
typedef ULONGLONG BTH_ADDR;

typedef struct _BLUETOOTH_ADDRESS {
    union {
        BTH_ADDR ullLong;       //  easier to compare again BLUETOOTH_NULL_ADDRESS
        BYTE    rgBytes[ 6 ];   //  easier to format when broken out
    };

} BLUETOOTH_ADDRESS_STRUCT;

#define BLUETOOTH_ADDRESS BLUETOOTH_ADDRESS_STRUCT

typedef struct _SOCKADDR_BTH {
  USHORT   addressFamily;
  BTH_ADDR btAddr;
  GUID     serviceClassId;
  ULONG    port;
} SOCKADDR_BTH;
*/

struct al::Bluetooth::Impl{
	
	SOCKET mSocket;
	SOCKADDR_BTH mSockAddr;


	Impl()
	:	mSocket(INVALID_SOCKET)
	{
		memset(&mSockAddr, 0, sizeof(mSockAddr));

		// WSAStartup must be called before using Windows sockets
		WSADATA WSAData = { 0 };
		if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0){
			fprintf(stderr,"Error initializing sockets for Bluetooth\n");
		}
	}

	~Impl(){
		WSACleanup();
	}


	// integer byte 0 is LSB
	static void btaddr2str(char dst[18], const unsigned char src[6]){

		const char * hex_itoa = "0123456789abcdef";
		for(int i=0; i<6; ++i){
			//printf("%2x -> %x %x\n", src[5-i], src[5-i]>>4, src[5-i]&15);
			dst[i*3  ] = hex_itoa[src[5-i]>>4];
			dst[i*3+1] = hex_itoa[src[5-i]&15];
			if(i!=5) dst[i*3+2] = ':';
		}
		dst[17] = '\0';
	}

	// integer byte 0 is LSB
	static bool str2btaddr(unsigned char dst[6], const char * src){

		struct F{
			static char hex_atoi(char a){
				#define CS(l,r) case l: return r;
				switch(a){
				CS('0', 0) CS('1', 1) CS('2', 2) CS('3', 3) CS('4', 4)
				CS('5', 5) CS('6', 6) CS('7', 7) CS('8', 8) CS('9', 9)
				CS('a',10) CS('A',10) CS('b',11) CS('B',11)
				CS('c',12) CS('C',12) CS('d',13) CS('D',13)
				CS('e',14) CS('E',14) CS('f',15) CS('F',15)
				default: return 0;
				}
				#undef CS
			}
		};

		for(int i=0; i<6; ++i){
			char c1 = src[0]; if(!c1) return false;
			char c2 = src[1]; if(!c2) return false;
			dst[5-i] = (F::hex_atoi(c1)<<4) | F::hex_atoi(c2);
			//printf("%c %c -> %2x\n", c1,c2, dst[5-i]);
			src += 3;
		}
		return true;
	}

	void setDevice(const std::string& addr){
		mSockAddr.addressFamily = AF_BTH;
		BLUETOOTH_ADDRESS btaddr;
		btaddr.ullLong = 0;
		str2btaddr(btaddr.rgBytes, addr.c_str());
		mSockAddr.btAddr = btaddr.ullLong;
		//printf("%x%x\n", long(mSockAddr.btAddr>>32), long(mSockAddr.btAddr));
	}

	bool recv(std::vector<unsigned char>& buf){

		char readbuf[128];
		int totalBytesRecv = 0;

		while(true){
			int bytesRecv = ::recv(mSocket, readbuf, sizeof(readbuf), 0);
			if(bytesRecv > 0 && bytesRecv != SOCKET_ERROR){
				totalBytesRecv += bytesRecv;
				for(int i=0; i<bytesRecv; ++i){
					buf.push_back(readbuf[i]);
				}
			}
			else{
				break;
			}
		}

		return totalBytesRecv > 0;
	}
	
	bool send(const unsigned char * buf, unsigned len){
		int bytesSent = ::send(mSocket, reinterpret_cast<const char*>(buf), len, 0);
		if(SOCKET_ERROR == bytesSent) {
			if(GetLastError() == WSAEWOULDBLOCK)
				return true;
			else
				return false;
		}
		return true;
	}
	
	bool opened() const {
		return mSocket != INVALID_SOCKET;
	}


	static void printLastError(){
		int errID = WSAGetLastError();

		static const DWORD bufSize = 100+1;
		char buf[bufSize];
		if(FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, bufSize, NULL)
		){
			fprintf(stderr,"%s ", buf);
		}

		fprintf(stderr,"(WSAGetLastError = %d)\n\n", errID);
	}

	int openRFCOMM(int chan){
        // Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
        // Port should be set to 0 if ServiceClassId is specified.

		// Create a reliable, stream-based connection (like TCP)
		mSocket = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		if(INVALID_SOCKET == mSocket){
			fprintf(stderr, "Can't open RFCOMM socket:\n");
			printLastError();
			return -1;
		}

		/* typedef struct _GUID {
		  DWORD Data1;
		  WORD  Data2;
		  WORD  Data3;
		  BYTE  Data4[8];
		} GUID;	*/
		if(chan > 0){
			mSockAddr.port = chan;
			GUID guid = { 0 };
			//GUID guid = {0x00001101, 0x0000, 0x1000, {0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB}};
			mSockAddr.serviceClassId = guid;
		}
		else{ // choose channel automatically...
			// TODO: determine port (channel) via SDP
			mSockAddr.port = 0;
			// SerialPortServiceClass_UUID in bthdef.h:
			GUID guid = {0x00001101, 0x0000, 0x1000, {0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB}};
			// From RunClientMode in bthcxn.cpp:
			//GUID guid = {0xb62c4e8d, 0x62cc, 0x404b, {0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74}};
			// RFCOMM_PROTOCOL_UUID
			//GUID guid = {0x00000003, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
			//GUID guid = {0};
			mSockAddr.serviceClassId = guid;
		}
		//printf("Attempting to connect on channel %d:\n", chan);
		if(SOCKET_ERROR == ::connect(mSocket, (struct sockaddr *)&mSockAddr, sizeof(mSockAddr))) {
			// Error 10022: The parameter s (mSocket) is a listening socket.
			// Error 10051: The network cannot be reached from this host at this time.
			// Error 10060: The connection timed out.
			// For all error codes, see:
			//	http://msdn.microsoft.com/en-us/library/windows/desktop/ms737625%28v=vs.85%29.aspx

			//perror("Can't connect RFCOMM socket");
			fprintf(stderr, "Can't connect RFCOMM socket:\n");
			printLastError();
			close();
			return -1;
		}

		bool bBlocking = false;
		// Using decltyp vs. u_long here due to bug in Ming64-w64
		decltype(WSABUF::len) blockingMode = bBlocking ? 0 : 1; // Non-zero means non-blocking
		if(SOCKET_ERROR == ::ioctlsocket(mSocket, FIONBIO, &blockingMode)){
			printLastError();
			close();
			return -1;
		}

		/*struct SOCKADDR_BTH name;
		int namelen = sizeof(name);
		getsockname(mSocket, &name, &namelen);*/

		return mSockAddr.port;
	}


	bool open(){
		return false;
	}
	
	bool close(){
		if(opened()){
			::closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			return true;
		}
		return false;
	}

	
	static void getDevices(std::vector<al::Bluetooth>& devs){


		//BLUETOOTH_ADDRESS returnAddress = { BLUETOOTH_NULL_ADDRESS };
		/*
		typedef struct {
		  DWORD  dwSize;
		  BOOL   fReturnAuthenticated;
		  BOOL   fReturnRemembered;
		  BOOL   fReturnUnknown;
		  BOOL   fReturnConnected;
		  BOOL   fIssueInquiry;
		  UCHAR  cTimeoutMultiplier;	// in units of 1.28 seconds
		  HANDLE hRadio;				// NULL == search all local radios
		} BLUETOOTH_DEVICE_SEARCH_PARAMS;
		*/
		BLUETOOTH_DEVICE_SEARCH_PARAMS params = {
			sizeof(params),
			//TRUE, FALSE, FALSE, TRUE, FALSE, 0, NULL
			TRUE, FALSE, TRUE, TRUE, TRUE, 5, NULL
		};
		BLUETOOTH_DEVICE_INFO info = { sizeof(info) };

		HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&params, &info);

		if(NULL != hFind){
			char addr[18];
			char name[256];

			Bluetooth bt;

			do{
				//printf("%x%x\n", long(info.Address.ullLong>>32), long(info.Address.ullLong));
				btaddr2str(addr, info.Address.rgBytes);			
				wcstombs(name, info.szName, sizeof(name));

				//printf("Found %s (%s)\n", addr, name);
				//wprintf(L"%s)\n", info.szName);

				bt.mName = name;
				bt.mAddr = addr;
				bt.mClass= info.ulClassofDevice;
				devs.push_back(bt);
			} while(BluetoothFindNextDevice(hFind, &info));
		}

		BluetoothFindDeviceClose(hFind);
	}


	static bool available(){
		bool res = false;
		HANDLE info;
		BLUETOOTH_FIND_RADIO_PARAMS params;
		params.dwSize = sizeof(params);
		HBLUETOOTH_RADIO_FIND find = BluetoothFindFirstRadio(&params, &info);
		if(find != 0) {
			res = true;
			CloseHandle(info);
		}
		BluetoothFindRadioClose(find);
		return res;
	}

	static void printDevices(){
		std::vector<al::Bluetooth> devs;
		getDevices(devs);
		for(unsigned i=0; i<devs.size(); ++i) devs[i].print();
	}
};

#endif


#if OS_LINUX || OS_WINDOWS

namespace al{

Bluetooth::Bluetooth()
:	mImpl(new Impl), mChannel(-1)
{}

Bluetooth::Bluetooth(const Bluetooth& other)
:	mImpl(new Impl),
	mName(other.mName), mAddr(other.mAddr), mClass(other.mClass),
	mChannel(-1)
{
}

Bluetooth::Bluetooth(const std::string& addr)
:	mImpl(new Impl), mAddr(addr)
{
}

Bluetooth::~Bluetooth(){
	close();
	//delete mImpl; // handled by auto_ptr as long as this dtor is defined
}

/*bool Bluetooth::find(Bluetooth& bt, const std::string& address){
	
}*/

bool Bluetooth::opened() const {
	return mImpl->opened();
}

bool Bluetooth::open(const std::string& addr){
	mImpl->setDevice(addr);
	return mImpl->open();
}

bool Bluetooth::openRFCOMM(const std::string& addr, int chan){
	mImpl->setDevice(addr);
	chan = mImpl->openRFCOMM(chan);
	if(chan != -1){
		mAddr = addr;
		mChannel = chan;
		return true;
	}
	else{
		return false;
	}
}

bool Bluetooth::close(){
	return mImpl->close();
}

bool Bluetooth::send(const std::vector<unsigned char>& buf){
	return send(&buf[0], buf.size());
}

bool Bluetooth::send(const unsigned char * buf, unsigned len){
	return mImpl->send(buf, len);
}

bool Bluetooth::recv(std::vector<unsigned char>& buf){
	return mImpl->recv(buf);
}

void Bluetooth::print(FILE * fp){
	fprintf(fp,"Name:    %s\n", mName.c_str());
	fprintf(fp,"Address: %s\n", mAddr.c_str());
	fprintf(fp,"Class:   %#6x\n", mClass);
	//fprintf(fp,"Service: %#x\n", mCoD & SERVICE_MAJOR_MASK);
	//fprintf(fp,"Device:  %#x, %#x\n", mCoD & DEVICE_MAJOR_MASK, mCoD & DEVICE_MINOR_MASK);
	if(mChannel != -1) fprintf(fp,"Channel: %d\n", mChannel);
	fprintf(fp,"\n");
}


bool Bluetooth::available(){
	return Impl::available();
}

void Bluetooth::printDevices(){
	printf("Searching for Bluetooth devices...\n");
	Impl::printDevices();
}

} // al::

#endif
