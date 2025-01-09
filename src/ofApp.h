#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxImGui.h"
#include "ofxSvg.h"
#include "poly2svg.h"

class ofApp : public ofBaseApp{

public:
    ofxCv::ContourFinder contour;
    ofxImGui::Gui gui;
    ofVideoPlayer video;
    string video_name;
    vector<ofImage> imgs;
    vector<string> imgs_names;
    ofFbo fbo;
    bool done = false;
    bool black_in_white = true;
    int img_index = 0;
    float min_radius = 0;
    float max_radius = 400;
    float threshold = 127;
    float scale = 1.0;
    float simplify = 0.5;
    ofImage piximg;
    
    int width = 1100;
    int height = 600;
    
    ofxSVG svgs;
    ofPath path;
    
    ofTrueTypeFont font;
    
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    
    void load();
    void save();
    
    void readFiles(const vector<string>& files);

    void keyPressed(int key) override;
    void keyReleased(int key) override;
    void mouseMoved(int x, int y ) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void mouseEntered(int x, int y) override;
    void mouseExited(int x, int y) override;
    void windowResized(int w, int h) override;
    void dragEvent(ofDragInfo dragInfo) override;
    void gotMessage(ofMessage msg) override;
		
};
