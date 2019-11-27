#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

void plain_file::init_directory(inode_ptr, inode_ptr){
   throw file_error ("is a plain file");
}

void directory::init_directory(inode_ptr parent, inode_ptr current){
   dirents.emplace(".",  current);
   dirents.emplace("..", parent);
}


ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   cwd= root;
   parent=root;
   root->contents->init_directory(cwd,parent);
   root->path = "/";
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
      << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
   << ", cwd = " << state.cwd;
   return out;
}

void inode_state::set_cwd(inode_ptr dir){
   cwd = dir; 
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
      contents = make_shared<plain_file>();
      break;
      case file_type::DIRECTORY_TYPE:
      contents = make_shared<directory>();
      break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}


void inode::set_parent(inode_ptr p) {
   parent = p;
}

const base_file_ptr& inode::get_contents(){
   return contents;
} 

   
file_error::file_error (const string& what):
runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   if (data.size() == 1) return size;
   string datastring = "";
   for (unsigned int i = 1; i < data.size(); ++i) 
      datastring += (data.at(i) + " ");
   size = datastring.size()-1;
   DEBUGF ('i', "size = " << size);
   return size;
}

void plain_file::readfile(const string& input) const {

   string contents_;

   for(unsigned int num = 1; num < this->data.size(); num++)
   {
      if (num == this->data.size() - 1)
         contents_ += data[num];
      else
         contents_ += data[num] + " ";
   }

   cout << contents_ << endl;
   cout <<input;
}


void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}


inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::print() {
   for(unsigned int i = 1; i < data.size(); ++i) 
      cout << data.at(i) << " ";
   cout << endl;
}

void plain_file::print2() {
   for(unsigned int i = 1; i < data.size(); ++i) 
   {
      cout << data.at(i) << " ";
   }
   cout << endl;
}

void plain_file::insert(string name, inode_ptr p) {
   DEBUGF ('i', name);
   DEBUGF ('i', p);
   throw file_error ("is a plain file");
}

inode_ptr plain_file::find_node(const string& name) {
   DEBUGF ('i', name);
   throw file_error ("is a plain file");
}

bool plain_file::is_directory(){
   return false;
}
   
size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::readfile(const string& input) const 
{
   bool existence = false;
   inode_ptr node;
   string s;
   try 
   {
      for(auto it = dirents.begin(); it != dirents.end(); ++it)
      {
         if (it->first == input && 
            it->second->get_contents()->getType() == "directory")
         {
            existence = false;
            throw file_error("Error: " + input + " is a directory");
         }
         else if (it->first == input && 
            it->second->get_contents()->getType() != "directory")
         {
            existence = true;
            node = it->second;
            node->get_contents()->readfile("");
         }
      }
      if (existence == false)
      {
      }
   }
   catch(file_error& error)
   {
     complain() << error.what() << endl;
  }
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr newdir = 
   shared_ptr<inode> (new inode(file_type::DIRECTORY_TYPE));
   this->insert(dirname, newdir);
   newdir->get_contents()->insert(".", newdir);
   newdir->get_contents()->insert("..", this->find_node("."));
   newdir->get_contents()->set_name(dirname);
   return newdir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr newfile = 
   shared_ptr<inode> (new inode(file_type::PLAIN_TYPE));
   this->insert(filename, newfile);
   return newfile;
}

void directory::print() {
   std::map<std::string, inode_ptr>::iterator it;
   for(it = dirents.begin(); it != dirents.end(); ++it)
   {
      if (it->second->get_contents()->getType() == "directory" and 
         it->first != "." and it->first!= ".." )
         cout << "   " << it->second->get_inode_nr() << "   " 
      << it->second->get_contents()->size() << "    " 
      << it->first + "/" << endl;
      else
         cout << "   " << it->second->get_inode_nr() << "   " <<   
         it->second->get_contents()->size() << "    " <<
         it->first << endl;
   }
}

void directory::print2() {
   string pwdstring = "";
   string path = "";

   for(auto i = dirents.cbegin(); i != dirents.cend(); ++i) {
      cout << "   " << i->second->get_inode_nr() << "   ";
      cout << i->second->get_contents()->size() << " " << i->first;
      if (i->second->get_contents()->is_directory() && 
         !((i->first == ".")||(i->first == ".."))) 
         cout << "/";
      cout << endl;
   }

   for(auto i = dirents.cbegin(); i != dirents.cend(); ++i) {
      if(i->second->get_contents()->is_directory() && 
         !((i->first == ".")||(i->first == "..")))
      {
         string contents = "/" + i->second->get_contents()->get_name();
         path = i->second->get_path();
         if(contents != path){
            path += "/";
            path += i->second->get_contents()->get_name();
         }
         path += ":";
         cout <<  path << endl;
         i->second->get_contents()->print2();
      }
   }
}

void directory::insert(string name, inode_ptr p) {
   dirents.emplace(name, p);
}

map<string,inode_ptr> plain_file::get_dirents() const {
   throw file_error ("is a plain_file");
}

inode_ptr directory::find_node(const string& name) {
   return dirents.at(name);
}

bool directory::is_directory(){
   return true;
}
