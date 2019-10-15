#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include <fstream>
#include<vector>
#include<map>
#include <mutex>
#include <iostream> 
#include <functional>
#include <semaphore.h> 
#include <unistd.h> 


using namespace std;

sem_t mutex1,mutex2;
void *server_service(void *);
void *servingeachclient(void *);
void *getclient(void *);
long int chunksize = 524288;
mutex tLock,ticketsLock,server_Lock; 
map<int,char *>allchunksoffile;

struct arg_struct            
{
    char *mtorrent, *destination, *fname, *ip, *port;
    int sock,part;
    long int size;
};

struct file_transfer{
    char filename[100];
    int chunkpart;
    long int size;
};

struct file_transfer_fellow_tracker{
    char filename[100];
};


struct struct_of_clientip_and_clientport{
   char *ip, *port;
   int part;
};

struct number_of_chunk_and_filediscriptor{
    int fd, part;
};

vector<string>vec;
vector<string>vec1;

string client_ip,server_ip;
vector<string>after_cut;
int chksz=524288;
std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

vector<string> stringsplit(string command, char delimeter)
{
   vector<string> strcollection;
    int i = 0;
    string s = "";
    while (command[i] != '\0')
    {if (command[i] == delimeter)
        {
            strcollection.push_back(s);
            s = "";
        }

        else
        {
            s = s + command[i];
        }
        i++;
    }
    strcollection.push_back(s);
    return strcollection;
}

vector<string> removeDupWord(string str) 
{ 
    vector<string>temp;
    istringstream ss(str); 
  
    do { 
        string word; 
        ss >> word; 
  
        temp.push_back(word); 
  
    } while (ss); 
} 



int main(int argc,char **argv){
    string str1="127.0.0.5";
    string str2="127.0.0.6";
    vec.push_back(str1);
    vec.push_back(str2);
    string c_port,c_ip,pt_port,pt_ip;
    int i=0;
    if(argc < 2)
    {
        printf("insufficent arguments");
    }
    
       c_ip=string(argv[1]);
       c_port=string(argv[2]);
       
   sem_init(&mutex1, 0, 1);
   sem_init(&mutex2, 0, 1);
   struct struct_of_clientip_and_clientport args;
    args.ip = new char[c_ip.length() + 1];
    strcpy(args.ip, c_ip.c_str());
    args.port = new char[c_port.length() + 1];
    strcpy(args.port, c_port.c_str());
    pthread_t thread_server;
    pthread_create(&thread_server,0,server_service,(void *)&args);

    



    
    //increase threads to do chunks from different files use while loop send part number as one of the args
    while(1){

    string string1;
    //cout<<"enter command";
    cin>>string1;
    vector<string>command=stringsplit(string1,'|');
    
    int sock=0;
    char *sip_1;
    struct sockaddr_in server_address;
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        printf("error in socket creation");
    }

    memset(&server_address,'0',sizeof(server_address));
    server_address.sin_family=AF_INET;
    string pt_port=string(argv[4]);
    server_address.sin_port=htons(stoi(pt_port));
    string ip1=string(argv[3]);
    char *ip = new char[ip1.length() + 1];
    strcpy(ip, ip1.c_str());
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    connect(sock,(struct sockaddr *)&server_address,sizeof(server_address));

    char buffer[1024];
    
    if(command[0]=="sendrequest"){
    
    while(1){
        bzero(buffer,1024);
        fgets(buffer,1024,stdin);
        long int p;
        p=write(sock,buffer,strlen(buffer));
        bzero(buffer,1024);
        p=read(sock,buffer,1024);
        cout<<buffer<<endl;
    }

    close(sock);
   }

   if(command[0]=="create_user"){
        string str1=command[0]+"|"+command[1]+"|"+command[2];
        char buffer1[1024];
        strcpy(buffer1,str1.c_str());
        //fgets(buffer,255,stdin);
        long int p;
        p=write(sock,buffer1,strlen(buffer1));
        bzero(buffer1,1024);
   }

   if(command[0]=="list_files"){
        string str1=command[0]+"|"+command[1];
        char buffer1[1024];
        strcpy(buffer1,str1.c_str());
        //fgets(buffer,255,stdin);
        long int p;
        p=write(sock,buffer1,strlen(buffer1));
        bzero(buffer1,1024);
        p=read(sock,buffer1,1024);
        cout<<buffer1;


   }
   if(command[0]=="create_group"){
        string str1=command[0]+"|"+command[1];
        char buffer1[1024];
        strcpy(buffer1,str1.c_str());
        //fgets(buffer,255,stdin);
        long int p;
        p=write(sock,buffer1,strlen(buffer1));
        bzero(buffer1,1024);

   }

    if(command[0]=="list_pending_requests"){
        string str1=command[0]+"|"+command[1];
        char buffer1[1024];
        strcpy(buffer1,str1.c_str());
        //fgets(buffer,255,stdin);
        long int p;
        p=write(sock,buffer1,strlen(buffer1));
        bzero(buffer1,1024);

    }

    long int PART=0;

    if(command[0]=="Download"){
        cout<<command[1];
        string str1=command[0]+"|"+command[1]+"|"+command[2];
        //newlines here
        char buffer1[1024];
        strcpy(buffer1,str1.c_str());
        //fgets(buffer,255,stdin);
        long int p;
        p=write(sock,buffer1,strlen(buffer1));
        bzero(buffer1,1024);
        p=read(sock,buffer1,1024);

        string listofipandports=string(buffer1);
        vector<string>ipandport=stringsplit(listofipandports,'|');
        // cout<<"ip address and port of one"<<ipandport[0]<<endl;
        // cout<<"ip adress of port 2"<<ipandport[1]<<endl;
        string s1="9897";
        string s2="9898";
        vec1.push_back(s1);
        vec1.push_back(s2);



    string fname=command[2];

    ifstream readInputFile(fname, ifstream::binary);
    long int fsize;
    const char* filename=fname.c_str();
    fsize=filesize(filename);
    chksz=chunksize;

    long int number_of_chunks=fsize/chksz;
    cout<<"number of chunks"<<endl;
    long int dup_number_of_chunks=fsize/chksz;
    dup_number_of_chunks=dup_number_of_chunks+2; 
    int i=0;
    while (1)
    {
        pthread_t threadId;

        if(dup_number_of_chunks!=0 && i==vec.size()){
        i=0;
        }
        if(dup_number_of_chunks==0){
            cout<<"breaking out of the loop"<<endl;
            cout<<"fired all threads"<<endl;
            pthread_join(threadId, NULL);
            break;
        }

        string ip1=vec[i];
        string port1=vec1[i];
        i++;
        string string1="";
       
        //connection part
    int sock=0;
    char *sip_1;
    struct sockaddr_in server_address;
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        printf("error in socket creation");
    }

    memset(&server_address,'0',sizeof(server_address));
    server_address.sin_family=AF_INET;
    string pt_port=port1;//trackerport
    server_address.sin_port=htons(stoi(pt_port));

    char *ip = new char[ip1.length() + 1];
    strcpy(ip, ip1.c_str());
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    connect(sock,(struct sockaddr *)&server_address,sizeof(server_address)); 

    struct arg_struct args;
            //fname
    string filenamenew=command[3];
    args.fname = new char[filenamenew.length() + 1];
    strcpy(args.fname, filenamenew.c_str());
            //portnumber
    //string port1=;//ithportnumber
    args.port = new char[port1.length() + 1];
    strcpy(args.port, port1.c_str());
            //ipadress
    args.ip = new char[ip1.length() + 1];
    strcpy(args.ip, ip1.c_str());
    
    // cout<<PART<<endl;
    args.part=++PART;
    if((PART-1)==number_of_chunks){
    args.size=fsize%chksz;
    }
    else{
        args.size=chunksize;
    }

    args.sock = sock;
    pthread_create(&threadId, 0, getclient, (void *)&args);
    pthread_detach(threadId);
    sleep(1); 
    dup_number_of_chunks--;
            

    }

   }

}

    return 0;


} 

void *getclient(void *arg)
{
    //connection to server
    struct arg_struct args = *((struct arg_struct *)arg);
    int sock1 = 0;
    struct sockaddr_in serv_addr1;
    if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return arg;
    }
    memset(&serv_addr1, '0', sizeof(serv_addr1));

    serv_addr1.sin_family = AF_INET;
    serv_addr1.sin_port = htons(stoi(string(args.port)));
    inet_pton(AF_INET, args.ip, &serv_addr1.sin_addr);

    connect(sock1, (struct sockaddr *)&serv_addr1, sizeof(serv_addr1));
    long int n;

   //newly added
    char *buffer2;
    buffer2 = new char[chunksize];
    int part=args.part;
    string part_in_string=to_string(part);
    char*part_in_char=new char[part_in_string.length()+1];
    strcpy(part_in_char,part_in_string.c_str());
     cout<<"******"<<endl;
     cout<<part_in_string;
     cout<<"******"<<endl;


     string f_name=string(args.fname);
     f_name=f_name+"|"+part_in_string;
     char *f_name_char=new char[f_name.length()+1];
     strcpy(f_name_char,f_name.c_str());


    struct file_transfer p;
    char f12[100]="kabirsingh.mp4";
    strcpy(p.filename,f12);
    p.chunkpart=part;
    p.size=args.size;
    send(sock1,&p,sizeof(struct file_transfer),0);


    
    long int size=args.size;

    
    char *buffer1=new char[size];
    n = read(sock1, buffer1,size);
  
    sem_wait(&mutex2);
    std::ofstream outfile ("kabirsinghcheck.mp4",std::ofstream::binary|std::ofstream::in|std::ofstream::out|std::ofstream::app);
    long int data=(part-1)*524288;
    outfile.seekp(data);
    outfile.write(buffer1,size);
    cout<<"the part is"<<part<<endl;
    delete[] buffer1;
    sem_post(&mutex2);

    


    return arg;
}

void *server_service(void *c_ipandport){
     cout<<"in server_service"<<endl;
    struct struct_of_clientip_and_clientport args = *((struct struct_of_clientip_and_clientport *)c_ipandport);
    int server_fd;
    struct sockaddr_in address;
    int socket_accept;
    int address_length=sizeof(address);
    char *sip;

    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==0)
        {
         perror("socket failed");
         exit(EXIT_FAILURE);
         }
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family=AF_INET;
    address.sin_port=htons(stoi(args.port));
    int valid=inet_pton(AF_INET,args.ip,&address.sin_addr);
    if(valid<=0){
        printf("address not supported");
        return args.ip;
    }
    bind(server_fd,(struct sockaddr *)&address,sizeof(address));
    listen(server_fd,10);
     pthread_t client_thread;
    

    
     while(1){
        sem_wait(&mutex1);
        //newly added 2 blocks
        struct number_of_chunk_and_filediscriptor arg;
        socket_accept=accept(server_fd,(struct sockaddr *)&address,(socklen_t *)&address_length);
        arg.fd=socket_accept;
        pthread_create(&client_thread,0,servingeachclient,(void*)&arg);
        pthread_detach(client_thread);
        sleep(1);
        sem_post(&mutex1); 
     }
    return args.ip;
}

void *servingeachclient(void *clientName){
    struct number_of_chunk_and_filediscriptor args = *((struct number_of_chunk_and_filediscriptor *)clientName); 
    int new_socket=args.fd;
    int part;
    struct file_transfer p;

    char buffer[1024]={0};
    read(new_socket,&p,sizeof(struct file_transfer));

   
    string fname=string(p.filename);
    part=p.chunkpart;

    cout<<"filename :::::::::::::"<<fname<<":::::::::"<<endl;
    



    cout<<"IN servingeachclient"<<"filename"<<part<<"&&&&&&"<<endl;
    ifstream readInputFile(fname, ifstream::binary);
    long int fsize,flag=0,chksz;
    //To just find file size
    const char* filename=fname.c_str();
    fsize=filesize(filename);
    chksz=p.size;

    
    part--;
    long int position=(part)*524288;
    char *Buffer;
    
    Buffer=new char[chksz];
  
  std::ifstream fin(filename, std::ios::binary | std::ios::in);
   fin.seekg(position,fin.beg);
   fin.read(Buffer,chksz);
    send(new_socket,Buffer,chksz,0);
    delete[] Buffer;
    close(new_socket);

    return clientName;

}