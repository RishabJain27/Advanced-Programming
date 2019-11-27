//Rhea Lingaiah rlingaia
//Rishab Jain rjain9

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "fstream"
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"rm" ,  cix_command::RM  },
   {"get" , cix_command::GET },
   {"put" , cix_command::PUT },
};
//the help method is provided by mackey
static const string help = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

//Internal to cix,not used in communication.
void cix_help() {
   cout << help;
}

//this will list all the files available 
//Request file (ls)information.
//The payload length and filename are zeroed.
void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   //calls the LSOUT
   //printing the contents in the file
   if (header.command != cix_command::LSOUT) {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }
   else {
      char *buffer= new char[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);

      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

//have to list a file name to remove
//Request to remove a file.The payload length is 0.
void cix_rm(client_socket& server, string filename) {

   cix_header header;
   snprintf(header.filename, filename.length() + 1, filename.c_str());
   header.command = cix_command::RM;
   header.nbytes = 0;

   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);

   //checks to see if the file is deleted correctly
   //or incorrectly here 
   //The payload field is set to the value of errno in the
   //server attempt to preform a task

   if (header.command == cix_command::NAK){
      cerr << "NAK received: failed to delete file: " <<
      filename<<endl;
   }
   else if (header.command == cix_command::ACK){
      cout <<"ACK received: succesfully deleted file: "<<
      filename<<endl;
   }
}

// Request a file from the server.
// Copy  the  file  named file name on  the  remote  server
// and  create  or  overwrite  a file of the same name in the
// current directory.
void cix_get(client_socket& server, string filename) { 

   cix_header header;
   header.command = cix_command::GET;
   //comes when the command is get

   snprintf(header.filename, filename.length() + 1, filename.c_str());
   log << "sending header " << header << endl;

   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   //send the packet to the server

   log << "received header " << header << endl;
   
   if (header.command != cix_command::FILEOUT) {
      log << filename << " is not a file on the server" << endl;
      log << "server returned " << header<<endl;
   }
   else {
      char *buffer= new char[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      std::ofstream os(header.filename, std::ofstream::binary);
      os.write(buffer, header.nbytes); 
   }
}

//The  length  of  the  payload  is  the  number
//  of  bytes  in  the  file.
//Copies  a  local  file  into  the  socket  and  causes  the
//  remote  server  to  create  thatfile in its directory.
void cix_put(client_socket& server, string filename) {

   cix_header header; 
   snprintf(header.filename, filename.length() + 1, filename.c_str());
   //The  contents  ofthe  file  immediately  follow  the  header.
   std::ifstream exist (header.filename, std::ifstream::binary);
   //The  bytes  of  the  payload  are  unstructured and 
   //may contain null bytes
   if (exist) {
      exist.seekg(0, exist.end);
      int length = exist.tellg();
      exist.seekg(0, exist.beg);
      char *buffer= new char[length];

      exist.read(buffer, length); 

      header.command = cix_command::PUT;
      header.nbytes = length;

      send_packet (server, &header, sizeof header);
      send_packet (server, buffer, length);
      recv_packet (server, &header, sizeof header);
   }
   else{
      cerr << "Error: could not find file: " << filename << endl;
   }
   if (header.command == cix_command::NAK){
      cerr << "NAK received: failed to put file on server" << endl;
   }
   else if (header.command == cix_command::ACK){
      cout << "ACK received: put file on the server" << endl;
   }
   exist.close();
}


void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

//main method
int main (int argc, char** argv) {
   vector<string> contents;
   string host;
   in_port_t port;
   log.execname (basename (argv[0]));
   log << "Starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);

   if (args.size() > 2) {
      usage();
   }

   if (args.size() == 1) {
      host = "localhost";
      port = get_cix_server_port (args, 0);
      log << to_string (hostinfo()) << endl;
   }
   else {
      host = get_cix_server_host (args, 0);
      port = get_cix_server_port (args, 1);
      log << to_string (hostinfo()) << endl;
   }
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         std::istringstream ss(line);
         std::string token;

         while(std::getline(ss, token, ' ')) 
            contents.push_back(token);
         const auto& itor = command_map.find (contents[0]);
         cix_command cmd = itor == command_map.end()
         ? cix_command::ERROR : itor->second;
         //check which command is being called
         switch (cmd) {
            //switch case calls command based on command
            //exit similar to control D
            case cix_command::EXIT:
            throw cix_exit();
            break;
            //ls command
            case cix_command::LS:
            cix_ls (server);
            contents.clear();
            break;
            //help command
            case cix_command::HELP:
            cix_help();
            contents.clear();
            break;
            //remove command
            case cix_command::RM: {
               cix_rm (server,contents[1]);
               contents.clear();
               break;
            }
            //get command
            case cix_command::GET: {
               cix_get (server, contents[1]);
               contents.clear();
               break;
            }
            //put command
            case cix_command::PUT: {
               cix_put (server,contents[1]);
               contents.clear();
               break;
            }
            default:
            log << line << ": invalid command" << endl;
            break;
         }
      }
   }
   catch (socket_error& error) {
      log << error.what() << endl;
   }
   catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "Finishing" << endl;
   return 0;
}
