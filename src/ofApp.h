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
			char* name;
			ofVec2f pos;
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


		//lenderunit head, lefthand, righthand, back;

		ofFbo mainlayer; //最終的にこのレイヤにすべてのlenderunit.imageをまとめて書き込む。

		/*typedef struct _part {
			//int startframe; //描画が開始されるフレーム番号
			ofImage loadedImage;
			ofFbo layer; //外部から読み込んだ画像、内部で描いた画像の両方を収容する。
			
			
			int startframe;
			int endframe; //描画が停止されるフレーム番号、v0.1では開始30フレーム（1秒)後
			int offset_x; //画像のpx単位のオフセット、自分で描いた場合はあまり適用しないかも、外部のpngを読んだときに利用する。
			int offset_y;
			int size_percent; //画像の大きさを％表示。外部から読み込んだpngが大きかったり小さかったときに。
			//とりあえず%処理だけで動けるかどうかやってみる。というかv0.1では未実装になるかも。
			//int size_x; //読み込んだ画像のもともとの
			//int size_y;
			//ofPixels picture;
		} part ;
		*/
		//動的配列vectorによる宣言
		/*
		std::vector<part> partsLeft;
		std::vector<part> partsRight;
		std::vector<part> partsBack;
		*/

		void initializeRSSDK();
		//void initializeLive();
		//void initializeCapture();
		//void initializePlayer();
		void initializeLoadedValue();

		void updateCamera();

		void drawPoints();
		void drawImages();
		void drawPicCtrl();

		ofxImGui::Gui gui;
};
