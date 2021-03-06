// Test_Vector.cpp : Defines the entry point for the console application.
//

#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <exception>
#include <cmath>
#include <vector>
#include "A4.h"
#include "MyCanny.h"
#include "A4Warpping.h"

#define PREFIX "cached_"
#define POSTFIX ".bmp"
#define DEFAULT_SYMBOL "."

using namespace std;

typedef cimg_library::CImg<unsigned char> Img;

void testWarpping(const int *x, const int *y, int *seq, int index) {
    if (index == 4) {
        int xx[4], yy[4];
        for (int i = 0; i < 4; i++) {
            xx[i] = x[seq[i]];
            yy[i] = y[seq[i]];
        }
        for (int i = 0; i < 4; i++)
            cout << xx[i] << " ";
        for (int i = 0; i < 4; i++)
            cout << yy[i] << " ";
        cout << "   ";
        A4Warpping::adjust(xx, yy);
        for (int i = 0; i < 4; i++)
            cout << xx[i] << " ";
        for (int i = 0; i < 4; i++)
            cout << yy[i] << " ";
        cout << endl;
        return;
    }
    testWarpping(x, y, seq, index + 1);
    for (int i = 1; i < 4 - index; i++) {
        int temp = seq[index];
        seq[index] = seq[index + i];
        seq[index + i] = temp;
        testWarpping(x, y, seq, index + 1);
        temp = seq[index];
        seq[index] = seq[index + i];
        seq[index + i] = temp;
    }
}

int main() {
    setbuf(stdout, NULL);

    bool showEdge, showHough, showLocalMax, showEquationAndIntersection;
    cout << "本程序对包含A4纸的bmp图像进行处理，裁剪得到比例正确的A4纸\n";
    cout << "enter 4 boolean value for whether display the edge, the hough space, local max and show the linear equations and intersections:\n-->";
    cin >> showEdge >> showHough >> showLocalMax >> showEquationAndIntersection;
    cout << "enter the filename each time in one line. (default parameters are used)\n";
    A4Warpping a4Warpping;
    A4 a4(showHough, showLocalMax, showEquationAndIntersection);
    MyCanny canny(0, 0, 0);
    string s, srcName;
    Img cached_edge;
    A4::Hough cached_hough;
    Img cached_result;
    A4::Points cached_points;
    Img cached_A4;
    while (true) {
        cout << "->";
        cin.sync();
        cin.clear();
        char in[1000] = { 0 };
        string filename;
        cin.getline(in, 1000);
        if (in[0] == '\0')
            continue;
        stringstream ss;
        ss << in;
        ss >> s;
        try {
            /**
             * 注意：以下文件名不需要写后缀.bmp
             * 1  边缘检测
             * 1  filename
             * 1  filename lowthreshold highthreshold gaussiankernelradius gaussiankernelwidth contrastnormalised
             *
             * 2  Hough
             * 2                        # 使用刚才上一步的结果
             * 2  .                     # 使用硬盘缓存
             * 2  filename
             * 2  filename precision
             * 2  filename width height
             *
             * 3  选择直线                                        # 依赖上一步
             * 3                        # 使用默认参数
             * 3  scale
             *
             * 4  绘制直线                                        # 依赖上一步
             * 4                        # 在空白图片上作图
             * 4  .                     # 使用第一步的图片
             * 4  filename
             * 4  filename red green blue
             *
             * 5  Warpping              # 依赖上一步
             * 5  interpolation .       # 使用第一步的图片
             * 5  interpolation filename
             *
             * 11,22,44,55分别保存最近的一次结果
             */
            if (s == "1") {  // 边缘检测
                ss >> srcName;
                if (!ss.good()) {
                    cached_edge = canny((srcName + POSTFIX).c_str(), 2.5f, 7.5f,
                                        7.0f, 16, 0);
                } else {
                    float lowthreshold = 2.5, highthreshold = 7.5,
                            gaussiankernelradius = 7;
                    int gaussiankernelwidth = 16, contrastnormalised = 0;
                    ss >> lowthreshold >> highthreshold >> gaussiankernelradius
                            >> gaussiankernelwidth >> contrastnormalised;
                    cached_edge = canny((srcName + POSTFIX).c_str(),
                                        lowthreshold, highthreshold,
                                        gaussiankernelradius,
                                        gaussiankernelwidth,
                                        contrastnormalised);
                }
                cimg_forXY(cached_edge, x, y)
                {
                    if (cached_edge(x, y))
                        cached_edge(x, y) = 255;
                }
                cached_edge.display("edge", false);
            } else if (s == "2") {  // 求Hough Space
                Img edge;
                if (!ss.good()) {
                    edge = cached_edge;
                    cached_hough = a4.houghSpace(edge);
                } else {
                    ss >> filename;
                    if (filename == DEFAULT_SYMBOL)
                        filename = PREFIX"edge";
                    edge.load((filename + POSTFIX).c_str());
                    double precision = 0.2;
                    ss >> precision;
                    if (!ss.good()) {
                        cached_hough = a4.houghSpace(edge, precision);
                    } else {
                        int width = precision, height = 500;
                        ss >> height;
                        cached_hough = a4.houghSpace(edge, width, height);
                    }
                }
                a4.displayHough();
            } else if (s == "3") {  // 选出直线方程
                if (!ss.good()) {
                    a4.find4Lines();
                } else {
                    double scale;
                    ss >> scale;
                    a4.find4Lines(scale);
                }
                cached_points = a4.calcPoints();
                a4.printEquationsAndIntersections();
                a4.displayLocalMax();
            } else if (s == "4") {  // 在指定图上绘制直线
                if (!ss.good()) {
                    cached_result = a4.drawLinesAndPoints();
                } else {
                    ss >> filename;
                    if (filename == DEFAULT_SYMBOL)
                        filename = srcName;
                    cached_result.load((filename + POSTFIX).c_str());
                    if (!ss.good()) {
                        cached_result = a4.drawLinesAndPoints(cached_result);
                    } else {
                        unsigned char color[3] = { 0 };
                        ss >> color[0];
                        ss >> color[1];
                        ss >> color[2];
                        cached_result = a4.drawLinesAndPoints(cached_result,
                                                              color);
                    }
                }
                cached_result.display("result", false);
            } else if (s == "5") {  // 进行A4纸Warpping
                bool interpolation = false;
                ss >> interpolation;
                if (!ss.good()) {
                    cout << "please input filename" << endl;
                } else {
                    ss >> filename;
                    if (filename == DEFAULT_SYMBOL)
                        filename = srcName;
                    Img src((filename + POSTFIX).c_str());
                    int x[4], y[4];
                    for (int i = 0; i < 4; i++) {
                        x[i] = cached_points[i].x;
                        y[i] = cached_points[i].y;
                    }
                    cached_A4 = a4Warpping(src, x, y, interpolation);
                    cached_A4.display("A4", false);
                }
            } else if (s == "11") {
                cached_edge.save(PREFIX"edge"POSTFIX);
            } else if (s == "22") {
                cached_hough.get_normalize(0, 255).save(PREFIX"hough"POSTFIX);
            } else if (s == "44") {
                cached_result.save(PREFIX"result"POSTFIX);
            } else if (s == "55") {
                cached_A4.save(PREFIX"A4"POSTFIX);
            } else if (s == "555") {  // 测试调整顺序功能
                int x[] = { 0, 2, 2, 0 };
                int y[] = { 3, 3, 0, 0 };
                int seq[] = { 0, 1, 2, 3 };
                testWarpping(x, y, seq, 0);
            } else if (s == "5552") {  // 单例测试矩阵求解功能
                double mm[][4] = { { 0, 1, 1, 0 }, { 1, 0, 1, 1 }, { 0.5,
                                                                     0,
                                                                     1,
                                                                     0 } };
                double *m[] = { mm[0], mm[1], mm[2] };
                A4Warpping::solve(m, 3, 4);
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 4; j++)
                        cout << m[i][j] << " ";
                    cout << endl;
                }
            } else if (s == "5553") {  // 测试三点形变
                Img src(201, 201);
                src.fill(0);
                cimg_forXY(src, x, y)
                {
                    src(x, y) = y;
                }
                Img dst(201, 201);
                dst.fill(0);
                int x[3] = { 0, 200, 0 };
                int y[3] = { 200, 200, 0 };
                int dx[3] = { 0, 200, 0 };
                int dy[3] = { 100, 100, 0 };
                for (int i = 0; i < 3; i++)
                    cin >> x[i];
                for (int i = 0; i < 3; i++)
                    cin >> y[i];
                for (int i = 0; i < 3; i++)
                    cin >> dx[i];
                for (int i = 0; i < 3; i++)
                    cin >> dy[i];
                A4Warpping::affine(src, dst, x, y, dx, dy);
                src.display();
                dst.display();
            } else {
                cached_edge = canny(s.c_str(), 2.5, 7.5, 4, 16, 0);
                if (showEdge)
                    cached_edge.display("edge detection", false);
                cout << "edge detect complete...\n";
                Img src(s.c_str());
                a4(src, cached_edge).display("A4 edges and points", false);
                cached_points = a4.getPoints();
                int x[4], y[4];
                for (int i = 0; i < 4; i++) {
                    x[i] = cached_points[i].x;
                    y[i] = cached_points[i].y;
                }
                cached_A4 = a4Warpping(src, x, y, true);
                cached_A4.display("A4", false);
                cached_A4.save("result"POSTFIX);
                cout << "result saved.\n";
            }
        } catch (exception &e) {
            cout << "Exception: " << e.what() << endl;
        }
    }
    return 0;
}
