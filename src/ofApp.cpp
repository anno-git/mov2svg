#include "ofApp.h"
#include "svgtiny.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("mov2svg");
    load();
    gui.setup();
    ofSetFrameRate(60);
    fbo.allocate(width, height);
    
    ofTrueTypeFontSettings settings("Honoka-Shin-Antique-Maru_R.otf", 20);
    settings.addRanges(ofAlphabet::Japanese);//日本語
    settings.addRange(ofUnicode::Space);//スペース
    settings.addRange(ofUnicode::IdeographicSpace);//全角スペース
    settings.addRange(ofUnicode::Latin);//アルファベット等
    settings.addRange(ofUnicode::Latin1Supplement);//記号、アクサン付き文字など
    settings.addRange(ofUnicode::NumberForms);//数字？
    settings.addRange(ofUnicode::Arrows);//矢印
    settings.addRange(ofUnicode::MathOperators);//数式記号
    settings.addRange(ofUnicode::Hiragana);//ひらがな
    settings.addRange(ofUnicode::Katakana);//カタカナ
    font.load(settings);
    font.setLetterSpacing(1.2);
}

//--------------------------------------------------------------
void ofApp::update(){
    if(video.isLoaded())
    {
        video.update();
        if(video.isFrameNew())
        {
            contour.setMinAreaRadius(min_radius);
            contour.setMaxAreaRadius(max_radius);
            contour.setThreshold(threshold);
            contour.setFindHoles(true);
            contour.findContours(video);

            if(!done && contour.size() > 0)
            {
                vector<ofPolyline> polys = contour.getPolylines();
                vector<bool> holes;
                for(auto& poly: polys)
                {
                    poly.simplify(simplify);
                }
                for(int i = 0; i < polys.size(); i++)
                {
                    holes.push_back(contour.getHole(i));
                }
                string svg = poly2svg::svgFromPolylines(polys, holes, width, height, scale);
                if(poly2svg::saveSvgToFile("svgs/" + video_name + ".svg", svg))
                {
                    img_index++;
                    video.setFrame(img_index);
                    if(img_index >= video.getTotalNumFrames())
                    {
                        done = true;
                        img_index = video.getTotalNumFrames() - 1;
                    }
                }
            }
        }
    }
    
    if(imgs.size() > 0)
    {
        ofImage& img = imgs[img_index];
        if(img.isAllocated())
        {
            fbo.begin();
            if(black_in_white)
            {
                ofBackground(255);
                img.draw(0, 0);
            }
            else
            {
                ofBackground(0);
                img.draw(0, 0);
            }
            fbo.end();
            ofPixels pixels;
            fbo.readToPixels(pixels);
            if(black_in_white)
            {
                ofxCv::invert(pixels);
            }
            piximg.setFromPixels(pixels);
            contour.setMinAreaRadius(min_radius);
            contour.setMaxAreaRadius(max_radius);
            contour.setThreshold(threshold);
            contour.setFindHoles(true);
            contour.findContours(pixels);
            if(!done && contour.size() > 0)
            {
                vector<ofPolyline> polys = contour.getPolylines();
                for(auto& poly: polys)
                {
                    poly.simplify(simplify);
                }
                vector<bool> holes;
                for(int i = 0; i < polys.size(); i++)
                {
                    holes.push_back(contour.getHole(i));
                }
                string svg = poly2svg::svgFromPolylines(polys, holes, width, height, scale);
                if(poly2svg::saveSvgToFile("svgs/" + imgs_names[img_index] + ".svg", svg))
                {
                    img_index++;
                    if(img_index >= imgs.size())
                    {
                        done = true;
                        img_index = imgs.size() - 1;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushStyle();
//    fbo.draw(0, 0);
    piximg.draw(0, 0);
    contour.draw();
    ofSetColor(255, 0, 0);
    ofNoFill();
    ofDrawRectangle(0, 0, width, height);
    ofSetColor(255);
    font.drawString("ビデオか画像ファイル、あるいは画像連番のフォルダをドラッグアンドドロップしてください", 20, 620);
    font.drawString("drag and drop video or images or folders contains images", 20, 680);
    gui.begin();
    ImGui::Begin("gui");
    if(ImGui::Button("load"))
    {
        load();
    }
    ImGui::SameLine();
    if(ImGui::Button("save"))
    {
        save();
    }
    ImGui::Checkbox("black in white", &black_in_white);
    ImGui::DragFloat("min_radius", &min_radius, 0.1, 0, max_radius);
    ImGui::DragFloat("max_radius", &max_radius, 0.1, min_radius, 1000);
    ImGui::DragFloat("threshold", &threshold, 0.1, 0, 255);
//    ImGui::DragInt("width", &width, 1, 1, 1920);
//    ImGui::DragInt("height", &height, 1, 1, 1080);
    ImGui::DragFloat("scale", &scale, 0.01, 0.01, 4.0);
    ImGui::DragFloat("simplify", &simplify, 0.01, 0.01, 1.0);
    ImGui::End();
    gui.end();
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::exit(){

}
//--------------------------------------------------------------
void ofApp::load(){
    ofJson json = ofLoadJson("settings.json");
    if(!json.empty())
    {
        min_radius = json["min_radius"];
        max_radius = json["max_radius"];
        threshold = json["threshold"];
        scale = json["scale"];
        simplify = json["simplify"];
    }
}
//--------------------------------------------------------------
void ofApp::save(){
    ofJson json;
    json["min_radius"] = min_radius;
    json["max_radius"] = max_radius;
    json["threshold"] = threshold;
    json["scale"] = scale;
    json["simplify"] = simplify;
    ofSavePrettyJson("settings.json", json);
}

//--------------------------------------------------------------
void ofApp::readFiles(const vector<string>& files)
{
    imgs.clear();
    imgs_names.clear();
    video.close();
    video_name = "";
    img_index = 0;
    done = false;
    vector<ofFile> flist;
    for(auto& file: files)
    {
        flist.push_back(ofFile(file));
    }
    sort(flist.begin(), flist.end());
    bool first = true;
    for(auto& f: flist)
    {
        if(f.isDirectory())
        {
            ofDirectory d(f.getAbsolutePath());
            vector<ofFile> d_files = d.getFiles();
            sort(d_files.begin(), d_files.end());
            for(auto& df: d_files)
            {
                if(df.getExtension() == "png")
                {
                    imgs.push_back(ofImage(df.getAbsolutePath()));
                    if(first)
                    {
                        first = false;
                        width = imgs.back().getWidth();
                        height = imgs.back().getHeight();
                        if(fbo.isAllocated())
                        {
                            fbo.clear();
                        }
                        fbo.allocate(width, height);
                        if(max_radius > max(width, height) * 0.95)
                        {
                            max_radius = max(width, height) * 0.95;
                        }
                    }
                    imgs_names.push_back(df.getFileName());
                }
            }
        }
        else if(f.getExtension() == "mp4")
        {
            video.load(f.getAbsolutePath());
            if(first)
            {
                first = false;
                width = video.getWidth();
                height = video.getHeight();
                if(fbo.isAllocated())
                {
                    fbo.clear();
                }
                fbo.allocate(width, height);
                if(max_radius > max(width, height) * 0.95)
                {
                    max_radius = max(width, height) * 0.95;
                }
            }
            video_name = f.getFileName();
        }
        else if(f.getExtension() == "png")
        {
            imgs.push_back(ofImage(f.getAbsolutePath()));
            if(first)
            {
                first = false;
                width = imgs.back().getWidth();
                height = imgs.back().getHeight();
                if(fbo.isAllocated())
                {
                    fbo.clear();
                }
                fbo.allocate(width, height);
                if(max_radius > max(width, height) * 0.95)
                {
                    max_radius = max(width, height) * 0.95;
                }
            }
            imgs_names.push_back(f.getFileName());
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    readFiles(dragInfo.files);
}
