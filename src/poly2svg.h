#pragma once

#include "ofxSvg.h"
#include "ofConstants.h"
#include <locale>

using std::string;
using std::vector;

extern "C"{
    #include "svgtiny.h"
}

class poly2svg
{

public:
    static const int svgtiny_PATH_NONMASK = 8888;
    static const int svgtiny_PATH_MASK = 8889;
    poly2svg() {};
    virtual ~poly2svg() {};
    // ユーティリティ関数: ファイルにSVGデータを書き込む
    static bool saveSvgToFile(const std::string& filename, const std::string& svg_content) {
        ofBuffer buff;
        buff.set(svg_content);
        ofBufferToFile(filename, buff, false);
        return true;
    }
    
    static string svgFromPolylines(const vector<ofPolyline>& polys, const vector<bool> holes, int width, int height, float scale = 1.0, float strokeWidth = 0.0f, ofColor strokeColor = ofColor::black, ofColor fillColor = ofColor::black)
    {
        struct svgtiny_diagram *diagram = svgtiny_create();
        diagram->width = width * scale;
        diagram->height = height * scale;
        
        diagram->shape = new svgtiny_shape[polys.size()];
        diagram->shape_count = polys.size();
        for(size_t outline = 0; outline < polys.size(); ++outline)
        {
            const auto& polyline = polys[outline];
            bool isHole = holes.size() > outline && holes[outline];
            size_t pathLength = polyline.size() * 7 + 1; // MOVE/LINEまたはBEZIERとCLOSE分

            diagram->shape[outline].path = new float[pathLength];
            diagram->shape[outline].path_length = pathLength;
            size_t idx = 0;
            for (size_t i = 0; i < polyline.size(); i++) {
                if (i == 0) {
                    diagram->shape[outline].path[idx++] = svgtiny_PATH_MOVE;
                    diagram->shape[outline].path[idx++] = polyline.getVertices()[i].x * scale;
                    diagram->shape[outline].path[idx++] = polyline.getVertices()[i].y * scale;
                } else {
                    diagram->shape[outline].path[idx++] = svgtiny_PATH_BEZIER;
                    diagram->shape[outline].path[idx++] = (polyline.getVertices()[i - 1].x * scale + polyline.getVertices()[i].x * scale) / 2;
                    diagram->shape[outline].path[idx++] = (polyline.getVertices()[i - 1].y * scale + polyline.getVertices()[i].y * scale) / 2;
                    diagram->shape[outline].path[idx++] = (polyline.getVertices()[i - 1].x * scale + polyline.getVertices()[i].x * scale) / 2;
                    diagram->shape[outline].path[idx++] = (polyline.getVertices()[i - 1].y * scale + polyline.getVertices()[i].y * scale) / 2;
                    diagram->shape[outline].path[idx++] = polyline.getVertices()[i].x * scale;
                    diagram->shape[outline].path[idx++] = polyline.getVertices()[i].y * scale;
                }
            }

            diagram->shape[outline].path[idx++] = svgtiny_PATH_CLOSE;
            diagram->shape[outline].fill = svgtiny_RGB(fillColor.r, fillColor.g, fillColor.b);
            diagram->shape[outline].stroke = svgtiny_RGB(strokeColor.r, strokeColor.g, strokeColor.b);
            diagram->shape[outline].stroke_width = strokeWidth;
        }
        // SVG出力文字列を生成
        std::string svg_content;
        svg_content += "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" width=\"" + std::to_string(diagram->width) + "\" height=\"" + std::to_string(diagram->height) + "\">\n";
        svg_content += "<defs></defs>\n";
        svg_content += "<g id=\"layer_0\">\n";
        svg_content += "<path fill=\"" + intToHexColor(fillColor.getHex()) +
                       "\" stroke=\"" + intToHexColor(strokeColor.getHex()) + "\" stroke-width=\"" + std::to_string(strokeWidth) + "\" d=\"";
        for (unsigned int i = 0; i < diagram->shape_count; ++i) {
            const svgtiny_shape &shape = diagram->shape[i];
            for (unsigned int j = 0; j < shape.path_length;) {
                int path_type = (int)shape.path[j];
                bool brk = false;
                switch (path_type) {
                    case svgtiny_PATH_MOVE:
                        svg_content += "M " + std::to_string(shape.path[j + 1]) + "," + std::to_string(shape.path[j + 2]) + " ";
                        j += 3;
                        break;
                    case svgtiny_PATH_LINE:
                        svg_content += "L " + std::to_string(shape.path[j + 1]) + "," + std::to_string(shape.path[j + 2]) + " ";
                        j += 3;
                        break;
                    case svgtiny_PATH_BEZIER:
                        svg_content += "C " + std::to_string(shape.path[j + 1]) + "," + std::to_string(shape.path[j + 2]) + " " +
                                       std::to_string((int)shape.path[j + 3]) + "," + std::to_string((int)shape.path[j + 4]) + " " +
                                       std::to_string(shape.path[j + 5]) + "," + std::to_string(shape.path[j + 6]) + " ";
                        j += 7;
                        break;
                    case svgtiny_PATH_CLOSE:
                        svg_content += "Z ";
                        j += 1;
                        brk = true;
                        break;
                    default:
                        j += 1;
                        break;
                }
                if(brk)
                {
                    break;
                }
            }
        }
        svg_content += "\"/>\n";
        svg_content += "</g>\n";
        svg_content += "</svg>\n";
        
//        // メモリ解放
//        for (size_t i = 0; i < diagram->shape_count; ++i) {
//            if(diagram->shape[i].path)
//            {
//                delete[] diagram->shape[i].path;
//                diagram->shape[i].path = nullptr;
//            }
//        }
//        if(diagram->shape)
//        {
//            delete[] diagram->shape;
//            diagram->shape = nullptr;
//        }
        if(diagram)
        {
            svgtiny_free(diagram);
            diagram = nullptr;
        }

        return svg_content;
    }
    
    static std::string intToHexColor(int hexValue) {
        // std::ostringstreamを使って変換
        std::ostringstream oss;
        oss << "#" << std::setfill('0') << std::setw(6) << std::hex << (hexValue & 0xFFFFFF);
        return oss.str();
    }
};
