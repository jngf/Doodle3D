#pragma once
#include "ofMain.h"
#include "ofxExtras.h"

class Files {
public:
    
    int cur;
    ofDirectory dir;
    //ofDirectory monitor;
    string doodlesFolder;
    
    Files() {
        cur=-1;
    }
    
    void setup() {
        string folder = ofFilePath::getPathForDirectory("~/Documents/Doodle3D/");
        ofDirectory::createDirectory(folder);
        ofDirectory::createDirectory(folder + "doodles/");
        ofDirectory::createDirectory(folder + "gcode/");
        
        //copy Doodle3D.ini, start.gcode, end.gcode
        //RICK: I fixed the copyTo function in ofFileUtils.cpp
        //ofFile("Doodle3D.ini").copyTo(folder + "Doodle3D.ini"); //overwrite disabled by default, use full path with filename for destination
        ofFile("start.gcode").copyTo(folder + "gcode/start.gcode");
        ofFile("end.gcode").copyTo(folder + "gcode/end.gcode");
        
        //now change the data path to Documents/Doodle3D
        ofSetDataPathRoot(folder);
        
        doodlesFolder = ofToDataPath(ini.get("doodlesFolder","doodles"));
        
        listDir();
        
        //auto load image
        string autoLoadImage = ini.get("autoLoadImage","");
        if (autoLoadImage!="") {
            for (cur=0; cur<dir.numFiles(); cur++) {
                if (dir.getName(cur)==autoLoadImage || dir.getPath(cur)==autoLoadImage) {
                    load(dir.getPath(cur));
                    break;
                }
            }
        }
        
//        doodlesFolder = ini.get("doodlesFolder","doodles");
//        if (doodlesFolder!="") {
//            monitor.reset();
//            monitor.listDir(doodlesFolder);
//            cout << "monitor numFiles: " << monitor.numFiles() << endl;
//        }

    }
        
    void listDir() {
        dir.reset();
        dir.listDir(doodlesFolder);
    }
    
    void loadPrevious() {
        if (dir.numFiles()==0) return;
        cur = (cur-1+dir.numFiles()) % dir.numFiles();
        load(dir.getPath(cur));
    }
    
    void loadNext() {
        if (dir.numFiles()==0) return;
        cur = (cur+1+dir.numFiles()) % dir.numFiles();
        load(dir.getPath(cur));
    }
        
    void load() {
        ofFileDialogResult result = ofSystemLoadDialog();
        if (result.bSuccess) {
            unloadFile();
            return load(result.getPath());
        }
    }
//    
//    void loadFromString(string str) {
//        path.clear();
//        loadFromStrings(ofSplitString(str, "\n")); //or \r
//    }
    
    void loadFromStrings(vector<string> lines) {
        for (int i=0; i<lines.size(); i++) {
            vector<string> coords = ofSplitString(lines[i], " ");
            for (int j=0; j<coords.size(); j++) {
                vector<string> tuple = ofSplitString(coords[j], ",");
                if (tuple.size()!=2) continue; //{ cout << "error in textfile" << endl; return; }
                float x = ofToFloat(tuple[0]);
                float y = ofToFloat(tuple[1]);
                ofPoint p = ofPoint(x,y); // + ofPoint(bounds.x, bounds.y);
                p += ini.get("loadOffset",ofPoint());
                if (j==0) {
                    path.moveTo(p.x,p.y);
                }
                else path.lineTo(p.x,p.y);
            }
        }
        
        //temp
        vector<ofPoint*> points = ofxGetPointsFromPath(path);
        if (points.size()<2) return;
        bool isLoop = points.front()->distance(*points.back())<25;
        //cout << filename << ", loop=" << isLoop << endl;
        ofxSimplifyPath(path); //dit stond uit
    }
    
    void load(string filename) {
        //if (!path) return;
        if (!ofxFileExists(filename)) return; //file not existing or removed   
        path.clear();
        loadFromStrings(ofxLoadStrings(filename));
    }

    void deleteCurrentFile() {
        cout << "delete: " << cur << endl;
        if (cur<0 || cur>=dir.numFiles()-1) return;
        ofFile f(dir.getPath(cur));
        f.remove();
        listDir();
        load(dir.getPath(cur)); //reload the thing on place cur
    }

    void save() {
        if (cur>-1 && cur<dir.numFiles()) {
            save(dir.getName(cur));
        } else {
            saveAs();
        }
    }
    
    void saveAs() {
        //ofSetFullscreen(false);
        listDir();
        string filename = "doodle"+ofToString(dir.numFiles()+1)+".txt";
        ofFileDialogResult result = ofSystemSaveDialog(filename,"Je tekening wordt altijd opgeslagen in de ~/Documents/Doodle3D/doodles/ map.");
        if (result.bSuccess) {
             save(result.getName());   
        }
        //ofSetFullscreen(true);
    }
    
    void save(string filename) {
        vector<string> lines;
        vector<ofSubPath> &subpaths = path.getSubPaths();
        for (int i=0; i<subpaths.size(); i++) {
            string line;
            vector<ofSubPath::Command> &commands = subpaths[i].getCommands();
            for (int j=0; j<commands.size(); j++) {
                ofPoint p = commands[j].to; // - ofPoint(bounds.x, bounds.y);
                p -= ini.get("loadOffset",ofPoint());
                line+=ofToString(p.x) + "," + ofToString(p.y) + " ";
            }
            lines.push_back(line);
        }
        string filePath = doodlesFolder + "/" + filename;
        ofxSaveStrings(filePath,lines); //will only save file in data/doodles/ folder
        listDir();
        cout << "saved: " << filePath << endl;
    }
    
    void unloadFile() {
        cur=-1;
    }
    
    string getFilename() {
        if (cur<0 || cur>=dir.numFiles()) return "noname";
        else {
//            cout << cur << endl;
            return dir.getName(cur);   
        }
    }
};