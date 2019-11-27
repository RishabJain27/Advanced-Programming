// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <cassert>
using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

const string dash_name = "-";

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   ifstream in;

   int statOfFile = 0;
   string progname (basename (argv[0]));
   vector<string> filenames (&argv[1], &argv[argc]);

   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   regex value_regex {R"(^\s*[=]\s*[\S]+\s*$)"};
   regex delete_regex {R"(^\s*[\S]+\s*[=]\s*$)"};
   regex equal_regex {R"(\s*[=]\s*$)"};
   str_str_map map;

   if (filenames.size() == 0) filenames.push_back (dash_name);
   for (const auto& filename: filenames) {
      int lineNumber = 1;
      if (filename == dash_name){
         for(;;) {
            string line;
            getline (cin, line);
            if (cin.eof()) break;
            cout << "-" << ": " << lineNumber << ": " << line << endl;
            ++lineNumber;

            smatch result;
            if (regex_search (line, result, comment_regex)) {
            continue;
            }
            if (regex_search (line, result, value_regex))
            {
               auto beg = map.begin();
               string val = result.str(0);
               bool first_equals = false;
               while (beg != (map.end()))
               {
                  while (val.back() == ' '){
                     val.pop_back();
                  }
                  while (val.front() == ' ' || 
                     val.front() == '=')
                  {
                     if (val.front() == '=' && first_equals == false){
                        val.erase(val.begin());
                        first_equals = true;
                     }
                     else
                        val.erase(val.begin());
                  }
                  if (beg.location()->value.second
                     == val)
                  {
                     cout << beg.location()->value.first <<
                        " = " << beg.location()->value.second << endl;
                  }
                  ++beg;
               }
            }
            else if (regex_search (line, result, delete_regex)){
               auto val = result.str(0);
               bool first_equals = false;
               while (val.front() == ' '){
                  val.erase(val.begin());
               }
               while (val.back() == ' ' ||
                     val.back() == '=')
               {
                  if (val.back() == '=' && first_equals == false){
                     val.erase(val.end() - 1);
                     first_equals = true;
                  }
                  else
                     val.erase(val.end() - 1);
               }
               
               auto beg = map.begin();
               auto en = map.end();
               while (beg != en){
                  if (beg.location()->value.first == val){
                     map.erase(beg);
                     cout << "deleted key" << endl;
                     break;
                  }
                  ++beg;
               }
            }
            else if (regex_search (line, result, trimmed_regex)) {
               if (map.find(result[1]) == map.end()){
                  cout << result[1] << ": " 
                     << "key not found" << endl;
               }
               else {
                  cout << result[1] << " = " 
                     << map.find(result[1])->second << endl;
               }
            }
            else if (regex_search (line, result, equal_regex)){
               map.print_map();
            }
            else if (regex_search (line, result, key_value_regex)) {
               cout << result[1] << " = " << result[2] << endl;
               string key = result[1];
               string value = result[2];
               str_str_pair pair(key, value);
               map.insert(pair);
            }
            else {
               assert (false and "This can not happen.");
            }
         }
      }
      else {
         ifstream infile (filename);
         if (infile.fail()) {
            statOfFile = 1;
            cerr << progname << ": " << filename << ": "
                 << strerror (errno) << endl;
         }else {
             for(;;) {
                string line;
                getline (infile, line);
                if (infile.eof()) break;
                cout << filename << ": " << lineNumber << ": " 
                   << line << endl;
                ++lineNumber;

                smatch result;
                if (regex_search (line, result, comment_regex)) {
                continue;
                }

                if (regex_search (line, result, value_regex)){
                   auto beg = map.begin();
                   string val = result.str(0);
                   bool first_equals = false;
                   while (beg != (map.end())){
                      while (val.back() == ' '){
                         val.pop_back();
                      }
                      while (val.front() == ' ' ||
                         val.front() == '=')
                      {
                         if (val.front() == '=' && 
                            first_equals == false)
                         {
                            val.erase(val.begin());
                            first_equals = true;
                         }
                         else
                            val.erase(val.begin());
                      }
                      if (beg.location()->value.second
                         == val)
                      {
                         cout << beg.location()->value.first <<
                            " = " << beg.location()->
                               value.second << endl;
                      }
                      ++beg;
                   }
                }
                else if (regex_search (line, result, delete_regex))
                {
                   auto val = result.str(0);
                   bool first_equals = false;
                   while (val.front() == ' '){
                      val.erase(val.begin());
                   }
                   while (val.back() == ' ' ||
                      val.back() == '=')
                   {
                      if (val.back() == '=' && first_equals == false){
                         val.erase(val.end() - 1);
                         first_equals = true;
                      }
                      else
                         val.erase(val.end() - 1);
                   }

                   auto beg = map.begin();
                   auto en = map.end();
                   while (beg != en){
                      if (beg.location()->value.first == val){
                         map.erase(beg);
                         cout << "deleted key" << endl;
                         break;
                      }
                      ++beg;
                   }
                }
                else if (regex_search (line, result, equal_regex)){
                   map.print_map();
                }
                else if (regex_search 
                   (line, result, key_value_regex)) {
                   cout << result[1] << " = " << result[2] << endl;
                   string key = result[1];
                   string value = result[2];
                   str_str_pair pair(key, value);
                   map.insert(pair);
                }
                else if (regex_search 
                   (line, result, trimmed_regex)) {
                   if (map.find(result[1]) == map.end()){
                      cout << result[1] << ": "
                         << "key not found" << endl;
                   }
                   else
                   {
                      cout << result[1] << " = "
                         << map.find(result[1])->second << endl;
                   }
                }
                else {
                   assert (false and "This can not happen.");
                }
            }
            infile.close();
         }
      }
   }
   return statOfFile;
   return EXIT_SUCCESS;
}
