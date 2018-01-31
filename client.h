#ifndef CLIENT_H
#define CLIENT_H
#include "drawable.h"
#include "pageturner.h"
#include <string>
class Client : public PageTurner
{
public:
    Client(Drawable *drawable, char *argv[]);
    void nextPage();

private:
    struct point{
        double x, y, r, g, b, z, zprime, vnx, vny, vnz;
    };

    struct matrix{
        double matrix[4][4];
    };

    Drawable *drawable;
    char **argv;

    void Setup();
    void draw_rect(int x1, int y1, int x2, int y2, unsigned int color);

    void CallDDA(point p1, point p2);
    int WhichOctant(int x1, int y1, int x2, int y2);
    void DDA1(point p1, point p2);
    void DDA2(point p1, point p2);
    void DDA3(point p1, point p2);
    void DDA4(point p1, point p2);
    void DDA5(point p1, point p2);
    void DDA6(point p1, point p2);
    void DDA7(point p1, point p2);
    void DDA8(point p1, point p2);

    void DrawTriangleWireframe(point p1, point p2, point p3);
    void DrawTriangleFilled(point p1, point p2, point p3);
    void TriangleRotation(float colour);
    void PolygonDepthShading(point p1, point p2, point p3, int nearplanecolor, int farplanecolor, bool filled);
    void LineDepthShading(point p1, point p2, int nearplanecolor);
    int CameraBasedDepthShading(point p);

    int SimpReader(std::string filename, int nearplanecolor, int farplanecolor, bool filled);
    int ObjReader(std::string filename, bool filled);
    double FlatShading(double vnx, double vny, double vnz);
    double VectorNormal(double x, double y, double z);
    point LightingCalculation(point pixel, point cameralocation, double vnx, double vny, double vnz);

    void ResetZBuffer(double farplane);
    void InitializeCTM();
    void GridOfPoints(int topleftx, int toplefty, int offset, struct point points[], int spacing);
    void GridOfTriangles(struct point points[], float opacity);
    void DrawWireframe(struct point points[]);

    int ColorBlend(int x, int y, int color, float opacity);
    int UnpackRed(int color);
    int UnpackGreen(int color);
    int UnpackBlue(int color);
    int RandomColor(int color);
    int RepackColor(int red, int green, int blue);

};

#endif // CLIENT_H
