#include <iostream>
#include <fstream>
#include <QTextStream>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <string>
#include <stack>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <typeinfo>
using std::ifstream;

using namespace std;
#define PI 3.14159265

#include "client.h"

struct point{
    double x, y, r, g, b, z, zprime, vnx, vny, vnz;
};

struct Matrix{
    double matrix[4][4];
};

struct Vertex{
    double x, y, z, w, r, g, b;
};

struct VertexNorm{
    double nx, ny, nz;
};

struct Face{
    Vertex v1, v2, v3;
    VertexNorm vn1, vn2, vn3;
};

struct Colour{
    double red, green, blue;
};

struct SurfaceSettings{
    double red, green, blue, ks, p;
};

struct Settings{
    double xlow, xhigh, ylow, yhigh, hither, yon;
};

struct Depth{
    double near, far;
    Colour fogcolour;
};

struct lightsource{
    double x, y, z, r, g, b, atta, attb;
};

int lightcount;
int lightmode; //0 means flat, 1 means gouraud, 2 means phong
double zbuffer[750][750];
lightsource lights[512];
Matrix CTM;
Matrix cameramatrix;
SurfaceSettings Surface;
Colour ambientlight;
Settings CamSettings;
Depth DepthSettings;
point cameralocation;

Client::Client(Drawable *drawable, char* argv[])
{
    this->drawable = drawable;
    this->argv = argv;
}


void Client::nextPage() {
    static int pageNumber = 0;
    pageNumber++;
    cout << "PageNumber " << pageNumber << endl;
    string filename = argv[1];
    switch(pageNumber % 6) {
    case 1:{
        Setup();
        SimpReader(filename, 0xffffa71a, 0x00000000, 1);
        drawable->updateScreen();
        break;}
   default:
        draw_rect(0, 0, 750, 750, 0xffffffff);
        draw_rect(400, 400, 700, 700, 0xff00ff40);
        drawable->updateScreen();
    }
}

void Client::draw_rect(int x1, int y1, int x2, int y2, unsigned int color) {
    for(int x = x1; x<x2; x++) {
        for(int y=y1; y<y2; y++) {
            drawable->setPixel(x, y, color);
        }
    }
}

void Client::Setup(){
    draw_rect(0, 0, 750, 750, 0xffffffff);
    draw_rect( 50,  50, 700, 700, 0x00000000);
    ResetZBuffer(200);
    InitializeCTM();
    DepthSettings.near = 2147483647;
    CamSettings = {0, 1000, 0, 1000, 0, 200};
    Surface = {0,0,0, 0.3, 8};
    ambientlight = {0,0,0};
    lightcount = 0;
    lightmode = 2;
}

void Client::ResetZBuffer(double farplane){
    for(int i = 50; i <=700; i++){
        for (int j = 50; j <= 700; j++){
            zbuffer[i][j] = farplane;
        }
    }
}

void Client::InitializeCTM(){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            if(i == j){
                CTM.matrix[i][j] = 1;
                cameramatrix.matrix[i][j] = 1;
            }
            else{
                CTM.matrix[i][j] = 0;
                cameramatrix.matrix[i][j] = 0;
            }
        }
    }

}

void Client::GridOfPoints(int topleftx, int toplefty, int offset, struct point points[], int spacing){
    int offsetx = 0;
    int offsety = 0;
    //cout << "starting grid of triangles" << endl;
    for(int j = 0; j < 10;j = j+1){
        for( int i = 0; i <10; i = i+1){
            //drawable->setPixel(topleftx + ((i+3)*20) + offsetx, toplefty + (j + 3)*20 + offsety, 0xff17affa);
            if (offset == 1){
                offsetx = rand() % 25;
                offsety = rand() % 25;
                offsetx = -12 + offsetx;
                offsety = -12 + offsety;
            }
            points[10*j + i].x = (topleftx + i*spacing) + offsetx;
            points[10*j + i].y = (toplefty + j*spacing) + offsety;
            points[10*j + i].r = rand()%256;
            points[10*j + i].g = rand()%256;
            points[10*j + i].b = rand()%256;
            points[10*j + i].z = 0;
            //cout << topleftx + (i + 3)*20 << " " << topleftx + (i + 3)*20 + offsetx << " " << toplefty + (j + 3)*20 << " " << (toplefty + (j + 3)*20) + offsety << endl;
        }
    }
}

void Client::DrawWireframe(struct point points[]){
    for (int j = 0; j < 9;j++){
        for (int i = 0; i < 9;i++){
             DrawTriangleWireframe(points[10*j + i], points[10*j + i + 1], points[10*(j+1) + i + 1]);
             DrawTriangleWireframe(points[10*j + i], points[10*(j+1) + i], points[10*(j+1) + i + 1]);
        }
    }
}

void Client::DrawTriangleWireframe(point p1, point p2, point p3){
    //First, Find the highest point in the triangle
    //Second, draw a line to the other two points
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int x3 = p3.x;
    int y3 = p3.y;
    int highx = x1;
    int highy = y1;
    int midx = x2;
    int midy = y2;
    int lowx = x3;
    int lowy = y3;
    int tempx, tempy;
    point highp, midp, lowp, tempp;

    if (y1 < y2){
        if (y1 < y3){
             if (y2 < y3){
                 highx = x1;
                 highy = y1;
                 highp = p1;
                 midx = x2;
                 midy = y2;
                 midp = p2;
                 lowx = x3;
                 lowy = y3;
                 lowp = p3;
             }
             else{
                 highx = x1;
                 highy = y1;
                 highp = p1;
                 midx = x3;
                 midy = y3;
                 midp = p3;
                 lowx = x2;
                 lowy = y2;
                 lowp = p2;
             }
        }
        else{
            highx = x3;
            highy = y3;
            highp = p3;
            midx = x1;
            midy = y1;
            midp = p1;
            lowx = x2;
            lowy = y2;
            lowp = p2;

        }
    }
    else{
        if (y1 < y3){
            highx = x2;
            highy = y2;
            highp = p2;
            midx = x1;
            midy = y1;
            midp = p1;
            lowx = x3;
            lowy = y3;
            lowp = p3;
        }
        else{
            if(y2 < y3){
                highx = x2;
                highy = y2;
                highp = p2;
                midx = x3;
                midy = y3;
                midp = p3;
                lowx = x1;
                lowy = y1;
                lowp = p1;
            }
            else{
                highx = x3;
                highy = y3;
                highp = p3;
                midx = x2;
                midy = y2;
                midp = p2;
                lowx = x1;
                lowy = y1;
                lowp = p1;
            }
        }
    }

    if (highy == midy){
        if (highx < midx){
            tempx = midx;
            tempy = midy;
            tempp = midp;
            midx = highx;
            midy = highy;
            midp = highp;
            highx = tempx;
            highy = tempy;
            highp = tempp;

        }
        else{
            tempx = highx;
            tempy = highy;
            tempp = highp;
            highx = midx;
            highy = midy;
            highp = midp;
            midx = tempx;
            midy = tempy;
            midp = tempp;
        }
    }

    CallDDA(highp, midp);
    CallDDA(highp, lowp);
    CallDDA(midp, lowp);

}

void Client::DrawTriangleFilled(point p1, point p2, point p3){
    //First, Find the highest point in the triangle
    //Second, draw a line to the other two points
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int x3 = p3.x;
    int y3 = p3.y;
    int highx = x1;
    int highy = y1;
    int midx = x2;
    int midy = y2;
    int lowx = x3;
    int lowy = y3;
    int tempx, tempy;
    point highp, midp, lowp, tempp, highmidp, highlowp, midlowp;
    if (y1 < y2){
        if (y1 < y3){
             if (y2 < y3){
                 highx = x1;
                 highy = y1;
                 highp = p1;
                 midx = x2;
                 midy = y2;
                 midp = p2;
                 lowx = x3;
                 lowy = y3;
                 lowp = p3;
             }
             else{
                 highx = x1;
                 highy = y1;
                 highp = p1;
                 midx = x3;
                 midy = y3;
                 midp = p3;
                 lowx = x2;
                 lowy = y2;
                 lowp = p2;
             }
        }
        else{
            highx = x3;
            highy = y3;
            highp = p3;
            midx = x1;
            midy = y1;
            midp = p1;
            lowx = x2;
            lowy = y2;
            lowp = p2;

        }
    }
    else{
        if (y1 < y3){
            highx = x2;
            highy = y2;
            highp = p2;
            midx = x1;
            midy = y1;
            midp = p1;
            lowx = x3;
            lowy = y3;
            lowp = p3;
        }
        else{
            if(y2 < y3){
                highx = x2;
                highy = y2;
                highp = p2;
                midx = x3;
                midy = y3;
                midp = p3;
                lowx = x1;
                lowy = y1;
                lowp = p1;
            }
            else{
                highx = x3;
                highy = y3;
                highp = p3;
                midx = x2;
                midy = y2;
                midp = p2;
                lowx = x1;
                lowy = y1;
                lowp = p1;
            }
        }
    }

    if (highy == midy){
        if (highx < midx){
            tempx = midx;
            tempy = midy;
            tempp = midp;
            midx = highx;
            midy = highy;
            midp = highp;
            highx = tempx;
            highy = tempy;
            highp = tempp;

        }
        else{
            tempx = highx;
            tempy = highy;
            tempp = highp;
            highx = midx;
            highy = midy;
            highp = midp;
            midx = tempx;
            midy = tempy;
            midp = tempp;
        }
    }
    highp.zprime = 1/highp.z;
    if (highp.z == 0){
        highp.zprime = 0;
    }
    midp.zprime = 1/midp.z;
    if (midp.z == 0){
        midp.zprime = 0;
    }
    lowp.zprime = 1/lowp.z;
    if (lowp.z == 0){
        lowp.zprime = 0;
    }
    int y = highy;
    float midslope =  float(highx - midx)/(highy - midy);
    float lowslope = float(highx - lowx)/(highy - lowy);
    float edgemidx = highx;
    float edgelowx = highx;
    float highmidred = highp.r;
    float highmidgreen = highp.g;
    float highmidblue = highp.b;
    float highmidz = highp.z;
    double highmidzprime = 1/highmidz;
    if (highmidz == 0){
        highmidzprime = 0;
    }
    double highmidvnx = highp.vnx;
    double highmidvny = highp.vny;
    double highmidvnz = highp.vnz;

    float highlowred = highp.r;
    float highlowgreen = highp.g;
    float highlowblue = highp.b;
    float highlowz = highp.z;
    double highlowzprime = 1/highlowz;
    if (highlowz == 0){
        highlowzprime = 0;
    }
    double highlowvnx = highp.vnx;
    double highlowvny = highp.vny;
    double highlowvnz = highp.vnz;

    float midlowred = midp.r;
    float midlowgreen = midp.g;
    float midlowblue = midp.b;
    float midlowz = midp.z;
    double midlowzprime = 1/midlowz;
    if (midlowz == 0){
        midlowzprime = 0;
    }
    double midlowvnx = midp.vnx;
    double midlowvny = midp.vny;
    double midlowvnz = midp.vnz;

    float highmiddr = float(midp.r - highp.r)/(midy - highy);
    float highmiddg = float(midp.g - highp.g)/(midy - highy);
    float highmiddb = float(midp.b - highp.b)/(midy - highy);
    float highmiddz = float(midp.z - highp.z)/(midy - highy);
    double highmiddzprime = double(midp.zprime - highp.zprime)/(midy - highy);
    double highmiddvnx = double(midp.vnx - highp.vnx)/(midy - highy);
    double highmiddvny = double(midp.vny - highp.vny)/(midy - highy);
    double highmiddvnz = double(midp.vnz - highp.vnz)/(midy - highy);

    float highlowdr = float(lowp.r - highp.r)/(lowy - highy);
    float highlowdg = float(lowp.g - highp.g)/(lowy - highy);
    float highlowdb = float(lowp.b - highp.b)/(lowy - highy);
    float highlowdz = float(lowp.z - highp.z)/(lowy - highy);
    double highlowdzprime = double(lowp.zprime - highp.zprime)/(lowy - highy);
    double highlowdvnx = double(lowp.vnx - highp.vnx)/(lowy - highy);
    double highlowdvny = double(lowp.vny - highp.vny)/(lowy - highy);
    double highlowdvnz = double(lowp.vnz - highp.vnz)/(lowy - highy);

    float midlowdr = float(lowp.r - midp.r)/(lowy - midy);
    float midlowdg = float(lowp.g - midp.g)/(lowy - midy);
    float midlowdb = float(lowp.b - midp.b)/(lowy - midy);
    float midlowdz = float(lowp.z - midp.z)/(lowy - midy);
    double midlowdzprime = double(lowp.zprime - midp.zprime)/(lowy - midy);
    double midlowdvnx = double(lowp.vnx - midp.vnx)/(lowy - midy);
    double midlowdvny = double(lowp.vny - midp.vny)/(lowy - midy);
    double midlowdvnz = double(lowp.vnz - midp.vnz)/(lowy - midy);
    //cout << "TriangleFilled,before starting the upper while loop, highp.r is " << highp.r << ", highp.g is " << highp.g << ", highp.b is " << highp.b << endl;
    if (highy == midy){
        //cout << "find me, TriangleFilled, highp.zprime is " << highp.zprime << ", highp.z is " << highp.z << endl;
        CallDDA(highp, midp);
    }

    while(y < midy){
       //calculate x positions of both lines
       //go from smaller to bigger, filling in the lines
       y = y + 1;
       edgemidx = edgemidx + float(midslope);
       edgelowx = edgelowx + float(lowslope);
       highmidred = highmidred + highmiddr;
       highmidgreen = highmidgreen + highmiddg;
       highmidblue = highmidblue + highmiddb;
       highmidz = highmidz + highmiddz;
       highmidzprime = highmidzprime + highmiddzprime;
       highmidvnx = highmidvnx + highmiddvnx;
       highmidvny = highmidvny + highmiddvny;
       highmidvnz = highmidvnz + highmiddvnz;

       highlowred = highlowred + highlowdr;
       highlowgreen = highlowgreen + highlowdg;
       highlowblue = highlowblue + highlowdb;
       highlowz = highlowz + highlowdz;
       highlowzprime = highlowzprime + highlowdzprime;
       highlowvnx = highlowvnx + highlowdvnx;
       highlowvny = highlowvny + highlowdvny;
       highlowvnz = highlowvnz + highlowdvnz;

       highmidp = {edgemidx, y, highmidred, highmidgreen, highmidblue, round(highmidz), highmidzprime, highmidvnx, highmidvny, highmidvnz};
       highlowp = {edgelowx, y, highlowred, highlowgreen, highlowblue, round(highlowz), highlowzprime, highlowvnx, highlowvny, highlowvnz};
       //cout << "edgemidx is " << edgemidx << " edgelowx is " << edgelowx << endl;
       CallDDA(highmidp, highlowp);

    }

    edgemidx = midx;
    midslope = float(midx - lowx)/(midy - lowy);

    if( lowy == midy){
        CallDDA(midp, lowp);
    }
    //cout << "TriangleFilled, before starting lower while loop, highlowzprime is " << highlowzprime << ", highlowdzprime is " << highlowdzprime << endl;
    while (y < lowy){
        y = y + 1;
        edgemidx = edgemidx + midslope;
        edgelowx = edgelowx + lowslope;

        highlowred = highlowred + highlowdr;
        highlowgreen = highlowgreen + highlowdg;
        highlowblue = highlowblue + highlowdb;
        highlowz = highlowz + highlowdz;
        highlowzprime = highlowzprime + highlowdzprime;
        highlowvnx = highlowvnx + highlowdvnx;
        highlowvny = highlowvny + highlowdvny;
        highlowvnz = highlowvnz + highlowdvnz;

        midlowred = midlowred + midlowdr;
        midlowgreen = midlowgreen + midlowdg;
        midlowblue = midlowblue + midlowdb;
        midlowz = midlowz + midlowdz;
        midlowzprime = midlowzprime + midlowdzprime;
        midlowvnx = midlowvnx + midlowdvnx;
        midlowvny = midlowvny + midlowdvny;
        midlowvnz = midlowvnz + midlowdvnz;

        highlowp = {edgelowx, y, highlowred, highlowgreen, highlowblue, round(highlowz), highlowzprime, highlowvnx, highlowvny, highlowvnz};
        midlowp = {edgemidx, y, midlowred, midlowgreen, midlowblue, round(midlowz), midlowzprime, midlowvnx, midlowvny, midlowvnz};
        //cout << "Triangle Filled, highlowzprime is " << highlowp.zprime << ", midlowzprime is " << midlowzprime << endl;
        CallDDA(highlowp, midlowp);
        //cout << "edgemidx is " << edgemidx << " edgelowx is " << edgelowx << endl;

        //cout << "Highlowp.x is " << highlowp.x << " midlowp.x is " << midlowp.x << endl;
    }
}

void Client::PolygonDepthShading(point p1, point p2, point p3, int nearplanecolor, int farplanecolor, bool filled){
    int nearplanered = UnpackRed(nearplanecolor);
    int nearplanegreen = UnpackGreen(nearplanecolor);
    int nearplaneblue = UnpackBlue(nearplanecolor);

    int farplanered = UnpackRed(farplanecolor);
    int farplanegreen = UnpackGreen(farplanecolor);
    int farplaneblue = UnpackBlue(farplanecolor);

    float p1percentage = float(200 - p1.z)/200;
    float p2percentage = float(200 - p2.z)/200;
    float p3percentage = float(200 - p3.z)/200;

    p1.r = nearplanered*p1percentage;
    p1.g = nearplanegreen*p1percentage;
    p1.b = nearplaneblue*p1percentage;

    p2.r = nearplanered*p2percentage;
    p2.g = nearplanegreen*p2percentage;
    p2.b = nearplaneblue*p2percentage;

    p3.r = nearplanered*p3percentage;
    p3.g = nearplanegreen*p3percentage;
    p3.b = nearplaneblue*p3percentage;

    if(filled){
        DrawTriangleFilled(p1,p2,p3);
    }
    else{
        DrawTriangleWireframe(p1,p2,p3);
    }
}

void Client::LineDepthShading(point p1, point p2, int nearplanecolor){
    int nearplanered = UnpackRed(nearplanecolor);
    int nearplanegreen = UnpackGreen(nearplanecolor);
    int nearplaneblue = UnpackBlue(nearplanecolor);

    float p1percentage = float(200 - p1.z)/200;
    float p2percentage = float(200 - p2.z)/200;

    p1.r = nearplanered*p1percentage;
    p1.g = nearplanegreen*p1percentage;
    p1.b = nearplaneblue*p1percentage;

    p2.r = nearplanered*p2percentage;
    p2.g = nearplanegreen*p2percentage;
    p2.b = nearplaneblue*p2percentage;

    CallDDA(p1, p2);
}

int Client::ObjReader(string filename, bool filled){
    Vertex vertices[255];
    VertexNorm vertexnormals[255];
    Face faces[255];
    vertices[0] = {0,0,0,1,0,0,0};
    vertexnormals[0] = {0,0,0};
    int vertexcount = 1;
    int normalcount = 1;

    fstream file;
    file.open(filename.c_str());
    const char* temp[255][255] = {};
    const char* commands[255][255] = {};
    int commandnum = 0;
    int n = 0;
    if(file.fail()){
        cout << "failed to open file" << endl;
        return 1;
    }
    while(!file.eof()){
        //cout << "starting while loop " << endl;
        char line[255];
        file.getline(line, 255);
        n = 0;
        //cout << line << endl;
        temp[commandnum][n] = strtok(line, " ,.-");

        if (temp[commandnum][n] != 0){
            while(temp[commandnum][n] != 0){
                n++;
                temp[commandnum][n] = strtok(NULL, "( ) \",");
            }
        }

       int i = 0;
        for(i = 0; i < n;i++){
            commands[commandnum][i] = strdup(temp[commandnum][i]);
            //cout << "commands["<<commandnum<<"]["<< i << "] is " << commands[commandnum][i] << endl;
        }
        commands[commandnum][i] = "!";

        if (commands[commandnum][0] != 0){
            //cout << "ObjReader, testing what commands[" << commandnum << "][0] is : " << commands[commandnum][0] << endl;
            if (strcmp(commands[commandnum][0], "#") == 0){
                //Line is a comment, can be ignored
            }
            else if (strcmp(commands[commandnum][0], "v") == 0){
                Vertex v;
                if(strcmp(commands[commandnum][4], "!") == 0){//no W,R,G,B given
                    v.x = atof(commands[commandnum][1]);
                    v.y = atof(commands[commandnum][2]);
                    v.z = atof(commands[commandnum][3]);
                    v.w = 1.0;
                    v.r = Surface.red;
                    v.g = Surface.green;
                    v.b = Surface.blue;
                }
                else if (strcmp(commands[commandnum][5], "!") == 0){
                    v.x = atof(commands[commandnum][1]);
                    v.y = atof(commands[commandnum][2]);
                    v.z = atof(commands[commandnum][3]);
                    v.w = atof(commands[commandnum][4]);
                    v.r = Surface.red;
                    v.g = Surface.green;
                    v.b = Surface.blue;
                }
                else if (strcmp(commands[commandnum][7], "!") == 0){
                    v.x = atof(commands[commandnum][1]);
                    v.y = atof(commands[commandnum][2]);
                    v.z = atof(commands[commandnum][3]);
                    v.w = 1.0;
                    v.r = atof(commands[commandnum][4]);
                    v.g = atof(commands[commandnum][5]);
                    v.b = atof(commands[commandnum][6]);
                }
                else if (strcmp(commands[commandnum][8], "!") == 0){
                    v.x = atof(commands[commandnum][1]);
                    v.y = atof(commands[commandnum][2]);
                    v.z = atof(commands[commandnum][3]);
                    v.w = atof(commands[commandnum][4]);
                    v.r = atof(commands[commandnum][5]);
                    v.g = atof(commands[commandnum][6]);
                    v.b = atof(commands[commandnum][7]);
                }

                vertices[vertexcount] = v;
                vertexcount++;
            }
            else if (strcmp(commands[commandnum][0], "vn") == 0){
                VertexNorm v;
                v.nx = atof(commands[commandnum][1]);
                v.ny = atof(commands[commandnum][2]);
                v.nz = atof(commands[commandnum][3]);
                vertexnormals[normalcount] = v;
                normalcount++;
            }

            else if (strcmp(commands[commandnum][0], "f") == 0){
                string firstvertex = string(commands[commandnum][1]);
                Vertex v1, veven, vodd;
                VertexNorm vn1, vneven, vnodd;
                int length = firstvertex.length();
                int slashpos = 0;
                int facenumber = 0;
                bool normal = 0;
                bool texture = 0;
                while (firstvertex[slashpos] != '/' && slashpos < length){
                    slashpos++;
                }

                int firstslash = slashpos;
                if (slashpos < length){
                    texture = 1;
                }

                slashpos++;
                while(firstvertex[slashpos] != '/' && slashpos < length){
                      slashpos++;
                }

                int secondslash = slashpos;
                if (slashpos < length){
                    normal = 1;
                }

                int vertexindex = stoi(firstvertex.substr(0,firstslash), nullptr, 10);
                int normalindex = 0;
                if(normal ==1 ){
                    normalindex = stoi(firstvertex.substr(secondslash+1, (length-secondslash)), nullptr, 10);
                }
                if (vertexindex < 0){
                    v1 = vertices[vertexcount + vertexindex];
                }
                else{
                    v1 = vertices[vertexindex];
                }
                if (normal == 1){
                    if (normalindex < 0){
                        vn1 = vertexnormals[normalcount + normalindex];
                    }
                    else{
                        vn1 = vertexnormals[normalindex];
                    }
                }

                string evenvertex, oddvertex;

                // ----------------------------------------------- v2 ----------------------------------------------------
                evenvertex = string(commands[commandnum][2]);
                length = evenvertex.length();
                slashpos = 0;
                while (evenvertex[slashpos] != '/' && slashpos < length){
                     slashpos++;
                }

                firstslash = slashpos;

                slashpos++;
                while(evenvertex[slashpos] != '/' && slashpos < length){
                      slashpos++;
                }

                secondslash = slashpos;

                vertexindex = stoi(evenvertex.substr(0,firstslash), nullptr, 10);
                if(normal ==1 ){
                    normalindex = stoi(evenvertex.substr(secondslash+1, (length-secondslash)), nullptr, 10);
                }
                if (vertexindex < 0){
                    veven = vertices[vertexcount + vertexindex];
                }
                else{
                    veven = vertices[vertexindex];
                }

                if (normal == 1){
                   if (normalindex < 0){
                       vneven = vertexnormals[normalcount + normalindex];
                   }
                   else{
                        vneven = vertexnormals[normalindex];
                   }
                }

                //-------------------------------------- v3 --------------------------------------------------------
                oddvertex = string(commands[commandnum][3]);
                length = oddvertex.length();
                slashpos = 0;
                while (oddvertex[slashpos] != '/' && slashpos < length){
                     slashpos++;
                }

                firstslash = slashpos;

                slashpos++;
                while(oddvertex[slashpos] != '/' && slashpos < length){
                      slashpos++;
                }

                secondslash = slashpos;

                vertexindex = stoi(oddvertex.substr(0,firstslash), nullptr, 10);
                if (normal == 1){
                    normalindex = stoi(oddvertex.substr(secondslash+1, (length-secondslash)), nullptr, 10);
                }
                if (vertexindex < 0){
                    vodd = vertices[vertexcount + vertexindex];
                }
                else{
                    vodd = vertices[vertexindex];
                }
                if (normal == 1){
                   if (normalindex < 0){
                       vnodd = vertexnormals[normalcount + normalindex];
                   }
                   else{
                        vnodd = vertexnormals[normalindex];
                   }
                }
                Face newface;
                newface.v1 = v1;
                newface.v2 = veven;
                newface.v3 = vodd;
                newface.vn1 = vn1;
                newface.vn2 = vneven;
                newface.vn3 = vnodd;

                faces[facenumber] = newface;
                facenumber++;
                int i = 4;
                while( strcmp(commands[commandnum][i], "!") != 0){
                    if (i %2 == 0){//even
                        evenvertex = string(commands[commandnum][i]);
                        length = evenvertex.length();
                        slashpos = 0;
                        while (evenvertex[slashpos] != '/' && slashpos < length){
                             slashpos++;
                        }

                        firstslash = slashpos;

                        slashpos++;
                        while(evenvertex[slashpos] != '/' && slashpos < length){
                              slashpos++;
                        }

                        secondslash = slashpos;

                        vertexindex = stoi(evenvertex.substr(0,firstslash), nullptr, 10);
                        if(normal ==1 ){
                            normalindex = stoi(evenvertex.substr(secondslash+1, (length-secondslash)), nullptr, 10);
                        }
                        if (vertexindex < 0){
                            veven = vertices[vertexcount + vertexindex];
                        }
                        else{
                            veven = vertices[vertexindex];
                        }
                        if (normal == 1){
                           if (normalindex < 0){
                               vneven = vertexnormals[normalcount + normalindex];
                           }
                           else{
                                vneven = vertexnormals[normalindex];
                           }
                        }
                        Face newface;
                        newface.v1 = v1;
                        newface.v2 = veven;
                        newface.v3 = vodd;
                        newface.vn1 = vn1;
                        newface.vn2 = vneven;
                        newface.vn3 = vnodd;

                        faces[facenumber] = newface;
                        facenumber++;
                    }
                    else{
                        oddvertex = string(commands[commandnum][i]);
                        length = oddvertex.length();
                        slashpos = 0;
                        while (oddvertex[slashpos] != '/' && slashpos < length){
                             slashpos++;
                        }

                        firstslash = slashpos;

                        slashpos++;
                        while(oddvertex[slashpos] != '/' && slashpos < length){
                              slashpos++;
                        }

                        secondslash = slashpos;

                        vertexindex = stoi(oddvertex.substr(0,firstslash), nullptr, 10);
                        if (normal == 1){
                            normalindex = stoi(oddvertex.substr(secondslash+1, (length-secondslash)), nullptr, 10);
                        }
                        if (vertexindex < 0){
                            veven = vertices[vertexcount + vertexindex];
                        }
                        else{
                            veven = vertices[vertexindex];
                        }
                        if (normal == 1){
                           if (normalindex < 0){
                               vneven = vertexnormals[normalcount + normalindex];
                           }
                           else{
                                vneven = vertexnormals[normalindex];
                           }
                        }
                        Face newface;
                        newface.v1 = v1;
                        newface.v2 = veven;
                        newface.v3 = vodd;
                        newface.vn1 = vn1;
                        newface.vn2 = vneven;
                        newface.vn3 = vnodd;

                        faces[facenumber] = newface;
                        facenumber++;
                    }

                    i++;
                }
                //All edges of the face have been extracted, now we can draw it.
                Matrix Temp;
                    for (int i = 0; i < 4; i++){
                        for (int j = 0; j < 4; j++){
                            if (i == j){
                                Temp.matrix[i][j] = 1;
                            }
                            else{
                                Temp.matrix[i][j] = 0;
                            }
                        }
                    }

                    for (int i = 0; i < 4; i++){
                        for (int j = 0; j < 4; j++){
                            for (int k = 0; k < 4;k++){
                                Temp.matrix[i][j] += cameramatrix.matrix[i][k] * CTM.matrix[k][j];
                            }
                        }
                    }
                    point p1, peven, podd;
                    double vlength, tempx, tempy, tempz, vnx, vny, vnz, vn1x, vn1y, vn1z, vn2x, vn2y, vn2z, vn3x, vn3y, vn3z;

                    int currentface = 0; //This number will iterate throught the faces array to select points
                    if (facenumber > 0){ //If there is at least one face, then the first vertex will be the same for every iteration
                        p1 = {faces[currentface].v1.x, faces[currentface].v1.y, faces[currentface].v1.r, faces[currentface].v1.g, faces[currentface].v1.b, faces[currentface].v1.z};
                        tempx = Temp.matrix[0][0]*p1.x + Temp.matrix[0][1]*p1.y + Temp.matrix[0][2]*p1.z + Temp.matrix[0][3];
                        tempy = Temp.matrix[1][0]*p1.x + Temp.matrix[1][1]*p1.y + Temp.matrix[1][2]*p1.z + Temp.matrix[1][3];
                        tempz = Temp.matrix[2][0]*p1.x + Temp.matrix[2][1]*p1.y + Temp.matrix[2][2]*p1.z + Temp.matrix[2][3];
                        p1.x = tempx;
                        p1.y = tempy;
                        p1.z = tempz;

                        //---------------------------------------Normal Calculations--------------------------------------------------------------
                        if (normal == 1)
                        {
                            vn1x = faces[currentface].vn1.nx;
                            vn1y = faces[currentface].vn1.ny;
                            vn1z = faces[currentface].vn1.nz;
                            //normalize by dividing by the length of the norm
                            vlength = VectorNormal(vn1x, vn1y, vn1z);
                            vn1x = vn1x/vlength;
                            vn1y = vn1y/vlength;
                            vn1z = vn1z/vlength;
                        }

                        //---------------------------------------Lighting Calculations-------------------------------------------------------------
                        p1.r = (p1.r)*ambientlight.red;
                        p1.g = (p1.g)*ambientlight.green;
                        p1.b = (p1.b)*ambientlight.blue;

                    }
                    while (currentface < facenumber){
                        peven = {faces[currentface].v2.x, faces[currentface].v2.y, faces[currentface].v2.r, faces[currentface].v2.g, faces[currentface].v2.b, faces[currentface].v2.z};
                        tempx = Temp.matrix[0][0]*peven.x + Temp.matrix[0][1]*peven.y + Temp.matrix[0][2]*peven.z + Temp.matrix[0][3];
                        tempy = Temp.matrix[1][0]*peven.x + Temp.matrix[1][1]*peven.y + Temp.matrix[1][2]*peven.z + Temp.matrix[1][3];
                        tempz = Temp.matrix[2][0]*peven.x + Temp.matrix[2][1]*peven.y + Temp.matrix[2][2]*peven.z + Temp.matrix[2][3];
                        peven.x = tempx;
                        peven.y = tempy;
                        peven.z = tempz;
                        //---------------------------------------Lighting Calculations-------------------------------------------------------------
                        peven.r = (peven.r)*ambientlight.red;
                        peven.g = (peven.g)*ambientlight.green;
                        peven.b = (peven.b)*ambientlight.blue;

                        podd = {faces[currentface].v3.x, faces[currentface].v3.y, faces[currentface].v3.r, faces[currentface].v3.g, faces[currentface].v3.b, faces[currentface].v3.z};
                        tempx = Temp.matrix[0][0]*podd.x + Temp.matrix[0][1]*podd.y + Temp.matrix[0][2]*podd.z + Temp.matrix[0][3];
                        tempy = Temp.matrix[1][0]*podd.x + Temp.matrix[1][1]*podd.y + Temp.matrix[1][2]*podd.z + Temp.matrix[1][3];
                        tempz = Temp.matrix[2][0]*podd.x + Temp.matrix[2][1]*podd.y + Temp.matrix[2][2]*podd.z + Temp.matrix[2][3];
                        podd.x = tempx;
                        podd.y = tempy;
                        podd.z = tempz;
                        //---------------------------------------Lighting Calculations-------------------------------------------------------------
                        podd.r = (podd.r)*ambientlight.red;
                        podd.g = (podd.g)*ambientlight.green;
                        podd.b = (podd.b)*ambientlight.blue;

                        //cout << p1.x << "," << p1.y << "," << p1.z << ", " << p1.r << "," << p1.g << "," << p1.b << "\t " << peven.x << "," << peven.y << "," << peven.z << "," << peven.r << "," << peven.g << "," << peven.b<< "\t " << podd.x << "," << podd.y << "," << podd.z << "," << podd.r << "," << podd.g << "," << podd.b << endl;
                        cameralocation = {Temp.matrix[0][4], Temp.matrix[1][4],0,0,0,Temp.matrix[2][4]};
                        point camtemp;
                        camtemp.x = cameralocation.x;
                        camtemp.y = cameralocation.y;
                        camtemp.z = cameralocation.z;
                        if (normal == 1)
                        {
                            vn2x = faces[currentface].vn2.nx;
                            vn2y = faces[currentface].vn2.ny;
                            vn2z = faces[currentface].vn2.nz;

                            vlength = VectorNormal(vn2x, vn2y, vn2z);
                            if (vlength == 0)
                            {
                                vlength = 1;
                            }
                            vn2x = vn2x/vlength;
                            vn2y = vn2y/vlength;
                            vn2z = vn2z/vlength;

                            vn3x = faces[currentface].vn3.nx;
                            vn3y = faces[currentface].vn3.ny;
                            vn3z = faces[currentface].vn3.nz;

                            vlength = VectorNormal(vn3x, vn3y, vn3z);
                            if (vlength == 0)
                            {
                                vlength = 1;
                            }
                            vn3x = vn3x/vlength;
                            vn3y = vn3y/vlength;
                            vn3z = vn3z/vlength;

                            vnx = (vn1x + vn2x + vn3x)/3;
                            vny = (vn1y + vn2y + vn3y)/3;
                            vnz = (vn1z + vn2z + vn3z)/3;
                            vlength = VectorNormal(vnx, vny, vnz);
                            if (vlength == 0)
                            {
                                vlength = 1;
                            }
                            vnx = vnx/vlength;
                            vny = vny/vlength;
                            vnz = vnz/vlength;

                            if (lightmode == 0)
                            {
                                point center;
                                center.x = (p1.x + peven.x + podd.x)/3;
                                center.y = (p1.y + peven.y + podd.y)/3;
                                center.z = (p1.z + peven.z + podd.z)/3;
                                center.r = (p1.r + peven.r + podd.r)/3;
                                center.g = (p1.g + peven.g + podd.g)/3;
                                center.b = (p1.b + peven.b + podd.b)/3;
                                center.vnx = (p1.vnx + peven.vnx + podd.vnx)/3;
                                center.vny = (p1.vny + peven.vny + podd.vny)/3;
                                center.vnz = (p1.vnz + peven.vnz + podd.vnz)/3;
                                center = LightingCalculation(center, camtemp, center.vnx, center.vny, center.vnz);
                                p1.r = center.r;
                                p1.g = center.g;
                                p1.b = center.b;

                                peven.r = p1.r;
                                peven.g = p1.g;
                                peven.b = p1.b;
                                podd.r = p1.r;
                                podd.g = p1.g;
                                podd.b = p1.b;
                            }
                            else
                            {
                                p1 = LightingCalculation(p1, camtemp, vn1x, vn1y, vn1z);
                                peven = LightingCalculation(peven, camtemp, vn2x, vn2y, vn2z);
                                podd = LightingCalculation(podd, camtemp, vn3x, vn3y, vn3z);
                            }
                        }
                        else
                        {
                            VertexNorm V, W, FaceNormal;
                            V.nx = faces[currentface].v2.x - faces[currentface].v1.x;
                            V.ny = faces[currentface].v2.y - faces[currentface].v1.y;
                            V.nz = faces[currentface].v2.z - faces[currentface].v1.z;

                            W.nx = faces[currentface].v3.x - faces[currentface].v1.x;
                            W.ny = faces[currentface].v3.y - faces[currentface].v1.y;
                            W.nz = faces[currentface].v3.z - faces[currentface].v1.z;
                            FaceNormal.nx = V.ny*W.nz - V.nz*W.ny;
                            FaceNormal.ny = V.nz*W.nx - V.nx*W.nz;
                            FaceNormal.nz = V.nx*W.ny - V.ny*W.nx;
                            vlength = VectorNormal(FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                            FaceNormal.nx = FaceNormal.nx/vlength;
                            FaceNormal.ny = FaceNormal.ny/vlength;
                            FaceNormal.nz = FaceNormal.nz/vlength;

                            if (lightmode == 0)
                            {
                                point center;
                                center.x = (p1.x + peven.x + podd.x)/3;
                                center.y = (p1.y + peven.y + podd.y)/3;
                                center.z = (p1.z + peven.z + podd.z)/3;
                                center.r = (p1.r + peven.r + podd.r)/3;
                                center.g = (p1.g + peven.g + podd.g)/3;
                                center.b = (p1.b + peven.b + podd.b)/3;
                                center = LightingCalculation(center, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                                p1.r = center.r;
                                p1.g = center.g;
                                p1.b = center.b;

                                peven.r = p1.r;
                                peven.g = p1.g;
                                peven.b = p1.b;
                                podd.r = p1.r;
                                podd.g = p1.g;
                                podd.b = p1.b;
                            }
                            else
                            {
                                p1 = LightingCalculation(p1, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                                peven = LightingCalculation(peven, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                                podd = LightingCalculation(podd, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                            }
                        }


                        if (filled == 1){
                            DrawTriangleFilled(p1, peven, podd);
                        }
                        else{
                            DrawTriangleWireframe(p1, peven, podd);
                        }
                        currentface++;
                    }
             }
         }
    commandnum++;
    }

}

int Client::CameraBasedDepthShading(point p){
    //cout << "start of DepthShading, p.zprime is " << p.zprime << endl;
    double percentage; // how close point is to the rear clipping plane
    double pz = 1/p.zprime;
    if (p.zprime == 0){
        pz = 0;
    }
    //cout << "DepthSettings, pz is " << pz <<", DepthSettings.near is " << DepthSettings.near << endl;

    if (pz <= DepthSettings.near){
        //cout << "DepthShading, pz is closer than the near fog clipping plane" << endl;
        //cout << "p.r is " << p.r << ", p.g is " << p.g << ", p.b is " << p.b << endl;
        return RepackColor(round(255*p.r), round(255*p.g), round(255*p.b));
    }

    else if (pz >= DepthSettings.far){
       //cout << "Depthshading, pz is past the far fog clipping plane" << endl;
       p.r = DepthSettings.fogcolour.red;
       p.g = DepthSettings.fogcolour.green;
       p.b = DepthSettings.fogcolour.blue;
       return RepackColor(round(255*p.r), round(255*p.g), round(255*p.b));
    }

    else{
        percentage = pz/(DepthSettings.near + DepthSettings.far);
        //cout << "DepthShading, DepthSettings.near is " << DepthSettings.near << ", DepthSettings.far is " << DepthSettings.far << endl;
        //cout << "CameraBased, percentage is " << percentage << endl;
        //cout << "CameraBased, p.r is " << p.r << ", p.g is " << p.g << ", p.b is " << p.b << endl;
        p.r = (percentage*DepthSettings.fogcolour.red) + (1-percentage)*p.r;
        p.g = (percentage*DepthSettings.fogcolour.green) + (1-percentage)*p.g;
        p.b = (percentage*DepthSettings.fogcolour.blue) + (1-percentage)*p.b;
        if (p.r <0){p.r =0;} if(p.g <0){p.g=0;} if(p.b <0){p.b=0;}
        //cout << p.r << "," << p.g << "," << p.b << endl;
        return RepackColor(round(255*p.r), round(255*p.g), round(255*p.b));
    }
    //cout << "end of depthshading" << endl;
}

int Client::SimpReader(string filename, int nearplanecolor, int farplanecolor, bool filled){

    stack <Matrix> transmatrices;

    fstream file;
    file.open(filename.c_str());
    const char* temp[255][255] = {};
    const char* commands[255][255] = {};
    int commandnum = 0;
    int n = 0;

    if (file.fail()){
        cout << "failed to open file" << endl;
        return 1;
    }
    while(!file.eof()){
        //cout << "starting while loop " << endl;
        char line[255];
        file.getline(line, 255);
        n = 0;
        //cout << line << endl;
        temp[commandnum][n] = strtok(line, " \t,.-");

        if (temp[commandnum][n] != 0){
            while(temp[commandnum][n] != 0){
                n++;
                temp[commandnum][n] = strtok(NULL, "\(\) \",");
            }
        }

       int i = 0;
        for(i = 0; i < n;i++){
            commands[commandnum][i] = strdup(temp[commandnum][i]);
            //cout << "commands["<<commandnum<<"]["<< i << "] is " << commands[commandnum][i] << endl;
        }
        commands[commandnum][i] = "!";


        if(commands[commandnum][0] != 0){

            //cout << "Testing what commands[" << commandnum << "][" << 0 << "] is: " << commands[commandnum][0] << endl;
            if (strcmp(commands[commandnum][0], "#") == 0){//strcmp returns 0 if the two strings are equal
                //line is a comment and may be ignored
            }
            else if (strcmp(commands[commandnum][0], "{") == 0){
                //cout << "beginning push operations" << endl;
                transmatrices.push(CTM);
            }

            else if (strcmp(commands[commandnum][0], "}") == 0){
                CTM = transmatrices.top();
                transmatrices.pop();
            }
            else if (strcmp(commands[commandnum][0], "scale") == 0){
                //cout << "beginning scale operations" << endl;
                double xscale = atof(commands[commandnum][1]);
                double yscale = atof(commands[commandnum][2]);
                double zscale = atof(commands[commandnum][3]);
                Matrix Temp;
                Matrix CTMcopy;
                for (int i = 0;i < 4;i++){
                    for (int j = 0; j < 4;j++){
                        if( i ==j){
                            Temp.matrix[i][j] = 1;
                        }
                        else{
                            Temp.matrix[i][j] = 0;
                        }
                        CTMcopy.matrix[i][j] = 0;
                    }
                }
                double xtranslate = CTM.matrix[0][3];
                double ytranslate = CTM.matrix[1][3];
                double ztranslate = CTM.matrix[2][3];
                CTM.matrix[0][3] -= xtranslate;
                CTM.matrix[1][3] -= ytranslate;
                CTM.matrix[2][3] -= ztranslate;

                Temp.matrix[0][0] = xscale;
                Temp.matrix[1][1] = yscale;
                Temp.matrix[2][2] = zscale;

                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        for (int k = 0; k < 4;k++){
                            CTMcopy.matrix[i][j] += CTM.matrix[i][k] * Temp.matrix[k][j];
                        }
                    }
                }

                CTMcopy.matrix[0][3] += xtranslate;
                CTMcopy.matrix[1][3] += ytranslate;
                CTMcopy.matrix[2][3] += ztranslate;
                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        CTM.matrix[i][j] = CTMcopy.matrix[i][j];
                    }
                }

            }

            else if (strcmp(commands[commandnum][0], "rotate") == 0){
                //cout << "beginning rotate operations" << endl;
                double angle = atof(commands[commandnum][2]);
                angle = angle*PI/180;
                Matrix Temp;
                Matrix CTMcopy;
                for (int i = 0;i < 4;i++){
                    for (int j = 0; j < 4;j++){
                        if( i ==j){
                            Temp.matrix[i][j] = 1;
                        }
                        else{
                            Temp.matrix[i][j] = 0;
                        }
                        CTMcopy.matrix[i][j] = 0;
                    }
                }
                double xtranslate = CTM.matrix[0][3];
                double ytranslate = CTM.matrix[1][3];
                double ztranslate = CTM.matrix[2][3];
                CTM.matrix[0][3] -= xtranslate;
                CTM.matrix[1][3] -= ytranslate;
                CTM.matrix[2][3] -= ztranslate;

                if (strcmp(commands[commandnum][1], "X") == 0){
                    Temp.matrix[1][1] = cos(angle);
                    Temp.matrix[1][2] = -sin(angle);
                    Temp.matrix[2][1] = sin(angle);
                    Temp.matrix[2][2] = cos(angle);
                }
                else if (strcmp(commands[commandnum][1], "Y") == 0){
                    Temp.matrix[0][0] = cos(angle);
                    Temp.matrix[2][0] = -sin(angle);
                    Temp.matrix[0][2] = sin(angle);
                    Temp.matrix[2][2] = cos(angle);
                }
                else if (strcmp(commands[commandnum][1], "Z") == 0){
                    Temp.matrix[0][0] = cos(angle);
                    Temp.matrix[0][1] = -sin(angle);
                    Temp.matrix[1][0] = sin(angle);
                    Temp.matrix[1][1] = cos(angle);
                }
                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        for (int k = 0; k < 4;k++){
                            CTMcopy.matrix[i][j] += CTM.matrix[i][k] * Temp.matrix[k][j];
                        }
                    }
                }

                CTMcopy.matrix[0][3] += xtranslate;
                CTMcopy.matrix[1][3] += ytranslate;
                CTMcopy.matrix[2][3] += ztranslate;
                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        CTM.matrix[i][j] = CTMcopy.matrix[i][j];
                    }
                }
            }
            else if (strcmp(commands[commandnum][0], "translate") == 0){
                //cout << "beginning translate operations" << endl;
                double tx = atof(commands[commandnum][1]);
                double ty = atof(commands[commandnum][2]);
                double tz = atof(commands[commandnum][3]);


                CTM.matrix[0][3] = CTM.matrix[0][3] + tx;

                CTM.matrix[1][3] = CTM.matrix[1][3] + ty;

                CTM.matrix[2][3] = CTM.matrix[2][3] + tz;

            }
            else if (strcmp(commands[commandnum][0], "line") == 0){
                point p1, p2;
                double x1,y1,z1,r1,g1,b1,x2,y2,z2,r2,g2,b2;
                if (strcmp(commands[commandnum][7], "!") == 0){ //Line did not have rgb values given
                    x1 = atof(commands[commandnum][1]);
                    y1 = atof(commands[commandnum][2]);
                    z1 = atof(commands[commandnum][3]);
                    x2 = atof(commands[commandnum][4]);
                    y2 = atof(commands[commandnum][5]);
                    z2 = atof(commands[commandnum][6]);
                }
                else{
                    x1 = atof(commands[commandnum][1]);
                    y1 = atof(commands[commandnum][2]);
                    z1 = atof(commands[commandnum][3]);
                    r1 = atof(commands[commandnum][4]);
                    g1 = atof(commands[commandnum][5]);
                    b1 = atof(commands[commandnum][6]);

                    x2 = atof(commands[commandnum][7]);
                    y2 = atof(commands[commandnum][8]);
                    z2 = atof(commands[commandnum][9]);
                    r2 = atof(commands[commandnum][10]);
                    g2 = atof(commands[commandnum][11]);
                    b2 = atof(commands[commandnum][12]);
                }

                p1.x = CTM.matrix[0][0]*x1 + CTM.matrix[0][1]*y1 + CTM.matrix[0][2]*z1 + CTM.matrix[0][3];
                p1.y = CTM.matrix[1][0]*x1 + CTM.matrix[1][1]*y1 + CTM.matrix[1][2]*z1 + CTM.matrix[1][3];
                p1.z = CTM.matrix[2][0]*x1 + CTM.matrix[2][1]*y1 + CTM.matrix[2][2]*z1 + CTM.matrix[2][3];

                p2.x = CTM.matrix[0][0]*x2 + CTM.matrix[0][1]*y2 + CTM.matrix[0][2]*z2 + CTM.matrix[0][3];
                p2.y = CTM.matrix[1][0]*x2 + CTM.matrix[1][1]*y2 + CTM.matrix[1][2]*z2 + CTM.matrix[1][3];
                p2.z = CTM.matrix[2][0]*x2 + CTM.matrix[2][1]*y2 + CTM.matrix[2][2]*z2 + CTM.matrix[2][3];

                if (strcmp(commands[commandnum][7], "!") == 0){ //Line did not have rgb values given
                    p1.r = Surface.red;
                    p1.g = Surface.green;
                    p1.b = Surface.blue;
                    p2.r = Surface.red;
                    p2.g = Surface.green;
                    p2.b = Surface.blue;
                }
                else{
                    p1.r = 255*r1;
                    p1.g = 255*g1;
                    p1.b = 255*b1;
                    p2.r = 255*r2;
                    p2.g = 255*g2;
                    p2.b = 255*b2;
                }
                LineDepthShading(p1,p2, nearplanecolor);
            }
            else if (strcmp(commands[commandnum][0], "polygon") == 0){
                //cout << "beginning polygon operations" << endl;
                point p1,p2,p3;
                double x1,y1,z1,r1,g1,b1,x2,y2,z2,r2,g2,b2,x3,y3,z3,r3,g3,b3;
                if (strcmp(commands[commandnum][10], "!") == 0){ //Polygon did not have rgb values specified
                    x1 = atof(commands[commandnum][1]);
                    y1 = atof(commands[commandnum][2]);
                    z1 = atof(commands[commandnum][3]);
                    x2 = atof(commands[commandnum][4]);
                    y2 = atof(commands[commandnum][5]);
                    z2 = atof(commands[commandnum][6]);
                    x3 = atof(commands[commandnum][7]);
                    y3 = atof(commands[commandnum][8]);
                    z3 = atof(commands[commandnum][9]);
                }
                else{
                    x1 = atof(commands[commandnum][1]);
                    y1 = atof(commands[commandnum][2]);
                    z1 = atof(commands[commandnum][3]);
                    r1 = atof(commands[commandnum][4]);
                    g1 = atof(commands[commandnum][5]);
                    b1 = atof(commands[commandnum][6]);

                    x2 = atof(commands[commandnum][7]);
                    y2 = atof(commands[commandnum][8]);
                    z2 = atof(commands[commandnum][9]);
                    r2 = atof(commands[commandnum][10]);
                    g2 = atof(commands[commandnum][11]);
                    b2 = atof(commands[commandnum][12]);

                    x3 = atof(commands[commandnum][13]);
                    y3 = atof(commands[commandnum][14]);
                    z3 = atof(commands[commandnum][15]);
                    r3 = atof(commands[commandnum][16]);
                    g3 = atof(commands[commandnum][17]);
                    b3 = atof(commands[commandnum][18]);
                }
                Matrix Temp;
                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        if (i == j){
                            Temp.matrix[i][j] = 1;
                        }
                        else{
                            Temp.matrix[i][j] = 0;
                        }
                    }
                }

                for (int i = 0; i < 4; i++){
                    for (int j = 0; j < 4; j++){
                        for (int k = 0; k < 4;k++){
                            Temp.matrix[i][j] += cameramatrix.matrix[i][k] * CTM.matrix[k][j];
                        }
                    }
                }

                /*cout << "the camera matrix is " << endl;
                for(int i = 0; i < 4;i++){
                    for(int j = 0;j < 4;j++){
                        cout << cameramatrix.matrix[i][j] << "\t";
                    }
                    cout << endl;
                }*/

                p1.x = Temp.matrix[0][0]*x1 + Temp.matrix[0][1]*y1 + Temp.matrix[0][2]*z1 + Temp.matrix[0][3];
                p1.y = Temp.matrix[1][0]*x1 + Temp.matrix[1][1]*y1 + Temp.matrix[1][2]*z1 + Temp.matrix[1][3];
                p1.z = Temp.matrix[2][0]*x1 + Temp.matrix[2][1]*y1 + Temp.matrix[2][2]*z1 + Temp.matrix[2][3];

                p2.x = Temp.matrix[0][0]*x2 + Temp.matrix[0][1]*y2 + Temp.matrix[0][2]*z2 + Temp.matrix[0][3];
                p2.y = Temp.matrix[1][0]*x2 + Temp.matrix[1][1]*y2 + Temp.matrix[1][2]*z2 + Temp.matrix[1][3];
                p2.z = Temp.matrix[2][0]*x2 + Temp.matrix[2][1]*y2 + Temp.matrix[2][2]*z2 + Temp.matrix[2][3];

                p3.x = Temp.matrix[0][0]*x3 + Temp.matrix[0][1]*y3 + Temp.matrix[0][2]*z3 + Temp.matrix[0][3];
                p3.y = Temp.matrix[1][0]*x3 + Temp.matrix[1][1]*y3 + Temp.matrix[1][2]*z3 + Temp.matrix[1][3];
                p3.z = Temp.matrix[2][0]*x3 + Temp.matrix[2][1]*y3 + Temp.matrix[2][2]*z3 + Temp.matrix[2][3];

                //---------------------------------------Lighting Calculations-------------------------------------------------------------

                if (strcmp(commands[commandnum][10], "!") == 0){ //Polygon did not have rgb values specified
                    p1.r = (Surface.red)*ambientlight.red;
                    p1.g = (Surface.green)*ambientlight.green;
                    p1.b = (Surface.blue)*ambientlight.blue;
                    p2.r = (Surface.red)*ambientlight.red;
                    p2.g = (Surface.green)*ambientlight.green;
                    p2.b = (Surface.blue)*ambientlight.blue;
                    p3.r = (Surface.red)*ambientlight.red;
                    p3.g = (Surface.green)*ambientlight.green;
                    p3.b = (Surface.blue)*ambientlight.blue;
                }
                else{
                    p1.r = (r1*ambientlight.red);
                    p1.g = (g1*ambientlight.green);
                    p1.b = (b1*ambientlight.blue);
                    p2.r = (r2*ambientlight.red);
                    p2.g = (g2*ambientlight.green);
                    p2.b = (b2*ambientlight.blue);
                    p3.r = (r3*ambientlight.red);
                    p3.g = (g3*ambientlight.green);
                    p3.b = (b3*ambientlight.blue);
                }
                //cout << p1.x << "," << p1.y << "," << p1.z << "\t " << p2.x << "," << p2.y << "," << p2.z << "\t " << p3.x << "," << p3.y << "," << p3.z << endl;
                //cout << p1.r << "," << p1.g << "," << p1.b << endl;
                //PolygonDepthShading(p1, p2, p3, nearplanecolor, farplanecolor, filled);
                //Normal calculations
                VertexNorm V, W, FaceNormal;
                V.nx = p2.x - p1.x;
                V.ny = p2.y - p1.y;
                V.nz = p2.z - p1.z;

                W.nx = p3.x - p1.x;
                W.ny = p3.y - p1.y;
                W.nz = p3.z - p1.z;
                FaceNormal.nx = V.ny*W.nz - V.nz*W.ny;
                FaceNormal.ny = V.nz*W.nx - V.nx*W.nz;
                FaceNormal.nz = V.nx*W.ny - V.ny*W.nx;
                double vlength = VectorNormal(FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                FaceNormal.nx = FaceNormal.nx/vlength;
                FaceNormal.ny = FaceNormal.ny/vlength;
                FaceNormal.nz = FaceNormal.nz/vlength;
                cameralocation = {Temp.matrix[0][4], Temp.matrix[1][4],0,0,0,Temp.matrix[2][4]};
                point camtemp;
                camtemp.x = cameralocation.x;
                camtemp.y = cameralocation.y;
                camtemp.z = cameralocation.z;
                if (lightmode == 0)//flat shading
                {
                    point center;
                    center.x = (p1.x + p2.x + p3.x)/3;
                    center.y = (p1.y + p2.y + p3.y)/3;
                    center.z = (p1.z + p2.z + p3.z)/3;
                    center.r = (p1.r + p2.r + p3.r)/3;
                    center.g = (p1.g + p2.g + p3.g)/3;
                    center.b = (p1.b + p2.b + p3.b)/3;
                    center = LightingCalculation(center, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                    p1.r = center.r;
                    p1.g = center.g;
                    p1.b = center.b;

                    p2.r = p1.r;
                    p2.g = p1.g;
                    p2.b = p1.b;
                    p3.r = p1.r;
                    p3.g = p1.g;
                    p3.b = p1.b;
                }
                else//gouraud shading or phong shading
                {
                    //cout << "Gouraud or phong shading" << endl;
                    p1 = LightingCalculation(p1, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                    p2 = LightingCalculation(p2, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                    p3 = LightingCalculation(p3, camtemp, FaceNormal.nx, FaceNormal.ny, FaceNormal.nz);
                }
                //cout << "back in SimpDrawer, p1 colour is " << p1.r << "," << p1.g << "," << p1.b << endl;
                //cout << "p2 colour is " << p2.r << "," << p2.g << "," << p2.b << endl;
                //cout << "p3 colour is " << p3.r << "," << p3.g << "," << p3.b << endl;

                DrawTriangleFilled(p1, p2, p3);

            }
            else if (strcmp(commands[commandnum][0], "file") == 0){
                //cout << "starting file operations" << endl;
                string newfilename = commands[commandnum][1];
                newfilename.append(".simp");
                //cout << "before starting simpreader" << endl;
                SimpReader(newfilename, nearplanecolor, farplanecolor, filled);
            }
            else if (strcmp(commands[commandnum][0], "obj") == 0){
                //cout << "opening obj file" << endl;
                string newfilename = commands[commandnum][1];
                newfilename.append(".obj");
                ObjReader(newfilename, filled);
            }
            else if (strcmp(commands[commandnum][0], "surface") == 0){
                Surface.red = atof(commands[commandnum][1]);
                Surface.green = atof(commands[commandnum][2]);
                Surface.blue = atof(commands[commandnum][3]);
                Surface.ks = atof(commands[commandnum][4]);
                Surface.p = atof(commands[commandnum][5]);
            }
            else if (strcmp(commands[commandnum][0], "ambient") == 0){
                ambientlight.red = atof(commands[commandnum][1]);
                ambientlight.green = atof(commands[commandnum][2]);
                ambientlight.blue = atof(commands[commandnum][3]);
            }
            else if (strcmp(commands[commandnum][0], "camera") == 0){
                cameramatrix.matrix[0][0] = CTM.matrix[0][0];
                cameramatrix.matrix[0][1] = CTM.matrix[1][0];
                cameramatrix.matrix[0][2] = CTM.matrix[2][0];
                cameramatrix.matrix[0][3] = (CTM.matrix[0][0]*-CTM.matrix[0][3] + CTM.matrix[1][0]*-CTM.matrix[1][3] + CTM.matrix[2][0]*-CTM.matrix[2][3]);

                cameramatrix.matrix[1][0] = CTM.matrix[0][1];
                cameramatrix.matrix[1][1] = CTM.matrix[1][1];
                cameramatrix.matrix[1][2] = CTM.matrix[2][1];
                cameramatrix.matrix[1][3] = (CTM.matrix[0][1]*-CTM.matrix[0][3] + CTM.matrix[1][1]*-CTM.matrix[1][3] + CTM.matrix[2][1]*-CTM.matrix[2][3]);

                cameramatrix.matrix[2][0] = CTM.matrix[0][2];
                cameramatrix.matrix[2][1] = CTM.matrix[1][2];
                cameramatrix.matrix[2][2] = CTM.matrix[2][2];
                cameramatrix.matrix[2][3] = (CTM.matrix[0][2]*-CTM.matrix[0][3] + CTM.matrix[1][2]*-CTM.matrix[1][3] + CTM.matrix[2][2]*-CTM.matrix[2][3]);

                cameramatrix.matrix[3][0] = 0;
                cameramatrix.matrix[3][1] = 0;
                cameramatrix.matrix[3][2] = 0;
                cameramatrix.matrix[3][3] = 1;

                cameralocation.x = cameramatrix.matrix[0][3];
                cameralocation.y = cameramatrix.matrix[1][3];
                cameralocation.z = cameramatrix.matrix[2][3];

                CamSettings.xlow = atof(commands[commandnum][1]);
                CamSettings.ylow = atof(commands[commandnum][2]);
                CamSettings.xhigh = atof(commands[commandnum][3]);
                CamSettings.yhigh = atof(commands[commandnum][4]);
                CamSettings.hither = atof(commands[commandnum][5]);
                CamSettings.yon = atof(commands[commandnum][6]);
                ResetZBuffer(CamSettings.yon);

            }

            else if (strcmp(commands[commandnum][0], "depth") == 0){
                DepthSettings.near = atof(commands[commandnum][1]);
                DepthSettings.far = atof(commands[commandnum][2]);
                DepthSettings.fogcolour.red = atof(commands[commandnum][3]);
                DepthSettings.fogcolour.green = atof(commands[commandnum][4]);
                DepthSettings.fogcolour.blue = atof(commands[commandnum][5]);
            }
            else if (strcmp(commands[commandnum][0], "light") == 0){
                lightsource newlight;
                newlight.x = CTM.matrix[0][3];
                newlight.y = CTM.matrix[1][3];
                newlight.z = CTM.matrix[2][3];
                newlight.r = atof(commands[commandnum][1]);
                newlight.g = atof(commands[commandnum][2]);
                newlight.b = atof(commands[commandnum][3]);
                newlight.atta = atof(commands[commandnum][4]);
                newlight.attb = atof(commands[commandnum][5]);
                lights[lightcount] = newlight;
                lightcount++;
            }
            else if (strcmp(commands[commandnum][0], "phong") == 0){
                lightmode = 2;
            }
            else if (strcmp(commands[commandnum][0], "gouraud") == 0){
                lightmode = 1;
            }
            else if (strcmp(commands[commandnum][0], "flat") == 0){
                lightmode = 0;
            }
            else if (strcmp(commands[commandnum][0], "wire") == 0){
                filled = 0;
            }
            else if (strcmp(commands[commandnum][0], "filled") == 0){
                filled = 1;
            }

        }

        /*cout << "after line #" << commandnum << " the transformation matrix is " << endl;
        for(int i = 0; i < 4;i++){
            for(int j = 0;j < 4;j++){
                cout << CTM.matrix[i][j] << "\t";
            }
            cout << endl;
        }*/
        commandnum++;
        if (commandnum > 254){
            cout << "Something has gone wrong" << endl;
            break;
        }
    }

    file.close();
    //cout << "File has been closed " << endl;
    return 0;
}

Client::point Client::LightingCalculation(point location, point cameralocation, double vnx, double vny, double vnz){
    //function begins with surface times ambient light, will iterate through a while loop for every light in the scene
    int iterations = 0;
    //cout << "At the beginning of Lighting Calculation, the original color is " << location.r << "," << location.g << "," << location.b << endl;
    double totalredintensity = 0;
    double totalgreenintensity = 0;
    double totalblueintensity = 0;
    while (iterations < lightcount)//while loop to get the effects of all the lights
    {
        lightsource curlight = lights[iterations];
        //cout << "light #" << iterations+1 << " has the colours " << curlight.r << "," << curlight.g << "," << curlight.b << endl;
        double red_intensity, green_intensity, blue_intensity;
        double distance = (VectorNormal(curlight.x - location.x, curlight.y - location.y, curlight.z - location.z));
        double fatt = 1/((curlight.atta) + ((curlight.attb) * distance));
        //cout << "fatt is " << fatt << endl;
        VertexNorm V, L, R;
        V.nx = cameralocation.x - location.x;
        V.ny = cameralocation.y - location.y;
        V.nz = cameralocation.z - location.z;
        double length = VectorNormal(V.nx, V.ny, V.nz);
        V.nx = V.nx/length;
        V.ny = V.ny/length;
        V.nz = V.nz/length;

        L.nx = curlight.x - location.x;
        L.ny = curlight.y - location.y;
        L.nz = curlight.z - location.z;
        length = VectorNormal(L.nx, L.ny, L.nz);
        L.nx = L.nx/length;
        L.ny = L.ny/length;
        L.nz = L.nz/length;

        double NLdotproduct = (vnx*L.nx) + (vny*L.ny) + (vnz*L.nz);
        R.nx = 2*vnx*NLdotproduct - L.nx;
        R.ny = 2*vny*NLdotproduct - L.ny;
        R.nz = 2*vnz*NLdotproduct - L.nz;

        double VRdotproduct = (V.nx*R.nx + V.ny*R.ny + V.nz*R.nz);
        double part3 = Surface.ks*(pow(VRdotproduct, Surface.p));
        red_intensity = (curlight.r)*(fatt)*(location.r*NLdotproduct + part3);
        green_intensity = (curlight.g)*(fatt)*(location.g*NLdotproduct + part3);
        blue_intensity = (curlight.b)*(fatt)*(location.b*NLdotproduct + part3);

        totalredintensity = totalredintensity + red_intensity;
        totalgreenintensity = totalgreenintensity + green_intensity;
        totalblueintensity = totalblueintensity + blue_intensity;
        iterations++;
    }
    //cout << "After iterating through the effects of the lightsources, the total intensities of the lights are " << totalredintensity << "," << totalgreenintensity << "," << totalblueintensity << endl;
    location.r = location.r + totalredintensity;
    location.g = location.g + totalgreenintensity;
    location.b = location.b + totalblueintensity;
    //cout << "The new colour is " << location.r << "," << location.g << "," << location.b << endl;
    return location;
}

double Client::VectorNormal(double x, double y, double z){
    double length = sqrt(((x)*(x)) + ((y)*(y)) + ((z)*(z)));
    return length;
}

void Client::TriangleRotation(float colour){
    int i = rand()%121;
    int z = rand()%200;
    float angle = (i)*PI/180;
    int adjacent = round(275 * cos(angle));
    int opposite = round(275 * sin(angle));
    point p1, p2, p3;
    p1 = {375 + adjacent, 375 - opposite, round(255*colour), round(255*colour), round(255*colour), z};
    angle = (i+120)*PI/180;
    adjacent = round(275*(cos(angle)));
    opposite = round(275*(sin(angle)));
    p2 = {375 + adjacent, 375 - opposite, round(255*colour), round(255*colour), round(255*colour), z};
    angle = (i+240)*PI/180;
    adjacent = round(275*(cos(angle)));
    opposite = round(275*(sin(angle)));
    p3 = {375 + adjacent, 375 - opposite, round(255*colour), round(255*colour), round(255*colour), z};
    PolygonDepthShading(p1, p2, p3, 0xffffffff, 0x00000000, 1);
}

int Client::UnpackRed(int color){
    int red = (color & 0x00ff0000) >> 16;
    return red;
}

int Client::UnpackGreen(int color){
    int green = (color & 0x0000ff00) >> 8;
    return green;
}

int Client::UnpackBlue(int color){
    int blue = (color & 0x000000ff);
    return blue;
}

int Client::RandomColor(int color){
    int red = UnpackRed(color);
    int green = UnpackGreen(color);
    int blue = UnpackBlue(color);
    red = (red * rand())%255;
    green = (green * rand())%255;
    blue = (blue * rand())%255;
    if (red == 0){
        red = 128;
    }
    if (green == 0){
        green = 128;
    }
    if (blue == 0){
        blue = 128;
    }
    color = RepackColor(red, green, blue);
    return color;
}

int Client::ColorBlend(int x, int y, int color, float opacity){
    int oldcolor = drawable->getPixel(x,y);
    int oldred = UnpackRed(oldcolor);
    int oldgreen = UnpackGreen(oldcolor);
    int oldblue = UnpackBlue(oldcolor);
    int newred = UnpackRed(color);
    int newgreen = UnpackGreen(color);
    int newblue = UnpackBlue(color);
    newred = float(opacity)*newred + (1 - float(opacity))*oldred;
    newgreen = float(opacity)*newgreen + (1 - float(opacity))*oldgreen;
    newblue = float(opacity)*newblue + (1 - float(opacity))*oldblue;
    color = RepackColor(newred, newgreen, newblue);
    return color;
}

int Client::RepackColor(int red, int green, int blue){
    int color = (0xff << 24) + ((red & 0xff) << 16) + ((green & 0xff) << 8) + (blue & 0xff);
    return color;
}

int Client::WhichOctant(int x1, int y1, int x2, int y2){
    if (y1 > y2){ //octants 1-4
        if(x1 < x2){ //octants 1-2
            if (abs(x1 - x2) >= abs(y1 - y2)){ //Octant 1
                return 1;
            }
            else{//Octant 2
                return 2;
            }
        }
        else{ //octants 3-4
            if (abs(x1 - x2) >= abs(y1 - y2)){ //Octant 4
                return 4;
            }
            else{//Octant 3
                return 3;
            }
        }
    }
    else{ // Octants 5-8
        if (x1 < x2){ //Octants 7-8
            if (abs(x1 - x2) >= abs(y1 - y2)){ //Octant 8
                return 8;
            }
            else{//Octant 7
                return 7;
            }
        }
        else{ //Octants 5-6
            if (abs(x1 - x2) >= abs(y1 - y2)){ //Octant 5
                return 5;
            }
            else{//Octant 6
                return 6;
            }
        }
    }
}

void Client::CallDDA(point p1, point p2){
    int octant = WhichOctant(p1.x, p1.y, p2.x, p2.y);
    if (octant == 1){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA1(p1, p2);
    }
    else if (octant == 2){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA2(p1, p2);
    }
    else if (octant == 3){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA3(p1, p2);
    }
    else if (octant == 4){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA4(p1, p2);
    }
    else if (octant == 5){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA5(p1, p2);
    }
    else if (octant == 6){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA6(p1, p2);
    }
    else if (octant == 7){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA7(p1, p2);
    }
    else if (octant == 8){
        //cout << "For the line at " << p1.y << " using DDA" << octant << endl;
        DDA8(p1, p2);
    }
}

void Client::DDA1(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;

    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(x2 - x1);
    float dg = float(g2 - g1)/(x2 - x1);
    float db = float(b2 - b1)/(x2 - x1);
    float dz = float(z2 - z1)/(x2 - x1);
    double dzprime = double(zprime2 - zprime1)/(x2 - x1);

    float m = float(y2 - y1)/(x2 - x1);
    float b = y1 - m*x1;
    for ( int x = x1; x <= x2; x = x + 1){
        float y = m*x + b;
        currentred = currentred + dr;
        currentgreen = currentgreen + dg;
        currentblue = currentblue + db;
        currentz = currentz + dz;
        currentzprime = currentzprime + dzprime;
        int ypos = round(y);
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[ypos][x])){
                point pixel = {x, round(y), currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable ->setPixel(x, round(y), color);
                zbuffer[ypos][x] = currentz;
                //cout << "DDA1, currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA2(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(y2 - y1);
    float dg = float(g2 - g1)/(y2 - y1);
    float db = float(b2 - b1)/(y2 - y1);
    float dz = float(z2 - z1)/(y2 - y1);
    double dzprime = (zprime2 - zprime1)/(y2 - y1);

    float m = float(x2 - x1)/(y2 - y1);
    float b = x1 - m*y1;
    for (int y = y1; y >= y2; y = y - 1){
        float x = m*y + b;
        currentred = currentred - dr;
        currentgreen = currentgreen - dg;
        currentblue = currentblue - db;
        currentz = currentz - dz;
        currentzprime = currentzprime - dzprime;
        int xpos = round(x);
        color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[y][xpos])){
                point pixel = {round(x), y, currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable->setPixel(round(x), y, color);
                zbuffer[y][xpos] = currentz;
                //cout << "DDA2, currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA3(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = z1;

    float dr = float(r2 - r1)/(y2 - y1);
    float dg = float(g2 - g1)/(y2 - y1);
    float db = float(b2 - b1)/(y2 - y1);
    float dz = float(z2 - z1)/(y2 - y1);
    double dzprime = (zprime2 - zprime1)/(y2 - y1);

    float m = float(x2 - x1)/(y2 - y1);
    float b = x1 - m*y1;
    for (int y = y1; y >= y2; y = y - 1){
        float x = m*y + b;
        currentred = currentred - dr;
        currentgreen = currentgreen - dg;
        currentblue = currentblue - db;
        currentz = currentz - dz;
        currentzprime = currentzprime - dzprime;
        int xpos = round(x);
        color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[y][xpos])){
                point pixel = {x, round(y), currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable->setPixel(round(x), y, color);
                zbuffer[y][xpos] = currentz;
                //cout << "DDA 3,currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA4(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(x2 - x1);
    float dg = float(g2 - g1)/(x2 - x1);
    float db = float(b2 - b1)/(x2 - x1);
    float dz = float(z2 - z1)/(x2 - x1);
    double dzprime = (zprime2 - zprime1)/(x2 - x1);

    float m = float(y2 - y1)/(x2 - x1);
    float b = y1 - m*x1;
    for ( int x = x1; x >= x2; x = x - 1){
        float y = m*x + b;
        currentred = currentred - dr;
        currentgreen = currentgreen - dg;
        currentblue = currentblue - db;
        currentz = currentz - dz;
        currentzprime = currentzprime - dzprime;
        int ypos = round(y);
        color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[ypos][x])){
                point pixel = {x, round(y), currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable ->setPixel(x, round(y), color);
                zbuffer[ypos][x] = currentz;
                //cout << "DDA4, currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA5(point p1, point p2){
    //cout << "dda5" << endl;
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(x2 - x1);
    float dg = float(g2 - g1)/(x2 - x1);
    float db = float(b2 - b1)/(x2 - x1);
    float dz = float(z2 - z1)/(x2 - x1);
    double dzprime = (zprime2 - zprime1)/(x2 - x1);

    float m = float(y2 - y1)/(x2 - x1);
    float b = y1 - m*x1;

    for ( int x = x1; x >= x2; x = x - 1){
        float y = m*x + b;
        currentred = currentred - dr;
        currentgreen = currentgreen - dg;
        currentblue = currentblue - db;
        currentz = currentz - dz;
        currentzprime = currentzprime - dzprime;
        int ypos = round(y);
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[ypos][x])){
                point pixel = {x, round(y), currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable ->setPixel(x, round(y), color);
                zbuffer[ypos][x] = currentz;
                //cout << "DDA5, currentz is " << currentz << endl;
            }
        }
    }
}
void Client::DDA6(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(y2 - y1);
    float dg = float(g2 - g1)/(y2 - y1);
    float db = float(b2 - b1)/(y2 - y1);
    float dz = float(z2 - z1)/(y2 - y1);
    double dzprime = (zprime2 - zprime1)/(y2 - y1);

    float m = float(x2 - x1)/(y2 - y1);
    float b = x1 - m*y1;
    for (int y = y1; y <= y2; y = y + 1){
        float x = m*y + b;
        currentred = currentred + dr;
        currentgreen = currentgreen + dg;
        currentblue = currentblue + db;
        currentz = currentz + dz;
        currentzprime = currentzprime + dzprime;
        int xpos = round(x);
        color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[y][xpos])){
                point pixel = {round(x), y, currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable->setPixel(round(x), y, color);
                zbuffer[y][xpos] = currentz;
                //cout << "DDA6, currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA7(point p1, point p2){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;

    float dr = float(r2 - r1)/(y2 - y1);
    float dg = float(g2 - g1)/(y2 - y1);
    float db = float(b2 - b1)/(y2 - y1);
    float dz = float(z2 - z1)/(y2 - y1);
    double dzprime = (zprime2 - zprime1)/(y2 - y1);

    float m = float(x2 - x1)/(y2 - y1);
    float b = x1 - m*y1;
    for (int y = y1; y <= y2; y = y + 1){
        float x = m*y + b;
        currentred = currentred + dr;
        currentgreen = currentgreen + dg;
        currentblue = currentblue + db;
        currentz = currentz + dz;
        currentzprime = currentzprime + dzprime;
        int xpos = round(x);
        color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[y][xpos])){
                point pixel = {round(x), y, currentred, currentgreen, currentblue, currentz, currentzprime};
                color = CameraBasedDepthShading(pixel);
                drawable->setPixel(round(x), y, color);
                zbuffer[y][xpos] = currentz;
                //cout << "DDA7, currentz is " << currentz << endl;
            }
        }
    }
}

void Client::DDA8(point p1, point p2){
    //cout << "dda8" << endl;
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    double r1 = p1.r;
    double g1 = p1.g;
    double b1 = p1.b;
    double r2 = p2.r;
    double g2 = p2.g;
    double b2 = p2.b;
    double z1 = p1.z;
    double z2 = p2.z;
    double zprime1 = p1.zprime;
    double zprime2 = p2.zprime;
    double vnx1 = p1.vnx;
    double vny1 = p1.vny;
    double vnz1 = p1.vnz;
    double vnx2 = p2.vnx;
    double vny2 = p2.vny;
    double vnz2 = p2.vnz;
    int color = RepackColor(r1, g1, b1);
    float currentred = r1;
    float currentgreen = g1;
    float currentblue = b1;
    double currentz = z1;
    double currentzprime = zprime1;
    double currentvnx = vnx1;
    double currentvny = vny1;
    double currentvnz = vnz1;

    float dr = float(r2 - r1)/(x2 - x1);
    float dg = float(g2 - g1)/(x2 - x1);
    float db = float(b2 - b1)/(x2 - x1);
    float dz = float(z2 - z1)/(x2 - x1);
    double dzprime = (zprime2 - zprime1)/(x2 - x1);
    double dvnx = (vnx2 - vnx1)/(x2 - x1);
    double dvny = (vny2 - vny1)/(x2 - x1);
    double dvnz = (vnz2 - vnz1)/(x2 - x1);

    float m = float(y2 - y1)/(x2 - x1);
    float b = y1 - m*x1;

    for ( int x = x1; x <= x2; x = x + 1){
        float y = m*x + b;
        currentred = currentred + dr;
        currentgreen = currentgreen + dg;
        currentblue = currentblue + db;
        currentz = currentz + dz;
        currentzprime = currentzprime + dzprime;
        currentvnx = currentvnx + dvnx;
        currentvny = currentvny + dvny;
        currentvnz = currentvnz + dvnz;
        double length = VectorNormal(currentvnx, currentvny, currentvnz);
        if (length == 0)
        {
            length = 1;
        }
        currentvnx = currentvnx/length;
        currentvny = currentvny/length;
        currentvnz = currentvnz/length;
        int ypos = round(y);
        //color = RepackColor(round(currentred), round(currentgreen), round(currentblue));
        //cout << "round(y)/currentz is " << round(y)/currentz << endl;
        //cout << "round(x)/currentz is " << round(x)/currentz << endl;
        if ((CamSettings.ylow <= round(y)/currentz) && (round(y)/currentz <= CamSettings.yhigh) && (CamSettings.xlow <= round(x)/currentz) && (round(x)/currentz <= CamSettings.xhigh)){
            if ((currentz >= 0) && (currentz < zbuffer[ypos][x])){
                point pixel = {x, round(y), currentred, currentgreen, currentblue, currentz, currentzprime, currentvnx, currentvny, currentvnz};
                point camtemp;
                camtemp.x = cameralocation.x;
                camtemp.y = cameralocation.y;
                camtemp.z = cameralocation.z;
                if (lightmode == 2) // phong shading
                {
                    pixel = LightingCalculation(pixel, camtemp, currentvnx, currentvny, currentvnz);
                }
                color = CameraBasedDepthShading(pixel);
                drawable ->setPixel(x, round(y), color);
                zbuffer[ypos][x] = currentz;
                //cout << "DDA8, currentz is " << currentz << endl;
            }
        }
    }
    //cout << "CamSettings :" << CamSettings.xlow << " " << CamSettings.xhigh << " " << CamSettings.ylow << " " << CamSettings.yhigh << endl;
}

