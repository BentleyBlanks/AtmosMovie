#include "ofApp.h"

//#define TEST

#ifdef TEST
enum a3MaterialType
{
    NONE = 0,
    LAMBERTIAN = 1,
    MIRROR = 2,
    GLASS = 3
};
#endif
//--------------------------------------------------------------
void ofApp::setup(){
#ifdef TEST
    // alloc
    a3Film* image = new a3Film(1024, 768, "data/grid.png");

    for(int x = 0; x < image->width; x++)
        for(int y = 0; y < image->height; y++)
            points.push_back(ofVec2f(x, y));

    imageWidth = image->width;
    imageHeight = image->height;

    previewPixels.allocate(imageWidth, imageHeight, OF_PIXELS_RGB);

    a3PerspectiveSensor* camera = camera = new a3PerspectiveSensor(t3Vector3f(0, 80, 20), t3Vector3f(0, 0, 0), t3Vector3f(0, 0, 1), 40, 100.0f, 0.0f, image);

    a3PathIntegrator* path = new a3PathIntegrator();
    path->russianRouletteDepth = 3;
    path->maxDepth = -1;

    a3BVH* bvh = NULL;
    a3Scene* se = new a3Scene();
    scene = se;
    se->primitiveSet = bvh = new a3BVH();

    renderer = new a3GridRenderer(8);
    renderer->setLevel(10, 10);
    renderer->camera = camera;
    renderer->sampler = new a3RandomSampler();
    renderer->integrator = path;
    renderer->enableGammaCorrection = true;
    renderer->enableToneMapping = false;

    auto addShape = [&se](a3Shape* s, a3Spectrum R, a3Spectrum emission, int type, a3Texture<a3Spectrum>* texture)->auto
    {
        s->emission = emission;

        switch(type)
        {
        case LAMBERTIAN:
            s->bsdf = new a3Diffuse(R);
            break;
        case MIRROR:
            s->bsdf = new a3Conductor(R);
            break;
        case GLASS:
            s->bsdf = new a3Dieletric(R);
            break;
        default:
            break;
        }

        s->bsdf->texture = texture;
        if(texture)
            s->bCalTextureCoordinate = true;

        se->addShape(s);

        return s->bsdf;
    };

    scene->addLight(new a3InfiniteAreaLight("data/images/grace-new_latlong.exr"));

    a3ModelImporter importer;
    std::vector<a3Shape*>* blender = importer.load("data/models/blender.obj");

    for(auto s : *blender)
        addShape(s, t3Vector3f(1.0f), t3Vector3f(0, 0, 0), GLASS, NULL);

    bvh->init();

    renderer->begin();
#else
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./data/DroidSans.ttf", 16.5f, NULL, io.Fonts->GetGlyphRangesChinese());

    gui.setup();

    initImGui();
#endif
}

//--------------------------------------------------------------
void ofApp::update(){
#ifdef TEST
    if(!renderer->isFinished())
    {
        static bool once = false; 
        if(!once)
        {
            timer.start();
            once = true;
        }
        renderer->render(scene);
    }
    else
    {
        static bool once = false;
        if(!once)
        {
            renderer->end();

            timer.end();

            a3Log::info("Cost time: %f sec", timer.difference());

            once = true;
        }
    }
#else

#endif
}

//--------------------------------------------------------------
void ofApp::draw(){
#ifdef TEST
    int gridWidth = renderer->gridWidth;
    int gridHeight = renderer->gridHeight;

    int gridX = renderer->startX + (int) ((renderer->currentGrid - 1) % renderer->levelX) * gridWidth;
    int gridY = renderer->startY + (int) ((renderer->currentGrid - 1) / renderer->levelY) * gridHeight;
    int gridEndX = gridX + gridWidth;
    int gridEndY = gridY + gridHeight;

    // 更新网格待渲染区域
    if(!renderer->isFinished())
    {
#pragma omp parallel for schedule(dynamic)
        for(int x = gridX; x < gridEndX; x++)
        {
            for(int y = gridY; y < gridEndY; y++)
            {
                a3Spectrum& color = renderer->colorList[x + y * imageWidth];

                previewPixels.setColor(x, y, ofColor(color.x * 255, color.y * 255, color.z * 255));
            }
        }

        preview.loadData(previewPixels);
    }

    preview.draw(0, 0, imageWidth, imageHeight);
#else
    gui.begin();

    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Rendering"))
        {
            openRenderingWindow = !openRenderingWindow;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Camera"))
        {
            openCameraWindow = !openCameraWindow;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Shape"))
        {
            openShapeWindow = !openShapeWindow;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Light"))
        {
            openLightWindow = !openLightWindow;
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    renderingMenu();
    cameraMenu();
    shapeMenu();
    lightMenu();

    gui.end();
#endif
}

//--------------------------------------------------------------
void ofApp::initAtmos()
{

}

//--------------------------------------------------------------
void ofApp::initImGui()
{
    //io.Fonts->AddFontFromFileTTF("./data/Cousine-Regular.ttf", 10);

    openCameraWindow = true;
    openShapeWindow = true;
    openRenderingWindow = true;
    openLightWindow = true;

    // config
    startFrame = 0.0f;
    endFrame = 0.0f;

    imageWidth = 1024;
    imageHeight = 768;

    hasKeyFrame = false;

    // image
    localStartPos[0] = 0;
    localStartPos[1] = 0;
    localRenderSize[0] = imageWidth;
    localRenderSize[1] = imageHeight;

    strcpy(saveToPath, "./data/");

    // integrator
    enablePath = true;
    enableBVH = true;
    maxDepth = -1;
    russianRouletteDepth = 3;

    // camera
    cameraLookat[0] = 0.0f;
    cameraLookat[1] = 0.0f;
    cameraLookat[2] = 0.0f;
    cameraUp[0] = 0.0f;
    cameraUp[1] = 0.0f;
    cameraUp[2] = 0.0f;   
    cameraOrigin[0] = 0.0f;
    cameraOrigin[1] = 0.0f;
    cameraOrigin[2] = 0.0f;

    cameraFov = 40.0f;
    cameraFocalDistance = 100.0f;
    cameraLensRadius = 0.0f;
}

//--------------------------------------------------------------
void ofApp::renderingMenu()
{
    if(!openRenderingWindow) return;

    ImGuiWindowFlags window_flags = 0;
    ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiSetCond_FirstUseEver);
    if(ImGui::Begin("Rendering", &openRenderingWindow))
    {
        ImGui::Text("Config");
        if(ImGui::DragInt("Start Frame", &startFrame, 1, 0, 10000))
        {
            if(startFrame > endFrame)
                startFrame = endFrame;
        }

        if(ImGui::DragInt("End Frame", &endFrame, 1, 0, 10000))
        {
            if(endFrame < startFrame)
                endFrame = startFrame;
        }

        ImGui::DragInt("Spp", &spp, 1, 0, 1000000);

        ImGui::Checkbox("Has Key Frame ?##Rendering", &hasKeyFrame);

        ImGui::Separator();

        ImGui::Text("Image");
        ImGui::DragInt("Width", &imageWidth, 1, 1, 1000000);
        ImGui::DragInt("Height", &imageHeight, 1, 1, 1000000);

        ImGui::Separator();
        ImGui::Text("Local Rendering");
        if(ImGui::DragInt2("start", localStartPos, 1, 1, 1000000))
        {
            if(localStartPos[0] + localRenderSize[0] > imageWidth)
                localStartPos[0] = imageWidth - localRenderSize[0];

            if(localStartPos[1] + localRenderSize[1] > imageHeight)
                localStartPos[1] = imageHeight - localRenderSize[1];
        }
        if(ImGui::DragInt2("size", localRenderSize, 1, 1, 1000000))
        {
            if(localRenderSize[0] + localStartPos[0] > imageWidth)
                localRenderSize[0] = imageWidth - localStartPos[0];

            if(localRenderSize[1] + localStartPos[1] > imageHeight)
                localRenderSize[1] = imageHeight - localStartPos[1];
        }

        ImGui::Separator();
        ImGui::Text("Save");
        ImGui::InputText("Image Path", saveToPath, sizeof(saveToPath) / sizeof(char));

        // save to button with custom color
        ImGui::PushID(0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));
        if(ImGui::Button("Save To..."))
        {
            ofFileDialogResult result = ofSystemSaveDialog("", "");
            strcpy(saveToPath, result.getPath().c_str());
            strcpy(saveImageName, result.getName().c_str());
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Set the image's save path");

        ImGui::Separator();
        ImGui::Text("Integrator");
        static int e = 0;
        if(ImGui::RadioButton("Direct", &e, 0))
            enablePath = false;
        ImGui::SameLine();
        if(ImGui::RadioButton("Path", &e, 1))
            enablePath = true;
        
        if(enablePath)
        {
            ImGui::DragInt("RR Depth", &russianRouletteDepth, 1, 0, 1000000);
            if(ImGui::IsItemHovered())
                ImGui::SetTooltip("Set number of Russian-Roulette Depth");
        }
        else
        {
            ImGui::DragInt("Max Depth", &maxDepth, 1, 0, 1000000);
            if(ImGui::IsItemHovered())
                ImGui::SetTooltip("Set Direct Integrator's Max Depth");
        }

        ImGui::Separator();
        ImGui::Text("Primitive Set");
        static int e1 = 0;
        if(ImGui::RadioButton("BVH", &e1, 0))
            enableBVH = true;
        ImGui::SameLine();
        if(ImGui::RadioButton("Exaustive", &e1, 1))
            enableBVH = false;

        ImGui::Text(("Time Elapsed: " + ofToString(ofGetElapsedTimef()) + "s").c_str());
        ImGui::Text("Time Remainning");
    }

    ImGui::End();
}

//--------------------------------------------------------------
void ofApp::cameraMenu()
{
    if(!openCameraWindow) return;

    ImGuiWindowFlags window_flags = 0;
    ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiSetCond_FirstUseEver);
    if(ImGui::Begin("Camera", &openCameraWindow))
    {
        ImGui::DragFloat3("Lookat", cameraLookat, 1.0f);
        ImGui::DragFloat3("Origin", cameraOrigin, 1.0f);
        ImGui::DragFloat3("Up", cameraUp, 1.0f);

        ImGui::Separator();
        ImGui::Text("Lens");
        ImGui::DragFloat("Focal Distance", &cameraFocalDistance, 1.0f, 0.0f);
        ImGui::DragFloat("Lens Radius", &cameraLensRadius, 1.0f, 0.0f, 1000.0f);
    }

    ImGui::End();
}

//--------------------------------------------------------------
void ofApp::shapeMenu()
{
    if(!openShapeWindow) return;

    ImGuiWindowFlags window_flags = 0;
    ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiSetCond_FirstUseEver);
    if(ImGui::Begin("Shape", &openShapeWindow))
    {
        // 强行规定顺序
        const char* items[] = {"Triangle Mesh", "Infinite Plane", "Sphere", "Disk", "Triangle", "Plane"};
        static int item2 = 1;
        ImGui::Combo("Shape Type", &item2, items, 6);

        if(ImGui::Button("Add Shape"))
        {
            switch(item2)
            {
            case 0:
                // 本质为读取obj 此处仅作占位符
                shapeList.push_back(new meshData());
                break;
            case 1:
                shapeList.push_back(new infinitePlaneData());
                break;
            case 2:
                shapeList.push_back(new sphereData());
                break;
            case 3:
                shapeList.push_back(new diskData());
                break;
            case 4:
                shapeList.push_back(new triangleData());
                break;
            case 5:
                shapeList.push_back(new planeData());
                break;
            }
        }            
        
        for(int index = 0; index < shapeList.size(); index++)
        {
            shapeData* i = shapeList[index];

            if(i->name == "Mesh")
                shapeTriangleMesh(index);
            else if(i->name == "InfinitePlane")
                shapeInfinitePlane(index);
            else if(i->name == "Sphere")
                shapeSphere(index);
            else if(i->name == "Disk")
                shapeDisk(index);
            else if(i->name == "Triangle")
                shapeTriangle(index);
            else if(i->name == "Plane")
                shapePlane(index);
        }
    }
    ImGui::End();
}

//--------------------------------------------------------------
void ofApp::lightMenu()
{
    if(!openLightWindow) return;

    ImGuiWindowFlags window_flags = 0;
    ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiSetCond_FirstUseEver);
    if(ImGui::Begin("Light", &openLightWindow))
    {
        // 强行规定顺序
        const char* items[] = {"Area", "Spot", "Point", "Infinite"};
        static int item2 = 1;
        ImGui::Combo("Shape Type", &item2, items, 4);

        if(ImGui::Button("Add Light"))
        {
            switch(item2)
            {
            case 0:
                // 本质为读取obj 此处仅作占位符
                lightList.push_back(new areaLightData());
                break;
            case 1:
                lightList.push_back(new spotLightData());
                break;
            case 2:
                lightList.push_back(new pointLightData());
                break;
            case 3:
                lightList.push_back(new infiniteAreaLightData());
                break;
            }
        }

        for(int index = 0; index < lightList.size(); index++)
        {
            lightData* i = lightList[index];

            if(i->name == "Area Light")
                lightArea(index);
            else if(i->name == "Spot Light")
                lightSpot(index);
            else if(i->name == "Point Light")
                lightPoint(index);
            else if(i->name == "Inifinite Area Light")
                lightInfinite(index);
        }
    }

    ImGui::End();
}

//--------------------------------------------------------------
void ofApp::shapeSphere(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3Sphere"

    ImGui::Separator();
    string sphereCount = "Sphere";
    sphereCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", sphereCount.c_str());

    sphereData* sphere = (sphereData*)shapeList[index];

    // radius
    string radiusWithID = "Radius##";
    addSuffix(radiusWithID);
    ImGui::DragFloat(radiusWithID.c_str(), &sphere->radius, 1.0f, 0.0f, 1000.0f);

    // position
    string centerWithID = "Center##";
    addSuffix(centerWithID);
    ImGui::DragFloat3(centerWithID.c_str(), sphere->center, 1.0f);

    setBSDF(index, sphere);

    shapeDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::shapeTriangle(int index)
{
    a3Log::warning("Triangle: 木有实现, 呵呵\n");
}

//--------------------------------------------------------------
void ofApp::shapeTriangleMesh(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3TriangleMesh"

    // 目前由于Mesh开发未完成，因此延迟分配primitive，仅作地址保存
    ImGui::Separator();
    string meshCount = "Mesh";
    meshCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", meshCount.c_str());

    meshData* mesh = (meshData*) shapeList[index];

    string supportButton = "Support Key Frame ?##";
    addSuffix(supportButton);
    ImGui::Checkbox(supportButton.c_str(), &mesh->supportKeyFrame);

    string modelPath = "Model Path##";
    addSuffix(modelPath);
    ImGui::InputText(modelPath.c_str(), mesh->modelPath, 1024);

    // save to button with custom color
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));
    if(ImGui::Button("Model Path..."))
    {
        ofFileDialogResult result = ofSystemLoadDialog("Open Obj Model", false, "");
        strcpy(mesh->modelPath, result.getPath().c_str());
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    setBSDF(index, mesh);

    shapeDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::shapeDisk(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3Disk"

    ImGui::Separator();
    string diskCount = "Disk";
    diskCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", diskCount.c_str());

    diskData* disk = (diskData*) shapeList[index];

    // radius
    string radiusWithID = "Radius##";
    addSuffix(radiusWithID);
    ImGui::DragFloat(radiusWithID.c_str(), &disk->radius, 1.0f, 0.0f, 1000.0f);

    // center
    string centerWithID = "Center##";
    addSuffix(centerWithID);
    ImGui::DragFloat3(centerWithID.c_str(), disk->center, 1.0f);

    // normal
    string normalWithID = "Normal##";
    addSuffix(normalWithID);
    ImGui::DragFloat3(normalWithID.c_str(), disk->normal, 0.0001f, -1.0f, 1.0f);

    setBSDF(index, disk);

    shapeDelete(index);
#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::shapePlane(int index)
{
    a3Log::warning("Plane: 木有实现, 呵呵\n");
}

//--------------------------------------------------------------
void ofApp::shapeInfinitePlane(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3InfinitePlane"

    ImGui::Separator();
    string inifinitePlaneCount = "InfinitePlane";
    inifinitePlaneCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", inifinitePlaneCount.c_str());

    infinitePlaneData* plane = (infinitePlaneData*) shapeList[index];

    // normal
    string normalWithID = "Normal##";
    addSuffix(normalWithID);
    ImGui::DragFloat3(normalWithID.c_str(), plane->normal, 0.0001f, -1.0f, 1.0f);

    // position
    string centerWithID = "Center##";
    addSuffix(centerWithID);
    ImGui::DragFloat3(centerWithID.c_str(), plane->position, 1.0f);

    setBSDF(index, plane);

    shapeDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::shapeDelete(int index)
{
    shapeData* data = shapeList[index];

    string deleteShape = "Delete Shape##";
    deleteShape += ofToString(index) + data->name;

    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.0f, 0.8f, 0.8f));
    if(ImGui::Button(deleteShape.c_str()))
    {
        for(vector<shapeData*>::iterator iter = shapeList.begin(); iter != shapeList.end(); )
        {
            if(*iter == data)
                iter = shapeList.erase(iter);
            else
                iter++;
        }

        delete data;
        data = NULL;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
}

//--------------------------------------------------------------
void ofApp::setBSDF(int index, shapeData* shape)
{
    string materialTypeName = "Material Type##";
    materialTypeName += ofToString(index) + shape->name;

    // 强行规定顺序
    const char* items[] = {"Glass", "Mirror", "Diffuse"};
    ImGui::Combo(materialTypeName.c_str(), &shape->materialType, items, 3);
}

//--------------------------------------------------------------
void ofApp::lightArea(int index)
{
    a3Log::warning("Area Light: 木有实现, 呵呵\n");
}

//--------------------------------------------------------------
void ofApp::lightSpot(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3SpotLight"

    ImGui::Separator();
    string spotLightCount = "Spot Light";
    spotLightCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", spotLightCount.c_str());

    spotLightData* data = (spotLightData*)lightList[index];

    // position
    string posWithID = "Position##";
    addSuffix(posWithID);
    ImGui::DragFloat3(posWithID.c_str(), data->position, 1.0f);

    // intensity
    string intensityWithID = "Intensity##";
    addSuffix(intensityWithID);
    ImGui::DragFloat3(intensityWithID.c_str(), data->intensity, 1.0f);

    // direction
    string dirWithID = "Direction##";
    addSuffix(dirWithID);
    ImGui::DragFloat3(dirWithID.c_str(), data->direction, 0.0001f, -1.0f, 1.0f);

    string coneAngleWithID = "Cone Angle##";
    addSuffix(coneAngleWithID);
    if(ImGui::DragFloat(coneAngleWithID.c_str(), &data->coneAngle, 1.0f, 0.0f, 360.0f))
    {
        if(data->coneAngle < data->falloffStart)
            data->coneAngle = data->falloffStart;
    }

    string falloffWithID = "Cos Falloff Start##";
    addSuffix(falloffWithID);
    if(ImGui::DragFloat(falloffWithID.c_str(), &data->falloffStart, 1.0f, 0.0f, 360.0f))
    {
        if(data->falloffStart > data->coneAngle)
            data->falloffStart = data->coneAngle;
    }

    lightDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::lightPoint(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3PointLight"

    ImGui::Separator();
    string spotLightCount = "Point Light";
    spotLightCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", spotLightCount.c_str());

    pointLightData* data = (pointLightData*) lightList[index];

    // position
    string posWithID = "Position##";
    addSuffix(posWithID);
    ImGui::DragFloat3(posWithID.c_str(), data->position, 1.0f);

    // intensity
    string intensityWithID = "Intensity##";
    addSuffix(intensityWithID);
    ImGui::DragFloat3(intensityWithID.c_str(), data->intensity, 1.0f);

    lightDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::lightInfinite(int index)
{
#define addSuffix(str) str += ofToString(index) + "a3PointLight"

    ImGui::Separator();
    string infiniteAreaLightCount = "Infinite Area Light";
    infiniteAreaLightCount += "[" + ofToString(index) + "]";
    ImGui::LabelText("Parameter", infiniteAreaLightCount.c_str());

    infiniteAreaLightData* data = (infiniteAreaLightData*) lightList[index];

    string imagePathWithID = "Model Path##";
    addSuffix(imagePathWithID);
    ImGui::InputText(imagePathWithID.c_str(), data->imagePath, 1024);

    // save to button with custom color
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));
    if(ImGui::Button("Image Path..."))
    {
        ofFileDialogResult result = ofSystemLoadDialog("Open PNG/PPM/EXR Model", false, "");
        strcpy(data->imagePath, result.getPath().c_str());
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    lightDelete(index);

#undef addSuffix
}

//--------------------------------------------------------------
void ofApp::lightDelete(int index)
{
    lightData* data = lightList[index];

    string deleteShape = "Delete Light##";
    deleteShape += ofToString(index) + data->name;

    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.0f, 0.8f, 0.8f));
    if(ImGui::Button(deleteShape.c_str()))
    {
        for(vector<lightData*>::iterator iter = lightList.begin(); iter != lightList.end(); )
        {
            if(*iter == data)
                iter = lightList.erase(iter);
            else
                iter++;
        }

        delete data;
        data = NULL;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
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


void ofApp::imGuiTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.0;
    //style.WindowFillAlphaDefault = 0.83;
    style.ChildWindowRounding = 3;
    style.WindowRounding = 3;
    style.GrabRounding = 1;
    style.GrabMinSize = 20;
    style.FrameRounding = 3;

    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ComboBg] = ImVec4(0.16f, 0.24f, 0.22f, 0.60f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
    style.Colors[ImGuiCol_Column] = ImVec4(0.00f, 0.50f, 0.50f, 0.33f);
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.00f, 0.50f, 0.50f, 0.47f);
    style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.00f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_CloseButton] = ImVec4(0.00f, 0.78f, 0.78f, 0.35f);
    style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.00f, 0.78f, 0.78f, 0.47f);
    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.00f, 0.78f, 0.78f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
    //style.Colors[ImGuiCol_TooltipBg] = ImVec4(0.00f, 0.13f, 0.13f, 0.90f);
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.04f, 0.10f, 0.09f, 0.51f);
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
