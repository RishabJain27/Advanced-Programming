//Rhea Lingaiah rlingaia
//Rishab Jain rjain9

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
private:
   shared_ptr<shape> pshape;
   vertex center;
   rgbcolor color;

public:
   rgbcolor selected_color;
   bool selected;
   object(const shared_ptr<shape> s, vertex& v, rgbcolor& c);
   void draw() { 
      if (selected){

         pshape->draw(center, color, false);
         pshape->draw (center, selected_color, true);
      }else{
         pshape->draw(center, color, selected);
      }

   }
   void move (GLfloat delta_x, 
     GLfloat delta_y, float w_width, float w_height) {
      if(center.xpos < 0){
         center.xpos = w_width;
      }
      if(center.xpos > w_width){
         center.xpos = 0; 
      }
      if(center.ypos < 0){
         center.ypos = w_height;
      }
      if(center.ypos > w_height){
         center.ypos = 0; 
      }
      center.xpos += delta_x;
      center.ypos += delta_y;
   }
};

class mouse {
   friend class window;
private:
   int xpos {0};
   int ypos {0};
   int entered {GLUT_LEFT};
   int left_state {GLUT_UP};
   int middle_state {GLUT_UP};
   int right_state {GLUT_UP};
private:
   void set (int x, int y) { xpos = x; ypos = y; }
   void state (int button, int state);
   void draw();
};


class window {
   friend class mouse;
private:
   static int width;
   static int height;
   static vector<object> objects;
   static size_t selected_obj;
   static mouse mus;
private:
   static void close();
   static void entry (int mouse_entered);
   static void display();
   static void reshape (int width, int height);
   static void keyboard (GLubyte key, int, int);
   static void special (int key, int, int);
   static void motion (int x, int y);
   static void passivemotion (int x, int y);
   static void mousefn (int button, int state, int x, int y);
   static void select_object (size_t obj);
   static void move_selected_object(GLfloat xpos, GLfloat ypos);

public:
      static void push_back (const object& obj) {
                  objects.push_back (obj); 
               }
   static void setwidth (int width_) { width = width_; }
   static void setheight (int height_) { height = height_; }
   static void main();
   static int moveby;
   static GLfloat border_thickness; 
   static GLfloat selected_border_thickness; 
   static rgbcolor border_color; 
};

#endif
