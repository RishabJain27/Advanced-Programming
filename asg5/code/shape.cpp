//Rhea Lingaiah rlingaia
//Rishab Jain rjain9

#include <typeinfo>
#include <unordered_map>
using namespace std;

#include "shape.h"
#include "util.h"
#include <math.h>


static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};
const double PI = 3.141592653589793;


ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height): polygon(
   {
    {-width/2, height/2},
    { width/2, height/2},
    { width/2,-height/2},
    {-width/2,-height/2}
   }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

equilateral::equilateral (GLfloat width): triangle(
   {
    {-width/2, -width/2},   
    { 0.0, width/2},   
    { width/2,-width/2},  
   }) {
   DEBUGF ('c', this << "(" << width << "," << ")");
}

diamond::diamond (GLfloat width, GLfloat height): polygon(
   {
    {0.0, height/2},
    { -width/2, 0.0}, 
    { 0.0,-height/2},
    {width/2,0.0}
   }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

triangle::triangle(const vertex_list& vertices_):
    polygon(vertices_) {
    }

void text::draw (const vertex& center, const rgbcolor& color, 
                const bool) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    auto font = glut_bitmap_font;
    glColor3ubv(color.ubvec);

    GLfloat xpos = center.xpos;
    GLfloat ypos = center.ypos;

    glRasterPos2f(xpos,ypos);

    for (auto chr : textdata) glutBitmapCharacter(font, chr);
}

void ellipse::draw (const vertex& center, const rgbcolor& color, 
                const bool selected) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   if(selected){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }else{
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }
   
   glBegin (GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3d(color.ubvec[0], color.ubvec[1], color.ubvec[2]);
   const float delta = 2 * PI / 32;
   for (float theta = 0; theta < 2 * PI; theta += delta) {
      float xdraw = (dimension.xpos * cos (theta))/2 + center.xpos;
      float ydraw = (dimension.ypos * sin (theta))/2 + center.ypos;
      glVertex2f (xdraw, ydraw);
   }
   glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color, 
                const bool selected) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   vertex_list adjusted_verticies;
   int vval = 0;
   int xval = 0;
   int yval = 0;
   for (auto iter = vertices.cbegin(); 
      iter != vertices.cend(); ++iter){
      vval++;
      xval += iter->xpos;
      yval += iter->ypos;
   }
   int cen1 = xval/vval;
   int cen2 = yval/vval;

    for (auto iter = vertices.cbegin();
      iter != vertices.cend(); ++iter){
      vertex v;
      v.xpos = iter->xpos - cen1;
      v.ypos = iter->ypos - cen2;
      adjusted_verticies.push_back(v);
    }

    if(selected){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else{
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

   glBegin(GL_POLYGON);

   for(size_t i = 0; i < adjusted_verticies.size(); ++i) {
      glColor3d(color.ubvec[0], color.ubvec[1], color.ubvec[2]);
      glVertex2f(adjusted_verticies[i].xpos+center.xpos,
               adjusted_verticies[i].ypos+center.ypos);
   }
   glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}
