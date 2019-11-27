#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "libfns.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
  DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
  is_negative = that.size() > 0 and that[0] == '_';
  uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
  return *this;
}

bigint bigint::operator- () const {
  return {uvalue, not is_negative};
}

//BigInt Operator for Add
//needs to check the this/that sign to know the sign of the sum
bigint bigint::operator+ (const bigint& that) const {
  if(is_negative and that.is_negative){
    return {uvalue + that.uvalue, true};
  }
  else if(is_negative and not(that.is_negative)){
    if(uvalue > that.uvalue){
      return {uvalue - that.uvalue, true};
    }
    else{
      return {that.uvalue - uvalue, false};
    }
  }
  else if( not(is_negative) and (that.is_negative)){
    if( uvalue > that.uvalue){
      return {uvalue - that.uvalue, false};
    }
    else if(uvalue == that.uvalue){
      return {that.uvalue - uvalue,false};
    }
    else {
      return {that.uvalue - uvalue, true};
    }
  }
  else {
    return {uvalue + that.uvalue, false};
  }
}

//BigInt operator for Subtract
//need to check the this/that sign to know the sign of the diff
bigint bigint::operator- (const bigint& that) const {
  if(is_negative and that.is_negative){
    return {uvalue + that.uvalue, true};
  }
  else if(is_negative and not(that.is_negative)){
    if(uvalue > that.uvalue){
      return {that.uvalue + uvalue, true};
    }
    else{
      return {uvalue + that.uvalue, true};
    }
  }
  else if( not(is_negative) and (that.is_negative)){
    if( uvalue > that.uvalue){
      return {uvalue + that.uvalue, false};
    }
    else {
      return {that.uvalue + uvalue, false};
    }
  }
  else {
    if( uvalue < that.uvalue){
      return {that.uvalue - uvalue, true};
    }
    else if(uvalue == that.uvalue){
      return {that.uvalue - uvalue, false};
    }
    else {
      return {uvalue - that.uvalue, false};
    }
  }
}

//BigInt Operator for multiply
//4 different possible cases
bigint bigint::operator* (const bigint& that) const {
  if(is_negative and that.is_negative){
    return {uvalue * that.uvalue, false};
  }
  else if(is_negative and not(that.is_negative)){
    return {uvalue * that.uvalue, true};
  }
  else if( not(is_negative) and (that.is_negative)){
    return {uvalue * that.uvalue, true};
  }
  else {
    return {uvalue * that.uvalue, false};
  }
}

bigint bigint::operator/ (const bigint& that) const {
  bigint result = uvalue / that.uvalue;
  return result;
}

bigint bigint::operator% (const bigint& that) const {
  bigint result = uvalue % that.uvalue;
  return result;
}

bool bigint::operator== (const bigint& that) const {
 return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
  if (is_negative != that.is_negative)
    return is_negative;
  return is_negative ? uvalue > that.uvalue
  : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
 return out << (that.is_negative ? "-" : "")
 << that.uvalue;
}
