//Rhea Lingaiah rlingaia
//Rishab Jain rjain9

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"
#include "fstream"

logstream log (cout);
struct cix_exit: public exception {};

//Request to remove a file.The payload length is 0.
void reply_rm(accepted_socket& client_sock, cix_header& header) {

   int success = unlink(header.filename);

   if (success == 0)
      header.command = cix_command::ACK;
   else
      header.command = cix_command::NAK;

   send_packet(client_sock, &header, sizeof header);
}

//The  length  of  the  payload  is  the  number
//  of  bytes  in  the  file.
// The  contents  ofthe  file  immediately  follow  the  header.
void reply_put(accepted_socket& client_sock, cix_header& header) {

   char *buffer= new char[header.nbytes + 1];
   recv_packet (client_sock, buffer, header.nbytes);

   buffer[header.nbytes] = '\0';
   std::ofstream os(header.filename, std::ofstream::binary);

   if (os) {
      header.command = cix_command::ACK;
      os.write(buffer, header.nbytes);
   }
   else {
      header.command = cix_command::NAK;
   }

   send_packet(client_sock, &header, sizeof header);

}
//for the get method

void reply_get(accepted_socket& client_sock, cix_header& header) {

   std::ifstream exist (header.filename, std::ifstream::binary);
   //Request a file from the server.
   //The filename is used both remotely and locally.
   if (exist) {
      exist.seekg(0, exist.end);
      int length = exist.tellg();
      exist.seekg(0, exist.beg);
      char *buffer= new char[length];

      exist.read(buffer, length); 

      header.command = cix_command::FILEOUT;
      header.nbytes = length;

      send_packet (client_sock, &header, sizeof header);
      send_packet (client_sock, buffer, length);
   }
   else 
   {
      header.command = cix_command::NAK;
      send_packet (client_sock, &header, sizeof header);
   }


   exist.close();
}
//LS METHOD
//The payload length and filename are zeroed.
//lists the contents
void reply_ls (accepted_socket& client_sock, cix_header& header) {
   const char* ls_cmd = "ls -l 2>&1";
   string outt;
   FILE* ls_pipe = popen (ls_cmd, "r");
   if (ls_pipe == NULL) { 
      log << "ls -l: popen failed: " << strerror (errno) << endl;
      header.command = cix_command::NAK;
      header.nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      outt.append (buffer);
   }

   int status = pclose (ls_pipe);
   if (status < 0) log << ls_cmd << ": " << strerror (errno) << endl;
   else log << ls_cmd << ": exit " << (status >> 8)
      << " signal " << (status & 0x7F)
   << " core " << (status >> 7 & 1) << endl;
   header.command = cix_command::LSOUT;
   header.nbytes = outt.size();
   memset (header.filename, 0, FILENAME_SIZE);
   log << "sending header " << header << endl;

   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, outt.c_str(), outt.size());
   log << "sent " << outt.size() << " bytes" << endl;
}


void run_server (accepted_socket& client_sock) {
   log.execname (log.execname() + "-server");
   log << "connected to " << to_string (client_sock) << endl;
   try {   
      for (;;) {
         cix_header header; 
         recv_packet (client_sock, &header, sizeof header);
         log << "received header " << header << endl;
         switch (header.command) {
            case cix_command::LS: 
            reply_ls (client_sock, header);
            break;
            case cix_command::RM:{
               reply_rm(client_sock, header);
               break;
            }
            case cix_command::PUT:{
               reply_put(client_sock, header);
               break;
            }
            case cix_command::GET:{
               reply_get(client_sock, header);
               break;
            }

            default:
            log << "invalid header from client:" << header << endl;
            break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "Finishing" << endl;
   throw cix_exit();
}

void fork_cixserver (server_socket& server, accepted_socket& accept) {
   pid_t pid = fork();
   if (pid == 0) { // child

      server.close();
      run_server (accept);
      throw cix_exit();
   }else {
      accept.close();
      if (pid < 0) {
         log << "fork failed: " << strerror (errno) << endl;
      }else {
         log << "forked cixserver pid " << pid << endl;
      }
   }
}


void reap_zombies() {
   for (;;) {
      int status;
      pid_t child = waitpid (-1, &status, WNOHANG);
      if (child <= 0) break;

      log << "child " << child
      << " exit " << (status >> 8)
      << " signal " << (status & 0x7F)
      << " core " << (status >> 7 & 1) << endl;
   }
}

void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   reap_zombies();
}

void signal_action (int signal, void (*handler) (int)) {
   struct sigaction action;
   action.sa_handler = handler;

   sigfillset (&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction (signal, &action, nullptr);

   if(rc < 0) 
      log << "sigaction " << strsignal (signal) << " failed: "
   << strerror (errno) << endl;
}


int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "Starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);

   signal_action (SIGCHLD, signal_handler);
   in_port_t port = get_cix_server_port (args, 0);

   try {
      server_socket listener (port);
      for (;;) {
         log << to_string (hostinfo()) << "Accepting port "
         << to_string (port) << endl;
         accepted_socket client_sock;
         for (;;) {

            try {
               listener.accept (client_sock);
               break;
            }catch (socket_sys_error& error) {
               switch (error.sys_errno) {
                  case EINTR:
                  log << "listener.accept caught "
                  << strerror (EINTR) << endl;
                  break;
                  default:
                  throw;
               }
            }
         }
         log << "accepted " << to_string (client_sock) << endl;
         try {
            fork_cixserver (listener, client_sock);
            reap_zombies();
         }catch (socket_error& error) {
            log << error.what() << endl;
         }
      }
   }
   catch (socket_error& error) {
      log << error.what() << endl;
   }
   catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

