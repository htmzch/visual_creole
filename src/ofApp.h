#pragma once

#include "ofMain.h"

#include <wchar.h>
#include <direct.h>

#include<string>
#include<iostream> 
#include<fstream> 
#include<sstream>

#include "pxccapture.h"
#include "pxcsensemanager.h"
#include "pxccapturemanager.h"
#include "pxchandcursormodule.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"
#include "pxcfacemodule.h"
#include "pxcfaceconfiguration.h"
#include "pxcprojection.h"

#include "ofxImGui.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		//Visual Creole全体のState。起動直後のプロジェクト選択、録画、編集の３つ
		enum _VC_State {
			ENTRY, //起動直後はこれ。
			RECORD,
			EDIT
		} ;

		_VC_State VC_State;

		//録画画面でのステート
		enum R_State {
			WAITING, //
			READY,	//
			RECORDING,//録画中、この時には画像の描画をしないでいるとよい？
			RECORDED //録画終了時。すぐ編集画面に遷移する
		} ;
		/*

		/*
		points 
		体のパーツそれぞれの情報（左手のひら、右手人差し指先、etc）
		dP[n]として宣言。
		便宜上背景の分も変数を確保しておく（enableは常にtrue）
		呼び方は dP[lEye].pos.x など（左目のx座標を取得）
		取得点を増やすときは、#defineで対応する呼称を定義すること。
		*/

		typedef struct _point {
			bool en;//手とかが検知されて、値を更新可能であったときにtrue
			char* name;//名前を入れる。ofapp.cppのsetupで行っているはず
			ofVec2f pos;//座標
		} point;
	
#define dPsize 8

		point dP[dPsize];

	#define lHandPalm	0
	#define lHandTip	1
	#define rHandPalm	2
	#define rHandTip	3
	#define lEye		4
	#define rEye		5
	#define mouse		6
	#define bg			7

		typedef struct _lenderunit {
			bool en;
			int to;
			ofFbo layer; //ドローイング内容を保存しておく。変更発生時のみ、このレイヤを画像化する。
			ofImage image; //layerを画像化したもの、これを各ドローポイント（指先etc）に追従させ表示する。
			ofImage fileimg; //外部から読み込んだ画像ファイル
			ofVec2f offset;
			int size; //%基準。layerおよびfileimgの
			int size_pre; //サイズ変更を検知するため、1フレ前の大きさも入れておきます
			//for future work
			int in, out; //描画有効、無効となるフレーム
		} lenderunit;

#define lunitsize 4

		lenderunit lunit[lunitsize]; //ひとまず左右の手、顔、背景に使えるように4つ確保

		ofFbo mainlayer; //最終的にこのレイヤにすべてのlenderunit.imageをまとめて書き込む。

		void initializeRSSDK();
		void initializeLoadedValue();

		void updateCamera();

		void drawPoints();
		void drawImages();
		void drawPicCtrl();

		ofxImGui::Gui gui;
};
