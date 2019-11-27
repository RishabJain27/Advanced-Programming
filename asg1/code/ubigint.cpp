// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>
#include <typeinfo>
#include <cmath>
#include <climits>
using namespace std;

#include "ubigint.h"
#include "debug.h"
#include "util.h"


ubigint::ubigint (unsigned long that){
  int modNum = that % 10;
  ubig_value.push_back(modNum);
  that = that / 10;
}

ubigint::ubigint (const string& that){
  for (char digit: that) {
    if (not isdigit (digit)) {
      throw invalid_argument ("ubigint::ubigint(" + that + ")");
    }
    int convDigit = digit - '0';
    ubig_value.insert(ubig_value.begin(),convDigit);
  }
  while (ubig_value.size() > 0 and ubig_value.back() == 0)   
    ubig_value.pop_back();
}

ubigint ubigint::operator+ (const ubigint& that) const {
  ubigint Answer;
  int sizeThis, sizeThat, greatestSize;
  int numThis, numThat,total,totalMod;
  int carry = 0;
  sizeThis = ubig_value.size();
  sizeThat = that.ubig_value.size();

  if(sizeThis > sizeThat){
    greatestSize = sizeThis;
  }
  else{
    greatestSize = sizeThat;
  }
  for(int i = 0; i < greatestSize; i++){
    if(sizeThis > i){
      numThis = ubig_value[i];
    }
    else{
      numThis = 0;
    }
    if(sizeThat > i){
      numThat = that.ubig_value[i];
    }
    else{
      numThat = 0;
    }
    total = numThis + numThat + carry;
    totalMod = total % 10;
    if(total >= 10){
      carry = 1;
      Answer.ubig_value.push_back(totalMod);
    }
    else{
      carry = 0;
      Answer.ubig_value.push_back(totalMod);
    }
  }
  if(carry == 1){
    Answer.ubig_value.push_back(1);
  }
  return Answer;
}

ubigint ubigint::operator- (const ubigint& that) const {
  ubigint Answer;
  int sizeThis, sizeThat;
  int numThat, numThis,num;
  bool carry = false;
  sizeThis = ubig_value.size();
  sizeThat = that.ubig_value.size();

  for(int j = 0; j < sizeThis; j++){
    if(sizeThat > j){
      numThat = that.ubig_value[j];
    }
    else{
      numThat = 0;
    }

    numThis = ubig_value[j];

    if(carry == true){
      numThis = numThis - 1;
    }

    if(numThis >= numThat){
      carry = false;
      num = numThis - numThat;
      Answer.ubig_value.push_back(num);
    }
    else{
      carry = true;
      numThis = numThis + 10;
      num = numThis - numThat;
      Answer.ubig_value.push_back(num);
    }
  }
  while (Answer.ubig_value.size()>0 and Answer.ubig_value.back()==0){
    Answer.ubig_value.pop_back();
  }
  return Answer;
}

ubigint ubigint::operator* (const ubigint& that) const{
  ubigint Answer;
  int sizeThis = ubig_value.size();
  int sizeThat = that.ubig_value.size();
  int totalsize = sizeThat +sizeThis;
  int num = 0;
  int carry = 0;

  for(int i = 0; i < totalsize; i++){
    Answer.ubig_value.push_back(0);
  }
  for(int i = 0; i < sizeThis; i++){
    carry = 0;
    for(int j = 0; j < sizeThat; j++){
      int prod = ubig_value[i] * that.ubig_value[j];
      num = Answer.ubig_value[i+j] + carry + prod;
      carry = num/10;
      Answer.ubig_value[i+j] = num % 10;
      Answer.ubig_value[i+sizeThat] = carry;
    }
  }
  while (Answer.ubig_value.size() > 0 and Answer.ubig_value.back() == 0)
    Answer.ubig_value.pop_back();
  return Answer;
}

void ubigint::multiply_by_2(){
  int size = ubig_value.size();
  int value, mult, answer;
  int carry = 0;

  for(int i = 0; i < size; i++){
    value = ubig_value[i];
    mult = (value * 2) + carry;
    answer = mult % 10;
    ubig_value[i] = answer;

    if(mult >= 10){
      carry = 1;
    }
    else{
      carry = 0;
    }
  }
  if(carry == 1){
    ubig_value.push_back(1);
  }

  while (ubig_value.size() > 0 and ubig_value.back() == 0)
    ubig_value.pop_back();
}


void ubigint::divide_by_2() {
  int size = ubig_value.size();
  int remainder = 0;
  int value, digit;
  for(int i = size-1; i >= 0; i--){
    value = ubig_value[i];
    digit = value/2;
    ubig_value[i] = digit + remainder;

    if(value % 2 == 1){
      remainder = 5;
    }
    else{
      remainder = 0;
    }
  }
  
  while (ubig_value.size() > 0 and ubig_value.back() == 0)
    ubig_value.pop_back();
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
 ubigint zero {0};
 if (divisor == zero) throw domain_error ("udivide by zero");
 ubigint power_of_2 {1};
 ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
    divisor.multiply_by_2();
    power_of_2.multiply_by_2();
  }
  while (power_of_2 > zero) {
    if (divisor <= remainder) {
     remainder = remainder - divisor;
     quotient = quotient + power_of_2;
   }
   divisor.divide_by_2();
   power_of_2.divide_by_2();
 }
 return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
  return udivide (*this, that).quotient;
}


ubigint ubigint::operator% (const ubigint& that) const {
  return udivide (*this, that).remainder;
}


bool ubigint::operator== (const ubigint& that) const {
  int size1 = ubig_value.size();
  int size2 = that.ubig_value.size();
  int val1, val2;
  if (size1 < size2) {
    return false;
  }

  if(size1 > size2){
    return false;
  }

  for(int i = (size1-1) ; i >= 0; i--){
    val1 = ubig_value[i];
    val2 = that.ubig_value[i];

    if(val1 > val2){
      return false;
    }
    if(val2 > val1){
      return false;
    }
  }
  return true;
}

bool ubigint::operator< (const ubigint& that) const {
  int size1 = ubig_value.size();
  int size2 = that.ubig_value.size();
  int val1, val2;
  if (size1 < size2) {
    return true;
  }
  if(size1 > size2){
    return false;
  }

  for(int i = (size1-1) ; i >= 0; i--){
    val1 = ubig_value[i];
    val2 = that.ubig_value[i];
    if(val1 > val2){
      return false;
    }
    if(val1 < val2){
      return true;
    }

    if(val1 == val2 and i == 0){
      return false;
    }
  }

  return true;
}

ostream& operator<< (ostream& out, const ubigint& that) {
  int size = that.ubig_value.size();
  int val;
  int j = 1;
  for (int i = size-1; i >= 0; i--)
  {
    val = that.ubig_value[i];
    out << val;
    if(j % 69 == 0){
      out << "\\" << "\n";
    }
    j++;
  }
  return out;
}
