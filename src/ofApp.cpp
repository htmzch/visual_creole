#include "ofApp.h"


PXCSenseManager* senseManager = 0;
PXCHandModule *handModule;
PXCHandData *handData = 0;
PXCHandConfiguration* config;

const int COLOR_WIDTH = 640;
const int COLOR_HEIGHT = 480;
const int DEPTH_WIDTH = 640;
const int DEPTH_HEIGHT = 480;
const int FPS = 30;
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
ofImage leftHandImage;
ofImage rightHandImage;
ofImage backImage;
ofImage leftHandImage_ld;
ofImage rightHandImage_ld;
ofImage backImage_ld;
bool rightHandFound;
bool leftHandFound;
bool leftHandImageFrag;
bool rightHandImageFrag;
bool backImageFrag;
bool leftPointerFound;
bool rightPointerFound;

bool isrightAssign2Tip;
bool isleftAssign2Tip;

bool isColorPointsEnable;

int rightHandImageSize;
int leftHandImageSize;
int backImageSize;
ofVec2f backImageAt;
int bkx, bky;

bool captureWindowFlag;
bool startMenuFlag;
bool projectControlFlag; //EDIT 1
bool playbackControlFlag; //EDIT 2
bool assetViewFlag; //EDIT 3
bool assetEditFlag; //EDIT 4


ofFileDialogResult result;

string path;
wstring widepath;
const wchar_t* path_wchar_t;

bool Pause;

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(180);
	ofSetWindowShape(1980, 1080);
	ofSetFrameRate(FPS);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("data\\YuGothM001.TTF", 24.0f, nullptr,nullptr);

	gui.setup();

	camViewport.set(60, 60, 1280, 960);

	VC_State = ENTRY;
	startMenuFlag = true;
	initializeLive();

}

void ofApp::initializeLive() {
	senseManager = PXCSenseManager::CreateInstance();
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, COLOR_WIDTH, COLOR_HEIGHT, 0);
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT, 0);

	senseManager->EnableHand();

	senseManager->Init();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();
}

void ofApp::initializeCapture() {
	
	senseManager = PXCSenseManager::CreateInstance();

	senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, true);
	// load your file at `path`
	senseManager->QueryCaptureManager()->SetRealtime(true);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	senseManager->EnableHand();

	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, COLOR_WIDTH,COLOR_HEIGHT, 30);
	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, DEPTH_WIDTH,DEPTH_HEIGHT, 30);

	senseManager->Init();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

}

void ofApp::initializePlayer() {
	senseManager = PXCSenseManager::CreateInstance();
	if (senseManager == 0) {
		throw std::runtime_error("SenseManagerの生成に失敗しました");
	}
	wprintf(L"path:%s", path_wchar_t);
	senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, false);
	// load your file at `path`


	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, 640, 480,30);
	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, 640,480, 30);

	senseManager->EnableHand();

	senseManager->Init();

	senseManager->QueryCaptureManager()->SetRealtime(false);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

	framePlayback = 0;
}


//--------------------------------------------------------------
void ofApp::update(){

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
		//if (VC_State != RECORD) {
			//hand
			handData->Update();
			pxcUID handId;
			PXCHandData::IHand* hand;
			PXCHandData::JointData jointData;
			//left hand at first
			if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_LEFT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
				handData->QueryHandDataById(handId, hand);
				auto center_l = hand->QueryMassCenterImage();
				leftHandAt.set(center_l.x, center_l.y);
				leftHandFound = true;
				hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
				leftPointerFound = true;
				leftPointerAt.set(jointData.positionImage.x, jointData.positionImage.y);
			}
			else { leftHandFound = false; 
			leftPointerFound = false;
			}

			if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_RIGHT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
				handData->QueryHandDataById(handId, hand);
				auto center_r = hand->QueryMassCenterImage();
				rightHandAt.set(center_r.x, center_r.y);
				rightHandFound = true;
				rightPointerFound = true;
				hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
				rightPointerAt.set(jointData.positionImage.x, jointData.positionImage.y);
			}
			else { rightHandFound = false; 
			rightPointerFound = false;
			}

		//}
		senseManager->ReleaseFrame();
	}
	else { printf("Aquireframe error\n"); 
	Pause = true;
	senseManager->QueryCaptureManager()->SetPause(Pause);
	}
	texture.loadData(imagePixels.getPixels(), 640, 480, GL_BGRA);
}

//--------------------------------------------------------------
void ofApp::draw(){
	gui.begin();
	texture.draw(camViewport);
	

	switch (VC_State) {

	case ENTRY:
		drawPoints();
		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Start Menu", &startMenuFlag);
	

		//todo 新規プロジェクト生成時とロード時で、フォルダ名を指定するのとファイルを指定するので扱いが異なってしまう。
		if (ImGui::Button("Create New Project"))
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
				path.append("\\record.rssdk");
				cout << path << endl;
				for (int i = 0; i < path.length(); ++i) {
					widepath += wchar_t(path[i]);
				}
				path_wchar_t = widepath.c_str();
				wprintf(L"path:%s", path_wchar_t);
				initializeCapture();

				VC_State = RECORD;
			}
			else {
				initializeLive();
			}
		}
		if (ImGui::Button("Load and Play/Edit Project"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			result = ofSystemLoadDialog("Select Project.rssdk");
			if (result.bSuccess) {

				path = result.getPath();
				for (int i = 0; i < path.length(); ++i)
					widepath += wchar_t(path[i]);
				path_wchar_t = (wchar_t*)widepath.c_str();
				Pause = true;
				initializePlayer();
				VC_State = EDIT;
			}
			else {
				initializeLive();
			}
		}
		ImGui::End();
		break;

	case RECORD:
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Record Manager", &startMenuFlag);
		
		if (ImGui::Button("Start Recording"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		if (ImGui::Button("Stop Recording"))
		{
			Pause = true;
			//senseManager->QueryCaptureManager()->SetPause(Pause);
			VC_State = EDIT;
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			initializePlayer();
		}
		ImGui::Text("Recording Frame Count : %d", frameNum);
		ImGui::Text("Recording Time Duration : %d", frameNum / 30);
		ImGui::Checkbox("PointsEnable", &isColorPointsEnable);
		if (isColorPointsEnable) {
			drawPoints();
		}
		ImGui::End();
		drawPicCtrl();
		drawImages();
		break;

	case EDIT:
		//drawPoints();
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Playback Manager", &startMenuFlag);
	
		if (ImGui::Button("Start"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
		{
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{	
			framePlayback = 0;
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();

		if (ImGui::Button("Save Project"))
		{
			std::string pjdir;
			pjdir.copy();
			//leftHandImage_ld.saveImage()
		}

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

		ImGui::Text("Playback Frame Count : %d", framePlayback);
		ImGui::Text("Playback Time Duration : %d", framePlayback / 30);
		ImGui::Checkbox("PointsEnable", &isColorPointsEnable);
		if (isColorPointsEnable) {
			drawPoints();
		}
		ImGui::End();
		drawPicCtrl();
		drawImages();
		break;

	}
	gui.end();
}

void ofApp::drawPicCtrl() {

	ImGui::SetNextWindowSize(ofVec2f(500, 110), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 280), ImGuiSetCond_Always);
	ImGui::Begin("Left Hand Control", &startMenuFlag);

	if (ImGui::Button("Assign Picture"))
	{
		result = ofSystemLoadDialog("Load Lefthand Image");
		if (result.bSuccess) {
			path = result.getPath();
			leftHandImage_ld.loadImage(path);
			//leftHandImage.resize(leftHandImage.getWidth() / 2, leftHandImage.getHeight() / 2);
			leftHandImageFrag = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Enable", &leftHandImageFrag);
	ImGui::SameLine();
	ImGui::Checkbox("at Tip", &isleftAssign2Tip);
	ImGui::SliderInt("Size", &leftHandImageSize, 100, 600,nullptr);
	ImGui::End();
	ImGui::SetNextWindowSize(ofVec2f(500, 110), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 420), ImGuiSetCond_Always);
	ImGui::Begin("Right Hand Control", &startMenuFlag);

	if (ImGui::Button("Assign Picture"))
	{
		result = ofSystemLoadDialog("Load Righthand Image");
		if (result.bSuccess) {
			path = result.getPath();
			rightHandImage_ld.loadImage(path);
			//rightHandImage.resize(rightHandImage.getWidth() / 2, rightHandImage.getHeight() / 2);
			rightHandImageFrag = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Enable", &rightHandImageFrag);
	ImGui::SameLine();
	ImGui::Checkbox("at Tip", &isrightAssign2Tip);
	ImGui::SliderInt("Size", &rightHandImageSize, 100, 600,nullptr);
	ImGui::End();
	/*
	ImGui::SetNextWindowSize(ofVec2f(500, 160), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 560), ImGuiSetCond_Always);
	ImGui::Begin("Head Control", &startMenuFlag);

	if (ImGui::Button("Assign Picture"))
	{
		result = ofSystemLoadDialog("Load Head Image");
		if (result.bSuccess) {
			path = result.getPath();
			headImage_ld.loadImage(path);
			//rightHandImage.resize(rightHandImage.getWidth() / 2, rightHandImage.getHeight() / 2);
			headImageFrag = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Enable", &headImageFrag);
	ImGui::SameLine();
	//ImGui::Checkbox("at Tip", &isrightAssign2Tip);
	ImGui::SliderInt("Size", &rightHandImageSize, 100, 600, nullptr);
	ImGui::End();
*/
	ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 740), ImGuiSetCond_Always);
	ImGui::Begin("Background Control", &startMenuFlag);

	if (ImGui::Button("Assign Picture"))
	{
		result = ofSystemLoadDialog("Load BackGround Image");
		if (result.bSuccess) {
			path = result.getPath();
			backImage_ld.loadImage(path);
			//backImage.resize(backImage.getWidth() / 2, backImage.getHeight() / 2);
			backImageFrag = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Enable", &backImageFrag);
	ImGui::SliderInt("Size", &backImageSize, 100, 500,nullptr);
	ImGui::SliderInt("X offset", &bkx, 0, 640, nullptr);
	ImGui::SliderInt("Y offset", &bky, 0, 480, nullptr);
	backImageAt.set(bkx, bky);
	ImGui::End();
}

void ofApp::drawPoints() {
	ofPushStyle();
	if (leftHandFound) {
		ofSetColor(255, 0, 0);
		ofFill();
		ofRect(leftHandAt.x * 2 + camViewport.x , leftHandAt.y * 2 + camViewport.y, 30, 30);
		ofSetColor(255, 100, 100);
		ofFill();
		ofRect(leftPointerAt.x * 2 + camViewport.x , leftPointerAt.y * 2 + camViewport.y, 10 ,10);
	}
	if (rightHandFound) {
		ofSetColor(0, 0, 255);
		ofFill();
		ofRect(rightHandAt.x * 2 + camViewport.x , rightHandAt.y * 2 + camViewport.y, 30, 30);
		ofSetColor(100, 100, 255);
		ofFill();
		ofRect(rightPointerAt.x * 2 + camViewport.x , rightPointerAt.y * 2 + camViewport.y, 10, 10);
	}
	ofPopStyle();
}

void ofApp::drawImages() {

	if (leftHandFound && leftHandImageFrag) {
		leftHandImage.clone(leftHandImage_ld);
		leftHandImage.resize(leftHandImageSize, leftHandImage_ld.getHeight()*leftHandImageSize / leftHandImage_ld.getWidth());
		if (isleftAssign2Tip) {
			leftHandImage.draw(leftPointerAt.x * 2 + 40 - leftHandImage.getWidth() / 2, leftPointerAt.y * 2 + 60 - leftHandImage.getHeight() / 2);
		}
		else {
			leftHandImage.draw(leftHandAt.x * 2 + 40 - leftHandImage.getWidth() / 2, leftHandAt.y * 2 + 60 - leftHandImage.getHeight() / 2);
		}
		}
	if (rightHandFound && rightHandImageFrag) {
		rightHandImage.clone(rightHandImage_ld);
		rightHandImage.resize(rightHandImageSize, rightHandImage_ld.getHeight()*rightHandImageSize / rightHandImage_ld.getWidth());
		if (isrightAssign2Tip) {
			rightHandImage.draw(rightPointerAt.x * 2 + 60 - rightHandImage.getWidth() / 2, rightPointerAt.y * 2 + 60 - rightHandImage.getHeight() / 2);
		}
		else {
			rightHandImage.draw(rightHandAt.x * 2 + 60 - rightHandImage.getWidth() / 2, rightHandAt.y * 2 + 60 - rightHandImage.getHeight() / 2);
		}
	}
	if (backImageFrag) {
		backImage.clone(backImage_ld);
		backImage.resize(backImageSize, backImage_ld.getHeight()*backImageSize / backImage_ld.getWidth());
		backImage.draw(backImageAt.x * 2 + 60 - backImage.getWidth() / 2, backImageAt.y * 2 + 60 - backImage.getHeight() / 2);
	}
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	Pause = !Pause;
	//senseManager->QueryCaptureManager()->SetPause(Pause);
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

}
