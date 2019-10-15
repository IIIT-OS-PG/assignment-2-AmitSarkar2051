/*
1.file transfer all types
2.treading 
*/

/*
running manual
1. ./cli <ip>:<PORT>
PORT number = any IP = check ifconfig

2. logout
program exit

3. download_file <filename> <file path> <group id> <src ip> <src port>
*/

//header files
#include<iostream>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include<unistd.h>
#include<stdio.h>
#include <arpa/inet.h>
#include<vector>
#include<sstream>
#include<thread>
#include<cmath>
#include <stdio.h>  
#include <sys/types.h> 
#include <fcntl.h> 
#include<fstream>
#include <map>
#include <algorithm>
#include<bits/stdc++.h>
#include <iterator> 

using namespace std;

//Predefined Parameters
#define _PORT 80000
#define _BLOG 5
#define MAX 1024
#define _IP "127.0.0.1"
#define _CHUNK 1024*512

//Global variable
char buf[MAX];
char bchunk[_CHUNK];
string sbuf;
char fname[100];
string GserverIP, GserverPORT;
long long int CHUNK=1024*512;
string tarfname;

map<string,string> map_uid_pwd;
map<string,string> map_uid_ip;


//Common utility functions

vector<string> _fstringsplit(string &str,char delim){
  	
  	vector<string> result;
    stringstream ss (str);
    string token;

    while (getline (ss, token, delim)) {
        result.push_back (token);
    }

    return result;
}

//client functions
void fchunkreq(FILE *fp,string filename, int i,string targetIP, string targetPort){
	/*
	1. send req type
	2. send file name + path size
	3. send file name + path
	4. Receive file size
	5. send i-th chunk req
	6. recv i-th chunk data 
	*/

	//cout<<"CLIENT CHUNK:Connecting to Server:"<<targetIP<<" Port:"<<targetPort<<" Chunk:"<<i<<endl;
	
	int sfd=socket(AF_INET,SOCK_STREAM,0);
       	if(sfd<0){
		cout<<"CLIENT CHUNK::ERROR: Socket Creation Failed!\n";
		exit(1);
	}
	//cout<<"Socket, chunk: :"<<sfd<<" "<<i<<endl;

	struct sockaddr_in ser_addr;
	memset(&ser_addr, 0,sizeof(sockaddr_in));
	ser_addr.sin_family=AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(targetIP.c_str());
	ser_addr.sin_port= atoi(targetPort.c_str());

	int size=sizeof(ser_addr);
	int cfd=connect(sfd,(struct sockaddr *)&ser_addr,size);
    if(cfd<0){
            cout<<"ERROR:Client - Chunk Download Connection failed! id,chunk:"<<cfd<<" "<<i<<endl;
            perror("CONNECT : ");
            exit(1);
    }
	// req type
    int n;
	int reqType=1;//0= file availability 1= chunk request
	n=send(sfd,&reqType,sizeof(reqType),0);//sendfing request time
	
	//file +path size
    size=filename.length();
	send(sfd,&size,sizeof(size),0);//size of the filename+path
	char *fn;
	fn=new char[filename.length() + 1];
	strcpy(fn,filename.c_str());
	//cout<<"file size: "<<size<<endl;

	//file +path
	n=send(sfd,fn,strlen(fn),0);//filename + path
	

	//reccv file size
	long long int filesize;
	n=read(sfd,&filesize,sizeof(filesize));
	//cout<<"File size Receive from server:"<<filesize<<endl;
	
	//request for i-th chunk
	int chunk_num=i;
	n=send(sfd,&chunk_num,sizeof(chunk_num),0);//chunk number
//	cout<<"sending for chunk:"<<chunk_num<<endl;

	

	//recv i-th chunk data
	long long int remsize=(filesize<=CHUNK) ? filesize : filesize-(chunk_num)*CHUNK;
	remsize=(remsize>=CHUNK) ? CHUNK : remsize;

	char rbuf[remsize];
	memset(rbuf,'\0',remsize);
	n= recv(sfd,rbuf,remsize,0); // data for that chunk
	//cout<<"receiving for chunk:"<<rbuf<<endl;

	//string dfname="dwnld2.txt";
	//string dfname="dwnld2.jpg";
	//string dfname="fish2.mp4";

	string dfname=tarfname;

	char *dfn;
	dfn=new char[dfname.length() + 1];
	strcpy(dfn,dfname.c_str());
	fseek ( fp , chunk_num*CHUNK, SEEK_SET);
	int wsize=fwrite(rbuf,sizeof(char),remsize,fp);//n=remsize
	
	

	cout<<"CLIENT CHUNK:Success:"<<chunk_num<<" write size:"<<remsize<<" FP:"<<ftell(fp)<<endl;
	fclose(fp);
}
void _fclient_downloadfile(vector<string> v,int sockid,string targetIP,string targetPort){
	/*
	1. send req type
	2. send file name + path size
	3. send file name + path
	4. Receive file size
	*/
	//1. send the filename with absolute path to respective server 
	string filename=v[1];
	string path=v[2];
	filename=path+"/"+filename;
	int n;
	int reqType=0;//0= file availability 1= chunk request
	n=send(sockid,&reqType,sizeof(reqType),0);//sendfing request type
	int size=filename.length();
	send(sockid,&size,sizeof(size),0);//size of the filename+path
	char *fn;
	fn=new char[filename.length() + 1];
	strcpy(fn,filename.c_str());
	//cout<<size<<endl;
	//strcpy(buf,&filename); send(sockid,buf,sizeof(filename),0);
	//send(sockid,&filename,size,0);//filename+path
	n=send(sockid,fn,strlen(fn),0);
	//cout<<"send:"<<n;
	if(n<0)
		perror("SEND");
	//cout<<filename<<"--"<<endl;
	//cout<<buf<<"--"<<endl;

	int filesize;
	n=read(sockid,&filesize,sizeof(filesize));
	cout<<"CLIENT:File size Receive from server:"<<filesize<<endl;
	int chk=(filesize/CHUNK);
	if(filesize%CHUNK>0)
		chk++;
//	cout<<"CLIENT:Required Chunk:"<<chk<<endl;

	//blank file create
	//string dfname="dwnld2.txt";
	string dfname="dwnld2.jpg";
	//string dfname="fish2.mp4";
	char *dfn;
	dfn=new char[dfname.length() + 1];
	strcpy(dfn,dfname.c_str());

	   FILE *fp = fopen(dfn, "wb");
       fseek(fp, filesize-1 , SEEK_SET);
       fputc('\0', fp);
       fclose(fp);
  //  cout<<"CLIENT:blank file creation created of size:"<<filesize<<endl;
 
    thread vth[chk];

	for(int i=0;i<chk;i++){

	FILE *fp=fopen(dfn,"wb");
	
	if(fp==NULL){
		cout<<"CHUNK Write: error in file Open"<<i<<endl;
		return;
	}
//	cout<<" Open Fp for "<<dfn<<" "<<i<<endl;

	 vth[i] = std::thread(fchunkreq,fp,filename,i,targetIP,targetPort);
		//cout<<"CLIENT CHUNK: thread called for Chunk number:"<<i<<endl;
	}

	for(int i=0;i<chk;i++){
		if(vth[i].joinable())
			vth[i].join();
		else
			cout<<endl<<endl<<"*"<<endl<<endl;
	}	
	return ;
}
void _fclient_download(std::vector<string> v){
/*	cout<<"\n Requesting to download the file :"<<v[1]<<"\nFrom the path :"<<v[2]<<"\nGroup :"<<v[3];
	cout<<"\nFrom machine:"<<v[4]<<" Port:"<<v[5];	
*/
	string targetIP, targetPort;
	targetIP=v[4]; targetPort=v[5];
	tarfname=v[1];
	int sfd=socket(AF_INET,SOCK_STREAM,0);
       	if(sfd<0){
		cout<<"ERROR: Socket Creation Failed!\n";
		exit(1);
	}
	//cout<<"Client - Download:Socket: "<<sfd<<endl;

	struct sockaddr_in ser_addr;
	memset(&ser_addr, 0,sizeof(sockaddr_in));
	ser_addr.sin_family=AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(targetIP.c_str());
	ser_addr.sin_port= atoi(targetPort.c_str());

	int size=sizeof(ser_addr);
	int cfd=connect(sfd,(struct sockaddr *)&ser_addr,size);
        if(cfd<0){
            cout<<"ERROR:Client - Download Connection failed! id:"<<cfd<<endl;
            perror("CONNECT : ");
            exit(1);
        }
    _fclient_downloadfile(v,sfd,targetIP,targetPort);
    return;
}


void _fclient_menu(std::vector<string> v){
	string s=v[0];
	if(s=="logout"){
		cout<<"Proceed to Logout for:"<<GserverIP<<" : "<<GserverPORT<<endl;
		cout<<"Logging Out. Good Bye. . . "<<endl;
		exit(0);
	}
	else if(s=="quit"){
		cout<<"Proceed to Close the terminal for:"<<GserverIP<<" : "<<GserverPORT<<endl;
		cout<<"Logging Out. Good Bye. . . "<<endl;
		exit(0);
	}
	else if(s=="download_file"){
		_fclient_download(v);

	}
	else{
		cout<<"NO COMMAND FOUND"<<endl;
		/*for(int i=0;i<v.size();i++){
		cout<<" | "<<v[i]<<"|";
		}*/
	//cout<<endl;

	//	cout<<"Menu executed. . . ";
	}
}
void _fclientthread(){

	while(1){
		string input;
		cout<<"\ncommand$:";
		getline(cin,input);
		vector<string> vinput= _fstringsplit(input,' ');
		_fclient_menu(vinput);
	}
}

// Server functions

void fchunksend(int sockid,char* filename,int chk,char* local_clientip,int local_port,int size){

	//1.connection create part 
	//2.chunk send part
	char rbuf[size];
	int n;
	FILE *fp=fopen(filename,"rb");
	if(fp==NULL){
		cout<<"\nUnable to Open the FILE. File name:"<<filename<<"..."<<endl;
		//exit(1);
		return;
	}
	//cout<<"SERVER CHUNK:Reading for Chunk:"<<chk<<endl;
	fseek ( fp , chk*size, SEEK_SET);
	memset(rbuf,'\0',size);
    n=fread(rbuf,sizeof(char),size,fp);
    send(sockid,rbuf,size,0);
    cout<<"SERVER CHUNK:Send for chunk:"<<chk<<" of size:"<<size<<endl;
	fclose(fp);
	
}
void _fsLogin(int sockid,char* local_clientip,int local_port){
	int size,n;

	//uid get
	n=read(sockid,&size,sizeof(size));
	char buf[size];
	memset(buf,0,size);
	n= recv(sockid,&buf,sizeof(buf),0);
	buf[size]='\0';
	string key =buf;

	//pwd get
	n=read(sockid,&size,sizeof(size));
	char dbuf[size];
	memset(dbuf,0,size);
	n= recv(sockid,&dbuf,sizeof(dbuf),0);
	dbuf[size]='\0';
	string val=dbuf;
	//cout<<"\nget from peer:"<<key<<" "<<val<<"--"<<map_uid_pwd[key]<<endl;
	int ack=1;//0= failed 1= success
	map<string, string>::iterator itr; 
	itr=map_uid_pwd.find(key);
	if (itr != map_uid_pwd.end()){

		string p=itr->second;
		if(p!=val){
			ack=0;
			cout<<"\nTRACKER:password not matched!\n ";
		}
		else
			ack=1;
	}
	else{
		ack=0;
		cout<<"\nTRACKER:User not exists!\n ";
	}
 
    n=send(sockid,&ack,sizeof(ack),0);
	return;
}
void _fsCreateAc(int sockid,char* local_clientip,int local_port){
	int size,n;
	//cout<<" Start Processing"<<endl;

	//uid get
	n=read(sockid,&size,sizeof(size));
	char buf[size];
	memset(buf,'\0',size);
	n= recv(sockid,&buf,sizeof(buf),0);
	buf[size]='\0';
	string key =buf;
//	cout<<key<<" "<<buf<<endl;

	//pwd get

	n=read(sockid,&size,sizeof(size));
	
	char dbuf[size];
	memset(dbuf,'\0',size);
	n= recv(sockid,&dbuf,sizeof(dbuf),0);
	dbuf[size]='\0';
	string val=dbuf;
//	cout<<dbuf<<endl;
	map_uid_pwd[key]=val;
    map_uid_pwd.insert({key,val});
    cout<<"\nget from peer:"<<key<<" "<<map_uid_pwd[key]<<endl;
    int ack=1;//0= failed 1= success
	n=send(sockid,&ack,sizeof(ack),0);
	return;
}
void _fserver_con_toclient(int sockid,char* local_clientip,int local_port){
	int filename_size,reqType=0,n;
	string filename;
/*	if(reqType==0)
	cout << "SERVER: THREAD to handle connection to client is called. . . ."<< endl;
*/
	n=read(sockid,&reqType,sizeof(reqType));// receive the request type
//	cout<<" Req type : "<<reqType<<endl;
	if(reqType==3)
	{
		cout<<"In create user prog\n";
		_fsCreateAc(sockid,local_clientip,local_port);
		return;
	}
	if(reqType==2)
	{
		cout<<"In  user login\n";
		_fsLogin(sockid,local_clientip,local_port);
		return;
	}
/*	if(reqType==0)
	cout<<"SERVER: Get request of type:"<<reqType<<"|"<<n<<endl;
*/
	memset(buf,'\0',MAX);
	n=read(sockid,&filename_size,sizeof(filename_size));
	//int n=read(sockid,&filename_size,MAX);
	if(reqType==0)
	cout<<"SERVER: Get request of a file  of path size:"<<filename_size<<"|"<<n<<endl;
	//memset(buf,0,MAX);

	char dbuf[filename_size+1];
	memset(dbuf,0,filename_size+1);
	//cout<<"\nWaiting for file name + path"<<endl;
	n= recv(sockid,&dbuf,sizeof(dbuf),0);
//	cout<<n<<endl;
//	cout<<dbuf<<"--"<<endl;

	//cout<<"SERVER:Start Processing . . . .\n";
	FILE *fp=fopen(dbuf,"rb");
	if(fp==NULL){
		cout<<"\nUnable to Open the FILE. File name:"<<dbuf<<"..."<<endl;
		//exit(1);
		return;
	}
	
	fseek ( fp , 0 , SEEK_END);
    int size = ftell ( fp );
    rewind ( fp );
       // n=fread(bchunk,1,chunk,fp);
    if(reqType==0)
    cout<<"File Found!"<<dbuf<<" of size:"<<size<<endl;
	send(sockid,&size,sizeof(size),0); //send file size 
	fclose(fp);

	if(reqType==1)//1=chunk request
	{
		
		int chk;
		recv(sockid,&chk,sizeof(chk),0);
		int filesize=size;
		int remsize=(filesize<=CHUNK) ? filesize : filesize-(chk)*CHUNK;
		remsize=(remsize>=CHUNK) ? CHUNK : remsize;
		//cout<<"Processing Chunk request chk: "<<chk<<" of size:"<<remsize<<endl;
		fchunksend(sockid,dbuf,chk,local_clientip,local_port,remsize);
		
	}

	return;
}
void _fserverthread(){

	//socket creation
	int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd<0){
    	cout<<"ERROR : Socket Creation Failed! id:"<<sfd<<endl;
		perror("SOCKET");	
		exit(1);
	}
	//cout<<"Socket id: "<<sfd<<endl;

	//Server address set
	struct sockaddr_in ser_addr;
	memset(&ser_addr,'\0',sizeof(sockaddr_in));
	ser_addr.sin_family=AF_INET;
	GserverIP="127.0.0.1";
    //ser_addr.sin_addr.s_addr = inet_addr(GserverIP.c_str());
    ser_addr.sin_addr.s_addr =htons(INADDR_ANY);
    ser_addr.sin_port= atoi(GserverPORT.c_str());

	//binding 
	int bnd=bind(sfd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
	if(bnd==-1){
		cout<<"ERROR: Binding Server IP and Socket Failed! id:"<<bnd<<endl;
		perror("BIND");
        exit(1);
	}
//	cout<<"Bind: "<<bnd<<endl;

	int lst=listen(sfd,_BLOG);
	if(lst==-1){
		 perror("Listen");
		cout<<"ERROR: Listening from Socket Failed!\n";
                exit(1);
	}
//	cout<<"Listen: "<<lst<<endl;
	
	//Accepting creation: bind a socket (sfd) to an IP (server IP) .Success= 0, ERROR= -1
	struct sockaddr_in cli_addr;
	memset(&ser_addr, 0,sizeof(sockaddr_in));
	int size=sizeof(cli_addr);

	while(1){
		cout<<"Server:TRACKER is Up for  Listening.. . ."<<endl;
		int cfd=accept(sfd,(struct sockaddr *)&cli_addr,(socklen_t *)&size);
        if(cfd<0){
            cout<<"ERROR: Accept Creation failed!. . .\n";
            perror("Connection to Client");
            exit(1);
        }
       	int local_port=ntohs(cli_addr.sin_port);
		char* local_clientip=inet_ntoa(cli_addr.sin_addr);
		cout << "Server:A new Client is Connected(acceptance socketid,port num, client ip ) : " << cfd <<" " << local_port<<" "<<local_clientip<< endl;
		thread _thread_con_toclient(_fserver_con_toclient,cfd,local_clientip,local_port);
		_thread_con_toclient.detach();
	}
	//close(sfd);

}
void _fsetserverIP(string s){
	GserverIP=s;	
	cout<<"TRACKER IP is set to:"<<GserverIP<<endl;
}
void _fsetserverPORT(string s){
	GserverPORT=s;
	cout<<"TRACKER PORT is set to:"<<GserverPORT<<endl;	
}


int main(int argc, char** argv){

	string input=argv[1];
	vector<string> vinput= _fstringsplit(input,':');

 //server IP PORT initialization from user command line argument
 // ./peer <IP>:<PORT>
	_fsetserverIP(vinput[0]);
	_fsetserverPORT(vinput[1]);

 //Thread to handle Server and clients
 	thread _thread_server(_fserverthread);
	thread _thread_client(_fclientthread);

 //MAIN Process will wait for both the thread
	_thread_server.join();
	_thread_client.join();
}