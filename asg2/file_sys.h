#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

// inode_t -
//    An inode is either a directory or a plain file.

enum class file_type {PLAIN_TYPE, DIRECTORY_TYPE};
class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
ostream& operator<< (ostream&, file_type);


// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.

class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_state (const inode_state&) = delete; // copy ctor
      inode_state& operator= (const inode_state&) = delete; // op=
      inode_ptr root {nullptr};
      inode_ptr cwd {nullptr};
      inode_ptr parent {nullptr};
      vector<string> pwd {};

   public:
      inode_state();
      const string& prompt();
      string prompt_ {"% "};

      inode_ptr& get_cwd(){return cwd;}
      inode_ptr get_root() const {return root;}

      //const inode_ptr& get_root();
      inode_ptr get_parent() const {return parent;}
      vector<string>& get_pwd(){return pwd;}
      void set_cwd(inode_ptr dir);
};

// class inode -
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//    

class inode {
   friend class inode_state;
   private:
      static int next_inode_nr;
      int inode_nr;
      inode_ptr parent;
      base_file_ptr contents;
      string path;
   public:
      string path2 = "/dir";
      inode (file_type);
      int get_inode_nr(){return inode_nr;}
      const base_file_ptr& get_contents();
      string& get_path() {return path2;}
      void printPath() { cout << path2; }
      void set_parent(inode_ptr p);
};


// class base_file -
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.

class file_error: public runtime_error {
   public:
      explicit file_error (const string& what);
};

class base_file {
   protected:
      base_file() = default;
      base_file (const base_file&) = delete;
      base_file (base_file&&) = delete;
      base_file& operator= (const base_file&) = delete;
      base_file& operator= (base_file&&) = delete;
   public:
      virtual ~base_file() = default;
      virtual size_t size() const = 0;
      virtual void readfile(const string& input) const = 0;
      virtual void writefile (const wordvec& newdata) = 0;
      virtual void remove (const string& filename) = 0;
      virtual inode_ptr mkdir (const string& dirname) = 0;
      virtual inode_ptr mkfile (const string& filename) = 0;
      virtual void init_directory(inode_ptr, inode_ptr) = 0;
      virtual string& getType() = 0;
      virtual void print() = 0;
      virtual void print2() = 0;
      virtual void insert(string name, inode_ptr p) = 0;
      virtual inode_ptr find_node(const string& name) = 0;
      virtual bool is_directory() = 0;
      virtual map<string,inode_ptr> get_dirents() const = 0;
      virtual string get_name() = 0;
      virtual void set_name(string) = 0;
      virtual void decrement_directory_size() = 0;
};


// class plain_file -
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
// writefile -
//    Replaces the contents of a file with new contents.

class plain_file: public base_file {
   private:
      wordvec data;
      string type = "file";
      string dir = "";
   public:
      virtual size_t size() const override;
      virtual void readfile(const string& input) const override;
      virtual void writefile (const wordvec& newdata) override;
      virtual void remove (const string& filename) override;
      virtual inode_ptr mkdir (const string& dirname) override;
      virtual inode_ptr mkfile (const string& filename) override;
      virtual void init_directory(inode_ptr, inode_ptr) override;
      virtual void print();
      virtual void print2();
      virtual string& getType() { return type; }
      virtual void insert(string name, inode_ptr p);
      virtual inode_ptr find_node(const string& name);
      virtual bool is_directory();
      virtual map<string,inode_ptr> get_dirents() const override;
      virtual void set_name(string) {}
      virtual string get_name() {return dir;}
      virtual void decrement_directory_size() {}      
};

// class directory -
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an file_error if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and 
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory: public base_file {
   private:
      // Must be a map, not unordered_map, so printing is lexicographic
      map<string,inode_ptr> dirents;
      string type = "directory";
      string directory_name = "";
      int directory_size = 0;
   public:
      virtual size_t size() const override;
      virtual void readfile(const string& input) const override;
      virtual void writefile (const wordvec& newdata) override;
      virtual void remove (const string& filename) override;
      virtual inode_ptr mkdir (const string& dirname) override;
      virtual inode_ptr mkfile (const string& filename) override;
      virtual void init_directory(inode_ptr, inode_ptr) override;
      virtual string& getType() { return type; }
      virtual void print();
      virtual void print2();
      virtual void insert(string name, inode_ptr p);
      virtual inode_ptr find_node(const string& name);
      virtual bool is_directory();
      virtual map<string,inode_ptr> get_dirents() const override 
      {return dirents;}
      virtual string get_name() { return directory_name; }
      virtual void set_name(string name) { directory_name = name; }
      virtual void decrement_directory_size() { --directory_size; }
};

#endif
