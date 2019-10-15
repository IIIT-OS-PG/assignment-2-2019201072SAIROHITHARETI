#include<stdio.h>
#include<bits/stdc++.h>
#include<map>
#include<string.h>
#include<vector>
#include <dirent.h>
#include<iostream>
#include<stdlib.h>
#include<string>
#include <unistd.h>
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
#include <mutex>
#include <iostream> 
#include <functional>
#include <semaphore.h> 
#include <unistd.h> 
using namespace std;

void *server_service(void *);
void *servingeachclient(void *);

sem_t mutex1,mutex2;
struct arg_struct            
{
    char *mtorrent, *destination, *fname, *ip, *port;
    int sock;
};

struct file_transfer_fellow_tracker{
    char filename[100];
};


struct struct_of_clientip_and_clientport{
   char *ip, *port;
};
struct number_of_chunk_and_filediscriptor{
    int fd, part;
};


//username and password
map<string,string>user_data;
//map between group_id and 
map<string,int>groupid_owner;
struct peerdata{
	//it is mixture of name and ip and port
	string user_name;
	string directory;
    //vector<string>files;
};
//in file delimiter owner|group_id|user_name|file1|file2|file3..
struct group{
	string owner;
	int group_id;
	vector<struct peerdata>vec;
};

//in file group_id..|username|files..filename is group
struct waitgroup{
	int group_id;
	string directory;
	string user_name;
};

vector<struct group>grp;
vector<struct waitgroup>wait_group;


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

string getallfilesindirectory(string directory){
	struct dirent **namelist;
	string str;
    int n;
    char *f=new char[directory.length()+1];
    strcpy(f,directory.c_str());

   n = scandir(f, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            string s1=string(namelist[n]->d_name);
            string s2=".";
            if(s1.find(s2)!=string::npos)
            {
                str+="|"+s1;
            }
            
            free(namelist[n]);
        }
        free(namelist);
    }

    return str;
}

vector<string> getallfilesindirectory_vector(string directory){
	vector<string>temp;
	struct dirent **namelist;
	string str;
    int n;
    char *f=new char[directory.length()+1];
    strcpy(f,directory.c_str());

   n = scandir(f, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            string s1=string(namelist[n]->d_name);
            string s2=".";
            if(s1.find(s2)!=string::npos)
            {
                //str+="|"+s1;
                temp.push_back(s1);
            }
            
            free(namelist[n]);
        }
        free(namelist);
    }

    return temp;
}

void addtofile(string filename,string data){
	fstream file;
	file.open(filename,ios::out|ios::in|ios::app);
	char *temp=new char[data.length()+1];
	strcpy(temp,data.c_str());
	file<<temp<<endl;
}

//username
void create_user(string user,string password){
	user_data.insert({user,password});
	user_data[user]=password;
	string str=user+"|"+password;
	string filename="user_password.txt";
	addtofile(filename,str);
}

void getuserandpassword(){

	fstream file1;
   file1.open("user_password.txt",ios::out|ios::in);
   string line;
    vector<string>after_split;
	while(getline(file1,line)){
		after_split=stringsplit(line,'|');
		user_data[after_split[0]]=after_split[1];
	}


}

bool check_user(string user){
	if(user_data.find(user)!=user_data.end()){
		return true;
	}
	return false;
}
int i;
//i will send my own arguments
//returns true if new group is going to be formed else retun false
bool create_group(string user,int groupid,string directory){
	for(auto itr=grp.begin();itr!=grp.end();itr++){
	if(groupid==itr->group_id){
		return false;
      }
   }
    struct group p;
	p.owner=user;
    p.group_id=groupid;
    grp.push_back(p);
    struct peerdata t;
    t.user_name=user;
    t.directory=directory;
    p.vec.push_back(t);

    return true;
}

//groupid|owner|username|directory
void insert_user_to_group(string newuser,int groupid,string directory){

	for(auto i=grp.begin();i!=grp.end();i++){
		if(i->group_id==groupid){
			struct peerdata p;
			p.user_name=newuser;
			p.directory=directory;
			i->vec.push_back(p);
			string owner=i->owner;
			groupid_owner[newuser]=groupid;
			string to_enter=to_string(groupid)+"|"+owner+"|"+newuser+"|"+directory;
			string filename="group.txt";
			addtofile(filename,to_enter);
			return;
		}
	}

	struct peerdata p;
	p.user_name=newuser;
	p.directory=directory;
	struct group g;
	g.owner=newuser;
	g.group_id=groupid;
	g.vec.push_back(p);
	grp.push_back(g);
	groupid_owner[newuser]=groupid;
	string to_enter=to_string(groupid)+"|"+newuser+"|"+newuser+"|"+directory;
	string filename="group.txt";
	addtofile(filename,to_enter);
	return;

	
}

//when we reload we need data..
void get_user_to_group(){
	string filename="group.txt";
	fstream file;
	file.open(filename,ios::in|ios::out);
	string line;
	vector<string>after_split;
	while(getline(file,line)){
		after_split=stringsplit(line,'|');
		struct group g;//username
		g.owner=after_split[1];
		g.group_id=stoi(after_split[0]);
		struct peerdata p;
		p.user_name=after_split[2];
		p.directory=after_split[3];
		groupid_owner[after_split[2]]=stoi(after_split[0]);
		g.vec.push_back(p);
	}
}

//get file names when the username is given
string get_files_when_username(string username){
	int groupid=groupid_owner[username];
	for(auto itr=grp.begin();itr!=grp.end();itr++){
		if(itr->group_id==groupid){
			for(auto i=itr->vec.begin();i!=itr->vec.end();i++){
				if(i->user_name==username){
                   return getallfilesindirectory(i->directory);
				}
			}
		}
	}
}

void insert_user_wait_group(string newuser,int groupid,string directory){
	struct waitgroup temp;
	temp.user_name=newuser;
	temp.group_id=groupid;
	temp.directory=directory;
	string str1=to_string(groupid)+"|"+newuser+"|"+directory;
	string filename="waitgroup.txt";
	addtofile(filename,str1);
	wait_group.push_back(temp);
	
}



void get_user_wait_group(){
	string filename="waitgroup.txt";
	fstream file;
	file.open(filename,ios::in|ios::out);
	string line;
	vector<string>after_split;
	while(getline(file,line)){
		after_split=stringsplit(line,'|');
		struct waitgroup temp;
		temp.group_id= stoi (after_split[0]);
		temp.user_name=after_split[1];
		temp.directory=after_split[2];
        wait_group.push_back(temp);
	}

}

void print_group_all(){
	for(auto itr=grp.begin();itr!=grp.end();itr++){
		cout<<itr->group_id;
		cout<<itr->owner;
		for(auto p=itr->vec.begin();p!=itr->vec.end();p++){
			cout<<p->user_name<<" ";
		}
		cout<<endl;
	}
}


//it prints all waited process to be accepted by owner of the group it prints all details of that particular groupid
void print_waitgroup_all(int groupid){
	for(auto itr=wait_group.begin();itr!=wait_group.end();itr++){
       if(itr->group_id==groupid){
       	cout<<"group"<<itr->group_id<<endl;
       	cout<<"username"<<itr->user_name<<endl;
       	cout<<"directory"<<itr->directory<<endl;
       }
	}
}


void add_group_joining_request(int group_id,string user_name){
   for(auto itr=wait_group.begin();itr!=wait_group.end();itr++){
       if(itr->group_id==group_id && itr->user_name==user_name){
       	insert_user_to_group(user_name,group_id,itr->directory);
       	wait_group.erase(itr);
       }
	}

}

void leave_group(int groupid,string user_name){
	for(auto itr=grp.begin();itr!=grp.end();itr++){
		if(itr->group_id==groupid){
			for(auto i=itr->vec.begin();i!=itr->vec.end();i++){
				if(i->user_name==user_name){
					itr->vec.erase(i);
					

				}
			}
		}
	}
}

void readfromfile(string filename){
	fstream file1;
   file1.open(filename,ios::out|ios::in);
   string line;
   while(getline(file1,line)){
   	cout<<line<<endl;
   }
}

void readandaddDS_username_password(){
	fstream file1;
	string filename="username_password.txt";
   file1.open(filename,ios::out|ios::in);
   string line;
   vector<string>temp;
   while(getline(file1,line)){
   	temp=stringsplit(line,'|');
   	user_data[temp[0]]=temp[1];
   }
}

void addtoDs_username_and_password(string data){
	string file="username_password.txt";
	addtofile(file,data);
}



void printusepassword(){
	for(auto itr=user_data.begin();itr!=user_data.end();itr++){
		cout<<itr->first<<endl;
		cout<<itr->second<<endl;
	}
}

//myusername current userename 
bool acceptrequests(int groupid,string username,string myusername){
	//get group_id from map
	int group_id=groupid_owner[myusername];
	
		for(auto itr=wait_group.begin();itr!=wait_group.end();itr++){
			if(itr->user_name==username){
				insert_user_to_group(myusername,groupid,itr->directory);
			}
		}



}

string getipaddressandport(int group_id,string filename){
	string str;
	for(auto itr=grp.begin();itr!=grp.end();itr++){
		if(itr->group_id==group_id){
			for(auto i=itr->vec.begin();i!=itr->vec.end();i++){
             vector<string>temp=getallfilesindirectory_vector(i->directory);
             for(auto j=temp.begin();j!=temp.end();j++){
             	if(*j==filename){
                      str=i->user_name+"|"+str;
             	}
             }
			}
		}
	}

	return str;
}






int main(int argc,char **argv){
	cout<<"hello";
	get_user_wait_group();
	//127.0.0.6,9898
	add_group_joining_request(1,"127.0.0.6,9898");
	add_group_joining_request(1,"127.0.0.5,9897");
	get_user_to_group();
	print_group_all();
	getuserandpassword();
	// string freind=getipaddressandport(1,"kabirsingh.mp4");
	// cout<<freind<<endl;


	// print_waitgroup_all(1);
	// print_waitgroup_all(2);
	// print_waitgroup_all(3);
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    string c_ip=string(argv[1]);
    string c_port=string(argv[2]);
    struct struct_of_clientip_and_clientport args;
	args.ip = new char[c_ip.length() + 1];
    strcpy(args.ip, c_ip.c_str());
    args.port = new char[c_port.length() + 1];
    strcpy(args.port, c_port.c_str());
    pthread_t thread_server;
    //cout<<"hungry";
	pthread_create(&thread_server,0,server_service,(void *)&args);
	pthread_join(thread_server,NULL);


	// cout<<"create_group so enter user_name and groupid";
	// int t;
	// cin>>t;



	//accept_request
	//do map insert after accept_request
	//maintain a map of username and groupid

	//create group


	// while(t--){
	// 	int groupid;
	// 	cin>>groupid;
	// 	string username;
	// 	cin>>username;
	// 	string directory;
	// 	cin>>directory;

	// 	insert_user_wait_group(username,groupid,directory);

	// }



	//get_user_wait_group();


	// while(t--){
	// 	string user;
	// 	cin>>user;
	// 	string password_;
	// 	cin>>password_;
	// 	string str1=user+"|"+password_;
	// 	addtoDs_username_and_password(str1);
	// }
	// readandaddDS_username_password();
	// cout<<"........................"<<endl;
	// printusepassword();
	// string str;
	// cin>>str;
	// string data=getallfilesindirectory(str);
	// cout<<data;
	// int k;
	// cin>>k;
	//creating groups
	// while(t--){
	// string user_name;
	// cin>>user_name;
	// int g_id;
	// cin>>g_id;
	// create_group(user_name,g_id);
 //   }
 //   cout<<"printing all groups"<<endl;
 //    print_group_all();
 //   //insert into wait_groups
 //   insert_user_wait_group("jeep",2);
 //   insert_user_wait_group("bmw",2);
 //   insert_user_wait_group("toyata",2);
 //   insert_user_wait_group("killer",2);
 //   insert_user_wait_group("pointer",2);
 //   print_waitgroup_all(2);
 //   cout<<"waiting for access"<<endl;
 //   cout<<"after gaining add access"<<endl;
 //   add_group_joining_request(2,"killer");
 //   print_waitgroup_all(2);

 //   cout<<"after add request"<<endl;
 //   print_group_all();

	return 0;
}

void *server_service(void *c_ipandport){

	cout<<"hi"<<endl;
     cout<<"in server_service"<<endl;
	struct struct_of_clientip_and_clientport args = *((struct struct_of_clientip_and_clientport *)c_ipandport);
	int server_fd;
	struct sockaddr_in address;
	int socket_accept;
	int address_length=sizeof(address);
	char *sip;

	server_fd=socket(AF_INET,SOCK_STREAM,0);
	int opt=1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family=AF_INET;
    address.sin_port=htons(stoi(args.port));

    int valid=inet_pton(AF_INET,args.ip,&address.sin_addr);
    bind(server_fd,(struct sockaddr *)&address,sizeof(address));
    listen(server_fd,10);
    pthread_t client_thread;
    //socket_accept=accept(server_fd,(struct sockaddr *)&address,(socklen_t *)&address_length);
    //socket_accept=accept(server_fd,(struct sockaddr *)&address,(socklen_t *)&address_length);

    while(1){
    	sem_wait(&mutex1);
    	cout<<"in while loop of server_service"<<endl;
        socket_accept=accept(server_fd,(struct sockaddr *)&address,(socklen_t *)&address_length);
    	struct number_of_chunk_and_filediscriptor arg;
    	arg.fd=socket_accept;
    	pthread_create(&client_thread,0,servingeachclient,(void *)&arg);
    	pthread_detach(client_thread);
    	cout<<"threadcreatedand";
        sleep(1);
        sem_post(&mutex1); 

    }
    return args.ip;
}

void *servingeachclient(void *arg){
	//int new_socket=*((void *)clientName);
	struct number_of_chunk_and_filediscriptor args = *((struct number_of_chunk_and_filediscriptor *)arg);
	int new_socket=args.fd;
	while(1){
	//sem_wait(&mutex2);
	cout<<"in  servingclient";
	char buffer[1024]={0};
	long int n;
	int k;
	
	n=read(new_socket,buffer,1024);

    
	cout<<"the client sent"<<buffer<<"|||||||||||"<<endl;
	// bzero(buffer,1024);
	// fgets(buffer,1024,stdin);
	// n=write(new_socket,buffer,strlen(buffer));

	string str1=string(buffer);
	vector<string>command=stringsplit(str1,'|');
	if(command[0]=="Download"){
    int groupid=stoi(command[1]);
    cout<<groupid;
    string filename=command[2];
    cout<<filename;
    string freind=getipaddressandport(groupid,filename);
    cout<<freind<<endl;

    char buffer2[1024]={0};
    strcpy(buffer2,freind.c_str());

	n=write(new_socket,buffer2,strlen(buffer2));
	cout<<n<<"write data";
	command[0]=" ";



	// struct file_transfer_fellow_tracker p;
    //char f12[100]="kabirsingh.mp4";
    // strcpy(p.filename,buffer2);
    // send(new_socket,&p,sizeof(struct file_transfer_fellow_tracker),0);
	//command[0]="nothing";
	bzero(buffer,1024);
    }
    if(command[0]=="create_user"){
    	string user=command[1];
    	string password=command[2];
    	create_user(user,password);
    }

    if(command[0]=="list_files"){
    	string groupid=command[1];
    	print_group_all();
    }
 

    sleep(2);
	//sem_post(&mutex2); 
   }

}
