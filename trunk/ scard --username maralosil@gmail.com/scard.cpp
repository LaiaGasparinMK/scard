/*
 *
 *
 */


#include <string>
#include <iostream>
#include "scard.h"

using namespace std;

SCard::SCard()
	: context(SCARD_SCOPE_USER), card(0), reader(0), protocol(0) {}


SCard::~SCard(){
	
	if(context){
		
		if(reader){
			::SCardFreeMemory(context, reader);
			cout << "SCardFreeMemory" << endl;
		}
		::SCardReleaseContext(context);
		cout << "SCardReleaseContext" << endl;
	}

	cout << "end destructor" << endl;
	
}


void SCard::setContext(unsigned int scope) {
	
	::SCardEstablishContext(scope,NULL,NULL,&context);
	cout << "SCardEstablishContext" << endl;
	cout << "scope = " << scope << " context = "<< context << endl;  
}

/*
void SCard::listReaders(){

	::SCardListReaders(context, NULL,(LPTSTR)&readers, &dwReaders);
}
*/

char* SCard::getReader(){

	return reader;
	
}


void SCard::connect(){

	unsigned long AUTO_ALLOC = SCARD_AUTOALLOCATE;
	
	::SCardListReaders(context, NULL, (LPTSTR)&reader, &AUTO_ALLOC);

	::SCardConnect(context, reader, SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &card, &protocol);

	switch(protocol)
	{
		case SCARD_PROTOCOL_T0:
			pioSendPci = *SCARD_PCI_T0;
			break;

		case SCARD_PROTOCOL_T1:
			pioSendPci = *SCARD_PCI_T1;
			break;
		//default: throw exception!
			
	}

	pioRecvPci.dwProtocol = protocol;
	pioRecvPci.cbPciLength = 255;

	cout << "pioSendPci.dwProtocol = " << pioSendPci.dwProtocol << endl; 
	cout << "pioSendPci.cbPciLength = " << pioSendPci.cbPciLength << endl; 

	cout << "protocol = "<< protocol << endl;
}

void SCard::disconnect(){
	
	::SCardDisconnect(card, SCARD_LEAVE_CARD);
}

void SCard::transmit(const APDU &cmdApdu, APDU &respApdu){

	vector<byte> cmdApduBuffer = cmdApdu.getBuffer();
	vector<byte>::iterator it;

	int cmdSize = cmdApduBuffer.size();
	byte* cmd = new byte[cmdSize];
	
	int index = 0;
	for(it = cmdApduBuffer.begin(); it < cmdApduBuffer.end(); it++){
	
		cmd[index] = *it;
		index++;
	}

	byte resp[255];
	unsigned long respLength = sizeof(resp);

	int rv = ::SCardTransmit(card,&pioSendPci,cmd,cmdSize,&pioRecvPci,resp,&respLength);
	
	cout << "respLength = " <<  respLength << endl;
	cout << "rv = " <<  rv << endl;

	vector<byte> respApduBuffer;

	for(int ct =0; ct < respLength; ct++){
	
		respApduBuffer.push_back(resp[ct]);
	}
		
	respApdu.setBuffer(respApduBuffer);

	delete[] cmd;

}