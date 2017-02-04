#include "ofApp.h"


PXCSenseManager* senseManager = 0;
PXCHandModule *handModule;
PXCHandData *handData = 0;
PXCHandConfiguration* config;
PXCFaceModule *faceModule;
PXCFaceConfiguration *faceConfig;
PXCFaceData *faceData;
PXCProjection *projection;

const int COLOR_WIDTH = 640;
const int COLOR_HEIGHT = 480;
const int DEPTH_WIDTH = 640;
const int DEPTH_HEIGHT = 480;
const int FPS = 15;
const int capCalibrateX = 50;

ofRectangle camViewport;
ofPixels imagePixels;
ofTexture texture;

int running;

uint32_t frameNum;
uint32_t framePlayback;
ofVec2f rightHandAt;
ofVec2f leftHandAt;
ofVec2f leftPointerAt;
ofVec2f rightPointerAt;
ofImage leftHandImage, rightHandImage, backImage, headImage;
ofImage leftHandImage_ld, rightHandImage_ld, backImage_ld, headImage_ld;
bool rightHandFound, leftHandFound;
bool leftHandImageflag, rightHandImageflag, backImageflag, headImageflag;
bool leftPointerFound;
bool rightPointerFound;

ofVec2f leftEyeAt, rightEyeAt, mouseAt;
bool faceFound;

int isrightAssign2Tip;
int isleftAssign2Tip;
int flocation = 0;

bool isColorPointsEnable;

int rightHandImageSize;
int leftHandImageSize;
int backImageSize;
int headImageSize;
ofVec2f backImageAt;
int backImageOffsetX, backImageOffsetY, rightHandImageOffsetX, rightHandImageOffsetY,
leftHandImageOffsetX, leftHandImageOffsetY, faceImageOffsetX, faceImageOffsetY;
//TODO ここらへんの画像系変数をクラスにまとめ、left,right,back,faceの4いんすたんすで管理出来るようにする。


bool windowcloserflag;

bool captureWindowFlag;
bool startMenuFlag;
bool projectControlFlag; //EDIT 1
bool playbackControlFlag; //EDIT 2
bool assetViewFlag; //EDIT 3
bool assetEditFlag; //EDIT 4


ofFileDialogResult result;

string path;
string dir;
wstring widepath;
const wchar_t* path_wchar_t;
string recdir;
bool writeOut;

bool Pause;

ImGuiWindowFlags window_flags = 0;


//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(180);
	ofSetWindowShape(1980, 1080);
	ofSetFrameRate(FPS);
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1; 
	font_config.PixelSnapH = 1;
	io.Fonts->AddFontFromFileTTF("data\\YuGothM001.TTF", 24.0f, &font_config, io.Fonts->GetGlyphRangesJapanese() );
	io.ImeWindowHandle = ofGetWin32Window();
	gui.setup();

	static bool no_titlebar = false;
	static bool no_border = true;
	static bool no_resize = true;
	static bool no_move = true;
	static bool no_scrollbar = false;
	static bool no_collapse = true;
	static bool no_menu = true;
	if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	dP[0].name = u8"ひだりてのひら";
	dP[1].name = u8"ひだりて ひとさしゆび";
	dP[2].name = u8"みぎてのひら";
	dP[3].name = u8"みぎて ひとさしゆび";
	dP[4].name = u8"ひだりめ";
	dP[5].name = u8"みぎめ";
	dP[6].name = u8"くち";
	dP[7].name = u8"はいけい";

	for (int i = 0; i < dPsize; i++) {
		dP[i].en = false;
	}

	for (int i = 0; i < lunitsize; i++) {
		lunit[i].size = 100;
		lunit[i].en = false;
		lunit[i].to = 0;
		lunit[i].image.clear();
		lunit[i].fileimg.clear();
		lunit[i].offset.x = 0;
		lunit[i].offset.y = 0;
	}


	camViewport.set(60, 60, 1280, 960);

	writeOut = false;
	mainlayer.allocate(1280, 960, GL_RGBA);
	//windowcloserflag = false;
	VC_State = ENTRY;
	isColorPointsEnable = true;
	//startMenuFlag = true;
	initializeRSSDK();
}

void ofApp::initializeRSSDK() {
	cout << "Initialize" << endl;
	senseManager = PXCSenseManager::CreateInstance();
	if(senseManager == 0)cout << "CreateInstance err" << endl;

	if (VC_State == RECORD)senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, true);
	if (VC_State == EDIT)senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, false);

	auto sts = senseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR);
	if(sts<PXC_STATUS_NO_ERROR) cout << "EnableColorStream err" << endl;

	sts = senseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH);
	if (sts<PXC_STATUS_NO_ERROR) cout << "EnableDepthStream err" << endl;

	senseManager->QueryCaptureManager()->SetRealtime(false);
	senseManager->QueryCaptureManager()->SetPause(true);

	senseManager->EnableHand();
	senseManager->EnableFace();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	faceModule = senseManager->QueryFace();
	faceData = faceModule->CreateOutput();

	sts = senseManager->Init();
	if (sts<PXC_STATUS_NO_ERROR) cout << "INIT err" << endl;



	auto device = senseManager->QueryCaptureManager()->QueryDevice();

	projection = device->CreateProjection();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();


	faceConfig = faceModule->CreateActiveConfiguration();
	faceConfig->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

	frameNum = 0;
	framePlayback = 0;
}

string IntToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void ofApp::initializeLoadedValue() {
	cout << dir << endl;
	ifstream ifs(dir + "offsets.csv");

	if (!ifs) {
		cout << "input err" <<endl;
	}
	else {
		//csvファイルを1行ずつ読み込む
		string str;
		string token;
		istringstream stream;
		//istringstream stream;
		for (int j = 0; j < lunitsize; j++) {
			getline(ifs, str);
			stream.str("");
			stream.clear(stringstream::goodbit);
			stream.str(str);
			getline(stream, token, ','); lunit[j].to = stoi(token);
			getline(stream, token, ','); lunit[j].size = stoi(token);
			getline(stream, token, ','); lunit[j].offset.x = stof(token);
			getline(stream, token, ','); lunit[j].offset.y = stof(token);
			//if (lunit[j].en) {
				lunit[j].fileimg.loadImage(dir + IntToString(j) + ".png");
				if (lunit[j].fileimg.isAllocated()) {
					lunit[j].en = true;
				}
			//}

		}
	}
}
//--------------------------------------------------------------
void ofApp::update() {

	ofSetWindowTitle("Visual Creole   Framerate at " + ofToString(ofGetFrameRate(), 2));

	switch (VC_State) {
	case ENTRY:
		updateCamera();
		break;
	case RECORD:
		updateCamera();
		break;
	case EDIT:
		senseManager->QueryCaptureManager()->SetFrameByIndex(framePlayback);
		senseManager->FlushFrame();
		if (!Pause) framePlayback++;
		updateCamera();

		break;
	}

}

void ofApp::updateCamera() { //Liveに必要なもののみ。他に必要なものはほかでやろう
							// This function blocks until a color sample is ready
	if (senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR) {
		// Retrieve the sample
		PXCCapture::Sample *sample;
		//color
		sample = senseManager->QuerySample();
		PXCImage* sampleImage;
		sampleImage = sample->color;
		PXCImage::ImageData sampleData;
		if (sampleImage->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &sampleData) >= PXC_STATUS_NO_ERROR) {
			uint8_t* cBuffer = sampleData.planes[0];
			imagePixels.setFromPixels(cBuffer, 640, 480, 4);
			//printf("%s", sampleData.planes[0]);
			sampleImage->ReleaseAccess(&sampleData);
		}
		//hand
		handData->Update();
		pxcUID handId;
		PXCHandData::IHand* hand;
		PXCHandData::JointData jointData;
		PXCPointF32 colorPoint;
		//left hand at first
		if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_LEFT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
			handData->QueryHandDataById(handId, hand);

			hand->QueryTrackedJoint((PXCHandData::JointType)1, jointData);
			colorPoint = { 0 };
			auto depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			dP[lHandPalm].pos.set(colorPoint.x, colorPoint.y);
			if (dP[lHandPalm].pos.x != -1.0f && dP[lHandPalm].pos.y != -1.0f) {
				dP[lHandPalm].en = true;
			}
			else dP[lHandPalm].en = false;

			hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
			colorPoint = { 0 };
			depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			dP[lHandTip].pos.set(colorPoint.x, colorPoint.y);
			if (dP[lHandTip].pos.x != -1.0f && dP[lHandTip].pos.y != -1.0f) {
				dP[lHandTip].en = true;
			}
			else dP[lHandTip].en = false;
		}
		else {
			dP[lHandPalm].en = false;
			dP[lHandTip].en = false;
		}

		if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_RIGHT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
			handData->QueryHandDataById(handId, hand);

			hand->QueryTrackedJoint((PXCHandData::JointType)1, jointData);
			colorPoint = { 0 };
			auto depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			dP[rHandPalm].pos.set(colorPoint.x, colorPoint.y);
			if (dP[rHandPalm].pos.x != -1.0f && dP[rHandPalm].pos.y != -1.0f) {
				dP[rHandPalm].en = true;
			}
			else dP[rHandPalm].en = false;

			hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
			colorPoint = { 0 };
			depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			dP[rHandTip].pos.set(colorPoint.x, colorPoint.y);
			if (dP[rHandTip].pos.x != -1.0f && dP[rHandTip].pos.y != -1.0f) {
				dP[rHandTip].en = true;
			}
			else dP[rHandTip].en = false;

		}
		else {
			dP[rHandPalm].en = false;
			dP[rHandTip].en = false;
		}

		//FACE
		faceData->Update();
		pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
			PXCFaceData::Face *face = faceData->QueryFaceByIndex(0);
			if (face != nullptr) {
				PXCFaceData::LandmarksData *ldata = face->QueryLandmarks();
				if (ldata != nullptr) {
					pxcI32 npoints = ldata->QueryNumPoints();
					PXCFaceData::LandmarkPoint *points = new PXCFaceData::LandmarkPoint[npoints];
					ldata->QueryPointsByGroup(
						PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_LEFT_EYE, points);
					//leftEyeAt.set(points[6].image.x, points[6].image.y);
					dP[lEye].pos.set(points[6].image.x, points[6].image.y);
					dP[lEye].en = true;

					ldata->QueryPointsByGroup(
						PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_RIGHT_EYE, points);
					//rightEyeAt.set(points[6].image.x, points[6].image.y);
					dP[rEye].pos.set(points[6].image.x, points[6].image.y);
					dP[rEye].en = true;

					ldata->QueryPointsByGroup(
						PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_MOUTH, points);
					//mouseAt.set(points[3].image.x, points[3].image.y);
					dP[mouse].pos.set(points[3].image.x, points[3].image.y);
					dP[mouse].en = true;

					//faceFound = true;
				}
				else {
					//faceFound = false;
					dP[lEye].en = false;
					dP[rEye].en = false;
					dP[mouse].en = false;
				}
			}
			else {
				//faceFound = false;
				dP[lEye].en = false;
				dP[rEye].en = false;
				dP[mouse].en = false;
			}
		senseManager->ReleaseFrame();
	}
	else {
		printf("Aquireframe error\n");
		Pause = true;
		writeOut = false;
		framePlayback--;
		senseManager->QueryCaptureManager()->SetPause(Pause);
		ofSetFrameRate(15);
	}
	texture.loadData(imagePixels.getPixels(), 640, 480, GL_BGRA);
}

//--------------------------------------------------------------
void ofApp::draw() {
	gui.begin();

	switch (VC_State) {

	case ENTRY:

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin(u8"メニュー", 0, window_flags);


		//todo 新規プロジェクト生成時とロード時で、フォルダ名を指定するのとファイルを指定するので扱いが異なってしまう。
		if (ImGui::Button(u8"あたらしくどうがをつくる"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			result = ofSystemSaveDialog("Project", "Input Projectname");
			if (result.bSuccess) {
				path = result.getPath();
				Pause = true;
				int len = path.length();
				char* dname = new char[len + 1];
				memcpy(dname, path.c_str(), len + 1);
				_mkdir(dname);
				dir = path + "\\";
				path.append("\\record.rssdk");
				cout << path << endl;
				for (int i = 0; i < path.length(); ++i) {
					widepath += wchar_t(path[i]);
				}
				path_wchar_t = widepath.c_str();
				wprintf(L"path:%s", path_wchar_t);
				VC_State = RECORD;
				initializeRSSDK();


			}
			else {
				initializeRSSDK();
			}
		}
		if (ImGui::Button(u8"どうがをよみこむ"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			result = ofSystemLoadDialog(u8"Select .rssdk");
			if (result.bSuccess) {

				path = result.getPath();
				dir = path;
				dir.erase(dir.length() - 12);
				//printf("%s\n",dir);
				cout << dir << endl;
				for (int i = 0; i < path.length(); ++i)
					widepath += wchar_t(path[i]);
				path_wchar_t = (wchar_t*)widepath.c_str();
				Pause = true;
				VC_State = EDIT;
				initializeRSSDK();

				initializeLoadedValue();

			}
			else {
				initializeRSSDK();
			}
		}
		ImGui::End();

		//drawPoints();
		break;

	case RECORD:
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin(u8"ろくがメニュー", 0, window_flags);

		if (ImGui::Button(u8"スタート"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		if (ImGui::Button(u8"ストップ"))
		{
			Pause = true;
			//senseManager->QueryCaptureManager()->SetPause(Pause);
			VC_State = EDIT;
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			initializeRSSDK();
		}
		ImGui::Text(u8"ろくがフレーム"); ImGui::SameLine();
		ImGui::Text(": %d", frameNum);
		ImGui::Text(u8"ろくがじかん"); ImGui::SameLine();
		ImGui::Text(": %d", frameNum / FPS);
		ImGui::Checkbox(u8"めじるしをだす", &isColorPointsEnable);
		ImGui::End();
		drawPicCtrl();

		/*
		if (isColorPointsEnable) {
			drawPoints();
		}
		drawImages();
		*/
		break;

	case EDIT:
		//drawPoints();
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin(u8"さいせいメニュー", 0, window_flags);

		if (ImGui::Button(u8"とりなおす"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			VC_State = RECORD;
			initializeRSSDK();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"ほぞん"))
		{
			ofstream ofs(dir + "offsets.csv");
			for (int i = 0; i < lunitsize; i++) {
				if (lunit[i].fileimg.isAllocated()) {
					lunit[i].fileimg.saveImage(dir + IntToString(i) + ".png", OF_IMAGE_QUALITY_HIGH);
				}
				ofs << lunit[i].to << ',' << lunit[i].size << ','
					<< (int)lunit[i].offset.x << ',' << (int)lunit[i].offset.y << ',' << endl;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"かきだす"))
		{
		//	ImGui::OpenPopup("WriteOut");

			recdir = dir + "rec";
			int len = recdir.length();
			char* dname = new char[len + 1];
			memcpy(dname, recdir.c_str(), len + 1);
			_mkdir(dname);

			ofSetFrameRate(30);
			framePlayback = 0;
			writeOut = true;
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		/*if (ImGui::BeginPopupModal("WriteOut", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::Button(u8"とりやめる", ImVec2(120, 0))) {
				writeOut = false;
			}
			if (!writeOut) {
				ofSetFrameRate(15);
				framePlayback = 0;
				Pause = true;
				senseManager->QueryCaptureManager()->SetPause(Pause);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}*/
		ImGui::SameLine();
		if (ImGui::Button(u8"さいしょにもどる"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			for (int i = 0; i < lunitsize; i++) {
				lunit[i].size = 100;
				lunit[i].en = false;
				lunit[i].to = 0;
				lunit[i].image.clear();
				lunit[i].fileimg.clear();
				lunit[i].offset.x = 0;
				lunit[i].offset.y = 0;
			}
			VC_State = ENTRY;
			isColorPointsEnable = true;
			initializeRSSDK();
		}

		ImGui::NewLine();
		if (ImGui::Button(u8"スタート"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"ストップ"))
		{
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"まきもどす"))
		{
			framePlayback = 0;
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}



		ImGui::Text(u8"<-もどる"); ImGui::SameLine();
		if (ImGui::Button("-3s"))
		{
			framePlayback -= 90;
		}
		ImGui::SameLine();
		if (ImGui::Button("-1s"))
		{
			framePlayback -= 30;
		}
		ImGui::SameLine();
		if (ImGui::Button("+1s"))
		{
			framePlayback += 30;
		}
		ImGui::SameLine();
		if (ImGui::Button("+3s"))
		{
			framePlayback += 90;
		}
		ImGui::SameLine();
		ImGui::Text(u8"すすむ->");

		ImGui::Text(u8"フレーム"); ImGui::SameLine();
		ImGui::Text(": %d", framePlayback); ImGui::SameLine();
		ImGui::Text(u8"じかん"); ImGui::SameLine();
		ImGui::Text(": %d", framePlayback / FPS); ImGui::SameLine();
		ImGui::Checkbox(u8"めじるしをだす", &isColorPointsEnable);

		ImGui::End();
		drawPicCtrl();

		break;

	}
	gui.end();

	mainlayer.begin();

	//texture.draw(camViewport);
	texture.draw(0, 0, 1280, 960);
	if (isColorPointsEnable) {
		drawPoints();
	}
	drawImages();
	mainlayer.end();

	//if (!writeOut) {
		mainlayer.draw(camViewport);
	//}
	if(writeOut){
		ofPixels wpixels;
		ofImage wimage;
		mainlayer.readToPixels(wpixels);
		wimage.setFromPixels(wpixels);
		wimage.saveImage(recdir + "\\" + IntToString(framePlayback) + ".bmp");
	}
}

int e = 0;
void ofApp::drawPicCtrl() {

	ImGui::SetNextWindowSize(ofVec2f(500, 300), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 280), ImGuiSetCond_Always);
	ImGui::Begin("Picture Control", 0,window_flags);
	//ImGui::RadioButton("")

	ImGui::RadioButton(u8"がぞう1", &e, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"がぞう2", &e, 1); ImGui::SameLine();
	ImGui::RadioButton(u8"がぞう3", &e, 2); ImGui::SameLine();
	ImGui::RadioButton(u8"がぞう4", &e, 3);
	ImGui::NewLine();

	if (ImGui::Button(u8"よみこむ"))
	{
		result = ofSystemLoadDialog(u8"よみこむがぞうをえらんでね");
		if (result.bSuccess) {
			path = result.getPath();
			lunit[e].image.loadImage(path);
			lunit[e].image.clear();
			lunit[e].en = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox(u8"オン", &lunit[e].en);
	ImGui::SameLine();
	if (ImGui::Button(u8"いちあわせをやりなおす"))
	{
		lunit[e].to = 0;
		lunit[e].size = 100;
		lunit[e].offset.x = 0;
		lunit[e].offset.y = 0;
	}

	if (ImGui::Button(u8"ばしょ"))
		ImGui::OpenPopup("select");
	ImGui::SameLine();
	ImGui::Text(dP[lunit[e].to].name);

	if (ImGui::BeginPopup("select"))
	{
		if (ImGui::BeginMenu(u8"ひだりて"))
		{
			if (ImGui::Selectable(u8"てのひら"))lunit[e].to = lHandPalm;
			if (ImGui::Selectable(u8"ひとさしゆび"))lunit[e].to = lHandTip;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(u8"みぎて"))
		{
			if (ImGui::Selectable(u8"てのひら"))lunit[e].to = rHandPalm;
			if (ImGui::Selectable(u8"ひとさしゆび"))lunit[e].to = rHandTip;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(u8"かお"))
		{
			if (ImGui::Selectable(u8"ひだりめ"))lunit[e].to = lEye;
			if (ImGui::Selectable(u8"みぎめ"))lunit[e].to = rEye;
			if (ImGui::Selectable(u8"くち"))lunit[e].to = mouse;
			ImGui::EndMenu();
		}
		if (ImGui::Selectable(u8"はいけい"))lunit[e].to = bg;

		ImGui::EndPopup();
	}

	ImGui::SliderInt(u8"おおきさ", &lunit[e].size, 10, 500, nullptr);
	ImGui::SliderFloat(u8"位置あわせ よこ", &lunit[e].offset.x, -640, 640, nullptr);
	ImGui::SliderFloat(u8"位置あわせ たて", &lunit[e].offset.y, -480, 480, nullptr);

	ImGui::End();
}

void ofApp::drawPoints() {
	ofPushStyle();
	for (int i = 0; i < dPsize; i++) {
		if (dP[i].en) {
			switch (i) {
			case 0: case 1:
				ofSetColor(255, 0, 0);
				break;
			case 2: case 3:
				ofSetColor(0, 0, 255);
				break;
			case 4: case 5: case 6:
				ofSetColor(0, 255, 0);
				break;
			}

			ofFill();
			ofRect(dP[i].pos.x * 2 /*+ camViewport.x*/, dP[i].pos.y * 2 /* + camViewport.y*/, 20, 20);
		}
	}

	ofPopStyle();
}

void ofApp::drawImages() {

	for (int i = 0; i < lunitsize; i++) {
		if (lunit[i].en && dP[lunit[i].to].en) {
			if(!lunit[i].image.isAllocated())lunit[i].image.clone(lunit[i].fileimg);
			if (lunit[i].size != lunit[i].size_pre) {
				lunit[i].image.clone(lunit[i].fileimg);
				lunit[i].image.resize(lunit[i].fileimg.getWidth()*lunit[i].size / 100, lunit[i].fileimg.getHeight()*lunit[i].size / 100);
				lunit[i].size_pre = lunit[i].size;
			}
			lunit[i].image.draw(
				dP[lunit[i].to].pos.x * 2 - lunit[i].image.getWidth() / 2 + (int)lunit[i].offset.x,
				dP[lunit[i].to].pos.y * 2 - lunit[i].image.getHeight() / 2 + (int)lunit[i].offset.y
			);
		}
	}
	
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	//Pause = !Pause;
	//senseManager->QueryCaptureManager()->SetPause(Pause);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (VC_State == RECORD || VC_State == EDIT) {
		lunit[e].fileimg.load(dragInfo.files[0]);
		lunit[e].image.clear();
		lunit[e].en = true;
	}
}
