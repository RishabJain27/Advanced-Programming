#include "commands.h"
#include "debug.h"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"#"     , fn_comment},
};

command_fn find_command_fn (const string& cmd) {
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_comment (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr tcwd = state.get_cwd();
   for (unsigned int num = 1; num < words.size(); num++){
      tcwd->get_contents()->readfile(words[num]);
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ((words.size() == 1) || (words.at(1) == "/")){
      state.set_cwd(state.get_root());
      state.get_pwd().clear();
      return;
   }

   if (words.at(1) == ".") return;
   state.set_cwd(state.get_cwd()->get_contents()->
   find_node(words.at(1)));
   if (words.at(1) == ".." && !(state.get_pwd().empty())) 
      state.get_pwd().pop_back();
   else{ 
      state.get_pwd().push_back(words.at(1));
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   exit_status exitStatus;
   int exitVal = atoi(words[1].c_str());

   if(words[1] == "0"){
      exitStatus.set(0);
   }
   else if (words.size()==1){
      exitStatus.set(0);
   }
   else if (exitVal != 0){
      exitStatus.set(exitVal);
   }
   else{
      exitStatus.set(127);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string pwdstring = "";
   wordvec cd_to_parent;
   cd_to_parent.push_back("cd");
   cd_to_parent.push_back("..");
   inode_ptr tcwd = state.get_cwd();

   if ((words.size() == 1) || (words.at(1) == "/")){
      pwdstring += "/:";
      cout << pwdstring << endl;
      state.get_cwd()->get_contents()->print();
   }
   else if(words[1] == ".."){
      fn_cd(state, cd_to_parent);
      pwdstring = "..:";
      cout << pwdstring << endl;
      state.get_cwd()->get_contents()->print();
      state.set_cwd(tcwd); 
   }
   else if(words[1] == "."){
      pwdstring = ".:";
      cout << pwdstring << endl;
      state.get_cwd()->get_contents()->print();
   }
   else{
      cout << "No files " << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec currState;
   currState.push_back("cd");
   fn_cd(state,currState);
   if ((words.size() == 1) || (words.at(1) == "/")){
      cout << "/:" << endl;
      state.get_cwd()->get_contents()->print2();
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() < 2) return;
   wordvec data = wordvec(words.begin()+1, words.end());
   state.get_cwd()->get_contents()->mkfile(words.at(1))->
   get_contents()->writefile(data);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   vector<string> temp;
   if (words.size() < 2) return;
   state.get_cwd()->get_contents()->mkdir(words.at(1));

   string pwdstring = "";
   if (state.get_pwd().empty()) pwdstring = "/";
   for(unsigned int i = 0; i < state.get_pwd().size(); ++i) 
      pwdstring += "/" + state.get_pwd().at(i);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string temp = "";
   int size = words.size();
   for(int i=1; i< size; i++){
      temp += words[i];
      temp += " ";
   }
   state.prompt_ = temp;
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string pwdstring = "";
   if (state.get_pwd().empty()) pwdstring = "/";
   for(unsigned int i = 0; i < state.get_pwd().size(); ++i) 
      pwdstring += "/" + state.get_pwd().at(i);
   cout << pwdstring << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for (unsigned int i = 1; i < words.size(); ++i)
      state.get_cwd()->get_contents()->remove(words.at(i));
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() != 1){
      if(state.get_cwd()->get_contents()->getType() == "directory")
         for (unsigned int i = 1; i < words.size(); ++i)
            state.get_cwd()->get_contents()->remove(words.at(i));
   }
}
