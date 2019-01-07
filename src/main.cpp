// MRE.cpp : Defines the entry point for the application.
// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions. You may freely use any other OpenGL loader such as: glew, glad, glLoadGen, etc.
#include <GLFW/glfw3.h>

//#include "../mathematics/geometry.h"

#include <thread>

#include "gui/TinyConsole.h"
#include "gui/widget.h"
#include "gui/eventhandler.h"
#include "utility/cmdline.h"
#include "volume/volume_utils.h"
#include "volume/volume.h"
#include "cameras/camera.h"
#include "opengl/shader.h"
#include "opengl/openglbuffer.h"
#include "opengl/openglvertexarrayobject.h"
#include "opengl/texture.h"
#include "openglutils.h"
#include "volume/virtualvolumehierachy.h"
#include "GL/glcorearb.h"
#include "framebuffer.h"
#include "timer.h"


//const static int g_proxyGeometryVertexIndices[] = { 1, 3, 7, 5, 0, 4, 6, 2, 2, 6, 7, 3, 0, 1, 5, 4, 4, 5, 7, 6, 0, 2, 3, 1 };
const static int g_proxyGeometryVertexIndices[] = { 1,3,7,1,7,5,0,4,6,0,6,2,2,6,7,2,7,3,0,1,5,0,5,4,4,5,7,4,7,6,0,2,3,0,3,1 };
static const float xCoord = 1.0, yCoord = 1.0, zCoord = 1.0;
static float g_proxyGeometryVertices[] = {
	0,0,0,
	xCoord, 0.0, 0.0 ,
	0 , yCoord , 0 ,
	xCoord , yCoord , 0 ,
	0 , 0 , zCoord  ,
	xCoord , 0 , zCoord ,
	0 , yCoord , zCoord,
	xCoord , yCoord , zCoord ,
		-0.5,0 - 0.5,0 - 0.5,
	xCoord - 0.5, 0.0 - 0.5, 0.0 - 0.5 ,
	0 - 0.5 , yCoord - 0.5, 0 - 0.5,
	xCoord - 0.5, yCoord - 0.5 , 0 - 0.5,
	0 - 0.5, 0 - 0.5 , zCoord - 0.5 ,
	xCoord - 0.5 , 0 - 0.5, zCoord - 0.5,
	0 - 0.5 , yCoord - 0.5, zCoord - 0.5,
	xCoord - 0.5, yCoord - 0.5, zCoord - 0.5,
};


/*
 * The reason why make the members public and supports corresponding
 * access methods as the same time is that these class should be redesigned later conform to OO
 */



 /**************************************************/

 /**************************************************/

namespace
{
	FocusCamera g_camera{ ysl::Point3f{0.f,0.f,10.f} };
	ysl::Transform g_projMatrix;
	ysl::Transform g_orthoMatrix;
	ysl::ShaderProgram g_rayCastingShaderProgram;
	ysl::ShaderProgram g_positionShaderProgram;
	ysl::ShaderProgram g_quadsShaderProgram;
	ysl::ShaderProgram g_clearIntermediateProgram;
	ysl::Point2i g_lastMousePos;

	//std::unique_ptr<char[]> g_rawData;
	std::vector<ysl::RGBASpectrum> m_tfData{ 256 };

	std::string g_lvdFileName = "D:\\scidata\\abc\\s1_512_512_512.lvd";

	ysl::Vector3f g_lightDirection;

	float step = 0.001;
	float ka = 1.0;
	float ks = 1.0;
	float kd = 1.0;
	float shininess = 50.0f;

	constexpr int pageTableBlockEntry = 16;
	Timer g_timer;
	Timer g_timer2;

	const std::size_t missedBlockCapacity = 5000 * sizeof(unsigned int);


	std::shared_ptr<OpenGLTexture> g_texTransferFunction;

	std::shared_ptr<OpenGLFramebufferObject> g_framebuffer;


	std::shared_ptr<OpenGLTexture> g_texEntryPos;

	std::shared_ptr<OpenGLTexture> g_texExitPos;

	std::shared_ptr<OpenGLTexture> g_texDepth;

	std::shared_ptr<OpenGLTexture> g_texIntermediateResult;


	std::shared_ptr<OpenGLBuffer> g_proxyEBO;

	std::shared_ptr<OpenGLBuffer> g_proxyVBO;

	OpenGLVertexArrayObject g_proxyVAO;

	std::shared_ptr<OpenGLBuffer> g_rayCastingVBO;

	OpenGLVertexArrayObject g_rayCastingVAO;

	std::shared_ptr<OpenGLTexture> g_texPageTable;

	std::shared_ptr<OpenGLTexture> g_texCache;
	std::list<std::pair<PageTableEntryAbstractIndex, CacheBlockAbstractIndex>> g_lruList;

	std::shared_ptr<OpenGLBuffer> g_blockPingBuf;
	std::shared_ptr<OpenGLBuffer> g_blockPongBuf;
	unsigned int g_pingPBO;
	unsigned int g_pongPBO;
	char * g_pboPtr[2];


	std::vector<GlobalBlockAbstractIndex> g_hits;
	std::vector<int> g_posInCache;
	ysl::Size3 g_gpuCacheBlockSize{4,4,4};


	std::shared_ptr<OpenGLBuffer> g_bufMissedHash;

	std::shared_ptr<OpenGLBuffer> g_bufMissedTable;

	std::shared_ptr<OpenGLBuffer> g_atomicCounter;

	char * g_data;


	std::unique_ptr<VolumeVirtualMemoryHierachy<pageTableBlockEntry, pageTableBlockEntry, pageTableBlockEntry>> g_largeVolumeData;

	int pageDirX;
	int pageDirY;
	int pageDirZ;

	int pageTableX;
	int pageTableY;
	int pageTableZ;

	int cacheWidth;
	int cacheHeight;
	int cacheDepth;
	int repeat;
	int blockDataSize;
	int originalDataWidth;
	int originalDataHeight;
	int originalDataDepth;

	int xBlockCount;
	int yBlockCount;
	int zBlockCount;

	int initialWidth = 800, initialHeight = 600;

}


void checkFrambufferStatus();


bool compareTexture(const unsigned char* buf1, const unsigned char* buf2, std::size_t bytes)
{
	for(auto i = 0ULL ; i < bytes;i++)
	{
		if (*(buf1 + i) != *(buf2 + i))
		{
			return false;
		}
			
	}
	return true;
}


void renderingWindowResize(ResizeEvent *event)
{
	const auto x = event->size().x;
	const auto y = event->size().y;
	g_orthoMatrix.SetOrtho(0, x, 0, y, -10, 100);

	//
	const auto aspect = static_cast<float>(x) / static_cast<float>(y);
	g_projMatrix.SetPerspective(45.0f, aspect, 0.01, 100);

	// Update frambuffer size
	g_texEntryPos->SetData(OpenGLTexture::RGBA32F, OpenGLTexture::RGBA, OpenGLTexture::Float32, x, y, 0, nullptr);
	g_texExitPos->SetData(OpenGLTexture::RGBA32F, OpenGLTexture::RGBA, OpenGLTexture::Float32, x, y, 0, nullptr);
	g_texDepth->SetData(OpenGLTexture::InternalDepthComponent, OpenGLTexture::ExternalDepthComponent, OpenGLTexture::Float32, x, y, 0, nullptr);
	g_texIntermediateResult->SetData(OpenGLTexture::RGBA32F, OpenGLTexture::RGBA, OpenGLTexture::Float32, x, y, 0, nullptr);

	// update raycastingtexture
	float windowSize[] =
	{
		0,0,x,0,0,y,0,y,x,0,x,y
	};

	g_rayCastingVBO->Bind();
	g_rayCastingVBO->AllocateFor(windowSize, sizeof(windowSize));
};




void mousePressedEvent(MouseEvent * event)
{
	g_lastMousePos = event->pos();
}

void mouseMoveEvent(MouseEvent * event)
{
	const auto & p = event->pos();
	// Update Camera
	float dx = p.x - g_lastMousePos.x;
	float dy = g_lastMousePos.y - p.y;
	if ((event->buttons() & MouseEvent::LeftButton) && (event->buttons() & MouseEvent::RightButton))
	{
		const auto directionEx = g_camera.up()*dy + dx * g_camera.right();
		g_camera.movement(directionEx, 0.002);
	}
	else if (event->buttons() & MouseEvent::LeftButton)
	{
		g_camera.rotation(dx, dy);
	}
	else if (event->buttons() == MouseEvent::RightButton)
	{
		const auto directionEx = g_camera.front()*dy;
		g_camera.movement(directionEx, 0.01);
	}
	//std::cout << g_camera.view();
	g_lastMousePos = p;
}

void mouseReleaseEvent(MouseEvent * event)
{

}

void keyboardPressEvent(KeyboardEvent)
{

}

void initBlockExistsHash()
{

	const auto ptr = g_bufMissedHash->Map(OpenGLBuffer::WriteOnly);
	memset(ptr, 0, g_bufMissedHash->Size());
	g_bufMissedHash->Unmap();
	GL_ERROR_REPORT;
}

void initMissedBlockVector()
{
	const auto ptr = g_bufMissedTable->Map(OpenGLBuffer::WriteOnly);
	memset(ptr, 0, g_bufMissedTable->Size());
	g_bufMissedTable->Unmap();
	g_atomicCounter->BindBufferBase(3);
	unsigned int zero = 0;
	g_atomicCounter->AllocateFor(&zero, sizeof(unsigned int));
	GL_ERROR_REPORT;
}

void initGPUCacheLRUPolicyList()
{
	const auto size = g_largeVolumeData->BlockSize();
	//const auto w = xCacheBlockCount(), h = yCacheBlockCount(), d = zCacheBlockCount();
	for (auto z = 0; z < g_gpuCacheBlockSize.x; z++)
		for (auto y = 0; y < g_gpuCacheBlockSize.y; y++)
			for (auto x = 0; x < g_gpuCacheBlockSize.z; x++)
			{
				g_lruList.push_back(std::make_pair(PageTableEntryAbstractIndex(-1, -1, -1), CacheBlockAbstractIndex(x*size, y*size, z*size)));
			}

	g_texPageTable->SetData(OpenGLTexture::RGBA32UI, OpenGLTexture::RGBAInteger, OpenGLTexture::UInt32, pageTableX, pageTableY, pageTableZ, g_largeVolumeData->PageTable->Data());
}

/**
 * \brief  Returns \a true if cache missed, otherwise returns false
 */
bool CaptureAndHandleCacheMiss()
{
	// There is a run-time error at nvoglv64.dll
	const auto counters = static_cast<int*>(g_atomicCounter->Map(OpenGLBuffer::ReadOnly));
	const int count = counters[0];
	if (count == 0)
		return false;

	const std::size_t cacheBlockThreshold = g_gpuCacheBlockSize.x*g_gpuCacheBlockSize.y*g_gpuCacheBlockSize.z;
	const std::size_t blockSize = g_largeVolumeData->BlockSize();
	const auto missedBlocks = (std::min)(count, (int)cacheBlockThreshold);


	g_hits.clear();
	g_posInCache.clear();
	g_hits.reserve(missedBlocks);
	g_posInCache.reserve(missedBlocks * 3);

	const auto ptr = static_cast<int*>(g_bufMissedTable->Map(OpenGLBuffer::ReadWrite));
	for (auto i = 0; i < missedBlocks; i++)
	{
		const auto blockId = ptr[i];
		g_hits.emplace_back(blockId, xBlockCount, yBlockCount, zBlockCount);
	}
	g_bufMissedTable->Unmap();


	auto & pageTable = *g_largeVolumeData->PageTable;
	auto & LRUList = g_lruList;

	
	//static int hitCount = 0;

	;

	// Update LRU List 
	for (int i = 0; i < missedBlocks; i++)
	{
		const auto & index = g_hits[i];
		auto & pageTableEntry = pageTable(index.x, index.y, index.z);
		auto & last = LRUList.back();
		pageTableEntry.w = EntryFlag::Mapped;		// Map the flag of page table entry
		// last.second is the cache block index

		const auto xInCache = last.second.x;
		const auto yInCache = last.second.y;
		const auto zInCache = last.second.z;

		pageTableEntry.x = xInCache;			// fill the page table entry
		pageTableEntry.y = yInCache;
		pageTableEntry.z = zInCache;

		if (last.first.x != -1)
		{
			pageTable(last.first.x, last.first.y, last.first.z).w = EntryFlag::Unmapped;
		}

		last.first.x = index.x;
		last.first.y = index.y;
		last.first.z = index.z;
		LRUList.splice(LRUList.begin(), LRUList, --LRUList.end());		// move from tail to head, LRU policy

		g_posInCache.push_back(xInCache);
		g_posInCache.push_back(yInCache);
		g_posInCache.push_back(zInCache);




	}
	// Update load missed block  data to GPU

	const auto blockBytes = blockSize * blockSize*blockSize * sizeof(char);

	// initialize second pbo 
	// Ping-Pong PBO Transfer
	
	long long accessPageTableTime = 0, 
	readDataTime = 0, 
	copyDataTime = 0, 
	dmaTime = 0,
	totalTime = 0,
	bindTime = 0 ;

	

	//std::shared_ptr<OpenGLBuffer> pbo[2] = { g_blockPingBuf,g_blockPongBuf };
	//auto curPBO = 0;
	//auto i = 0;
	//const auto & idx = g_hits[i];
	//const auto dd = g_largeVolumeData->ReadBlockDataFromCache(idx);

	//pbo[1 - curPBO]->Bind();
	//auto pp = pbo[1 - curPBO]->Map(OpenGLBuffer::WriteOnly);
	//memcpy(pp, dd, blockBytes);
	//pbo[1 - curPBO]->Unmap();		// copy data to pbo
	//pbo[1 - curPBO]->Bind();
	//
	//g_texCache->Bind();
	//for (;i < missedBlocks-1;)
	//{
	//	pbo[1 - curPBO]->Bind();
	//	g_texCache->SetSubData(OpenGLTexture::RED,
	//		OpenGLTexture::UInt8,
	//		g_posInCache[3 * i], blockSize,
	//		g_posInCache[3 * i + 1], blockSize,
	//		g_posInCache[3 * i + 2], blockSize,
	//		nullptr);
	//	pbo[1 - curPBO]->Unbind();
	//	i++;
	//	const auto & index = g_hits[i];

	//	const auto d = g_largeVolumeData->ReadBlockDataFromCache(index);

	//	pbo[curPBO]->Bind();
	//	auto p = pbo[curPBO]->Map(OpenGLBuffer::WriteOnly);

	//	memcpy(p, d, blockBytes);

	//	pbo[curPBO]->Unmap();		// copy data to pbo
	//	curPBO = 1 - curPBO;
	//}
	//pbo[1 - curPBO]->Bind();
	//g_texCache->SetSubData(OpenGLTexture::RED,
	//	OpenGLTexture::UInt8,
	//	g_posInCache[3 * i], blockSize,
	//	g_posInCache[3 * i + 1], blockSize,
	//	g_posInCache[3 * i + 2], blockSize,
	//	nullptr);
	//pbo[1 - curPBO]->Unbind();

	

	//////////////////


	//g_timer.begin();
	//g_texCache->Bind();
	//unsigned int pbo[2] = { g_pingPBO,g_pongPBO };
	//auto curPBO = 0;
	//auto i = 0;
	//const auto & idx = g_hits[i];
	//const auto dd = g_largeVolumeData->ReadBlockDataFromCache(idx);
	////glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[1 - curPBO]);
	//memcpy(g_pboPtr[1-curPBO], dd, blockBytes);
	//for (; i < missedBlocks - 1;)
	//{
	//	//using namespace std::chrono_literals;
	//	//std::this_thread::sleep_for(50ms);
	//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[1 - curPBO]);
	//	GL_ERROR_REPORT;
	//	g_texCache->SetSubData(OpenGLTexture::RED,
	//		OpenGLTexture::UInt8,
	//		g_posInCache[3 * i], blockSize,
	//		g_posInCache[3 * i + 1], blockSize,
	//		g_posInCache[3 * i + 2], blockSize,
	//		nullptr);
	//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	//	//ysl::Log("pos in cache:[%d, %d, %d], write to %u\n", 
	//	//	g_posInCache[3 * i],
	//	//	g_posInCache[3 * i + 1],
	//	//	g_posInCache[3 * i + 2],g_pboPtr[1-curPBO]);

	//	GLsync s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	//	glClientWaitSync(s, 0, 1000000);
	//	glDeleteSync(s);

	//	i++;
	//	const auto & index = g_hits[i];
	//	const auto d = g_largeVolumeData->ReadBlockDataFromCache(index);
	//	memcpy(g_pboPtr[curPBO], d, blockBytes);
	//	curPBO = 1 - curPBO;

	//}

	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[1 - curPBO]);
	//g_texCache->SetSubData(OpenGLTexture::RED,
	//	OpenGLTexture::UInt8,
	//	g_posInCache[3 * i], blockSize,
	//	g_posInCache[3 * i + 1], blockSize,
	//	g_posInCache[3 * i + 2], blockSize,
	//	nullptr);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	//g_texCache->Unbind();

	//g_timer.end();
	//ysl::Log("%d %d %d\n", missedBlocks, g_timer.duration(), g_timer.duration() / missedBlocks);


	///////

	g_texCache->Bind();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pingPBO);
	g_timer.begin();
	//GL_ERROR_REPORT;
	for(int i = 0;i<missedBlocks;i++)
	{
		const auto & index = g_hits[i];
		//const auto d = g_largeVolumeData->ReadBlockDataFromCache(index);
		memcpy(g_pboPtr[0], g_data, blockBytes);

		g_texCache->SetSubData(OpenGLTexture::RED,
			OpenGLTexture::UInt8,
			g_posInCache[3 * i], blockSize,
			g_posInCache[3 * i + 1], blockSize,
			g_posInCache[3 * i + 2], blockSize,
			nullptr);

		GLsync s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(s, 0, 1000000);
		glDeleteSync(s);

	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	g_texCache->Unbind();

	//ysl::Log("%d %d %d\n", missedBlocks, g_timer.duration(), g_timer.duration() / missedBlocks);
	//ysl::Log("blockCount:%d\nreadDataTime:%d\ncopyDataTime:%d\nDMATime:%d\ntotalTime:%d\nbindTime:%d\n", missedBlocks,readDataTime,copyDataTime,dmaTime,totalTime,bindTime);
	// update page table
	g_texPageTable->Bind();
	g_texPageTable->SetData(OpenGLTexture::RGBA32UI,
		OpenGLTexture::RGBAInteger,
		OpenGLTexture::UInt32,
		pageTableX,
		pageTableY,
		pageTableZ,
		pageTable.Data());
	g_texPageTable->Unbind();

	return true;
}

void initializeShaders()
{
	// Ray casting in gpu
	g_rayCastingShaderProgram.create();
	g_rayCastingShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\blockraycasting_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_rayCastingShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\blockraycasting_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_rayCastingShaderProgram.link();

	// Generate entry and exit position
	g_positionShaderProgram.create();
	g_positionShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\position_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_positionShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\position_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_positionShaderProgram.link();

	//  Draw the result texture into the default framebuffer
	g_quadsShaderProgram.create();
	g_quadsShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\quadsshader_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_quadsShaderProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\quadsshader_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_quadsShaderProgram.link();

	// Initialize the result texture
	g_clearIntermediateProgram.create();
	g_clearIntermediateProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\clear_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_clearIntermediateProgram.addShaderFromFile("D:\\code\\MRE\\src\\shader\\clear_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_clearIntermediateProgram.link();

}

void initializeProxyGeometry() {
	GL_ERROR_REPORT;

	g_proxyVAO.create();
	g_proxyVAO.bind();

	g_proxyVBO = std::make_shared<OpenGLBuffer>(OpenGLBuffer::VertexArrayBuffer);
	g_proxyVBO->Bind();
	g_proxyVBO->AllocateFor(g_proxyGeometryVertices, sizeof(g_proxyGeometryVertices));

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(sizeof(g_proxyGeometryVertices) / 2));
	GL_ERROR_REPORT;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
	GL_ERROR_REPORT;

	g_proxyEBO = std::make_shared<OpenGLBuffer>(OpenGLBuffer::ElementArrayBuffer);
	g_proxyEBO->Bind();
	g_proxyEBO->AllocateFor(g_proxyGeometryVertexIndices, sizeof(g_proxyGeometryVertexIndices));
	GL_ERROR_REPORT;

	g_proxyVAO.unbind();

	g_rayCastingVAO.create();
	g_rayCastingVAO.bind();

	//g_rayCastingVBO.create();
	g_rayCastingVBO = std::make_shared<OpenGLBuffer>(OpenGLBuffer::VertexArrayBuffer);
	g_rayCastingVBO->Bind();
	g_rayCastingVBO->AllocateFor(nullptr, 6 * sizeof(ysl::Point2f));

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));

	g_rayCastingVAO.unbind();
}

void initializeGPUTexture()
{
}


void initializeResource()
{


	initializeShaders();

	initializeProxyGeometry();



	g_largeVolumeData.reset(new VolumeVirtualMemoryHierachy<pageTableBlockEntry, pageTableBlockEntry, pageTableBlockEntry>(g_lvdFileName));

	pageDirX = g_largeVolumeData->PageDir->Size().x;
	pageDirY = g_largeVolumeData->PageDir->Size().y;
	pageDirZ = g_largeVolumeData->PageDir->Size().z;

	pageTableX = g_largeVolumeData->PageTable->Size().x;
	pageTableY = g_largeVolumeData->PageTable->Size().y;
	pageTableZ = g_largeVolumeData->PageTable->Size().z;

	cacheWidth = g_largeVolumeData->CacheSize().x;
	cacheHeight = g_largeVolumeData->CacheSize().y;
	cacheDepth = g_largeVolumeData->CacheSize().z;

	int blockSize = g_largeVolumeData->BlockSize();
	ysl::Size3 gpuBlockCacheSize = g_gpuCacheBlockSize * blockSize;  // number of block at every dim



	repeat = g_largeVolumeData->BoundaryRepeat();
	blockDataSize = g_largeVolumeData->BlockSize();
	originalDataWidth = g_largeVolumeData->OriginalDataSize().x;
	originalDataHeight = g_largeVolumeData->OriginalDataSize().y;
	originalDataDepth = g_largeVolumeData->OriginalDataSize().z;
	xBlockCount = g_largeVolumeData->SizeByBlock().x;
	yBlockCount = g_largeVolumeData->SizeByBlock().y;
	zBlockCount = g_largeVolumeData->SizeByBlock().z;

	const std::size_t totalBlockCountBytes = std::size_t(xBlockCount) * std::size_t(yBlockCount)*std::size_t(zBlockCount) * sizeof(int);

	std::cout << "pageDirX:" << pageDirX << std::endl;
	std::cout << "pageDirY:" << pageDirY << std::endl;
	std::cout << "pageDirZ:" << pageDirZ << std::endl;
	std::cout << "pageTableX:" << pageTableX << std::endl;
	std::cout << "pageTableY:" << pageTableY << std::endl;
	std::cout << "pageTableZ:" << pageTableZ << std::endl;
	std::cout << "cacheWidth:" << cacheWidth << std::endl;
	std::cout << "cacheHeight:" << cacheHeight << std::endl;
	std::cout << "cacheDepth:" << cacheDepth << std::endl;
	std::cout << "repeat:" << repeat << std::endl;
	//std::cout << "data size:" << g_largeVolumeData->Width() << std::endl;
	std::cout << "original data x:" << originalDataWidth << std::endl;
	std::cout << "original data y:" << originalDataHeight << std::endl;
	std::cout << "original data z:" << originalDataDepth << std::endl;
	std::cout << "BlockSize:" << g_largeVolumeData->BlockSize() << std::endl;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		// This is important


	GL_ERROR_REPORT;
	// page table
	g_texPageTable = OpenGLTexture::CreateTexture3D(OpenGLTexture::RGBA32UI, OpenGLTexture::Linear,
		OpenGLTexture::Linear, OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBAInteger,
		OpenGLTexture::UInt32, pageTableX, pageTableY, pageTableZ, g_largeVolumeData->PageTable->Data());
	// Bind to iimage3D
	g_texPageTable->Bind();
	g_texPageTable->BindToDataImage(1, 0, false, 0, OpenGLTexture::Read, OpenGLTexture::RGBA32UI);

	// allocate for volume data block cache

	initGPUCacheLRUPolicyList();
	g_texCache = OpenGLTexture::CreateTexture3D(OpenGLTexture::R16F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RED,
		OpenGLTexture::UInt8,
		gpuBlockCacheSize.x,
		gpuBlockCacheSize.y,
		gpuBlockCacheSize.z, nullptr);
	g_blockPingBuf = std::make_shared<OpenGLBuffer>(OpenGLBuffer::PixelUnpackBuffer, OpenGLBuffer::StreamDraw);
	g_blockPingBuf->AllocateFor(nullptr, blockDataSize*blockDataSize*blockDataSize * sizeof(char));
	g_blockPingBuf->Unbind();
	g_blockPongBuf = std::make_shared<OpenGLBuffer>(OpenGLBuffer::PixelUnpackBuffer, OpenGLBuffer::StreamDraw);
	g_blockPongBuf->AllocateFor(nullptr, blockDataSize*blockDataSize*blockDataSize * sizeof(char));
	g_blockPongBuf->Unbind();
	GL_ERROR_REPORT;

	g_bufMissedHash = std::make_shared<OpenGLBuffer>(OpenGLBuffer::ShaderStorageBuffer, OpenGLBuffer::DynamicDraw);
	g_bufMissedHash->AllocateFor(nullptr, totalBlockCountBytes);
	g_bufMissedHash->BindBufferBase(0);


	g_bufMissedTable = std::make_shared<OpenGLBuffer>(OpenGLBuffer::ShaderStorageBuffer, OpenGLBuffer::DynamicDraw);
	g_bufMissedTable->AllocateFor(nullptr, missedBlockCapacity);
	g_bufMissedTable->BindBufferBase(1);

	g_atomicCounter = std::make_shared<OpenGLBuffer>(OpenGLBuffer::AtomicCounterBuffer,
		OpenGLBuffer::DynamicCopy);
	g_atomicCounter->AllocateFor(nullptr, sizeof(GLuint));
	g_atomicCounter->BindBufferBase(3);
	g_atomicCounter->Unbind();
	GL_ERROR_REPORT;


	// shader program
	g_rayCastingShaderProgram.bind();
	//g_rayCastingShaderProgram.setUniformSampler("cacheVolume", OpenGLTexture::TextureUnit4, *g_texCache);
	g_rayCastingShaderProgram.setUniformValue("totalPageDirSize", ysl::Vector3i{ pageDirX,pageDirY,pageDirZ });
	g_rayCastingShaderProgram.setUniformValue("totalPageTableSize", ysl::Vector3i{ pageTableX,pageTableY,pageTableZ });
	g_rayCastingShaderProgram.setUniformValue("blockDataSize", ysl::Vector3i{ blockDataSize - 2 * repeat,blockDataSize - 2 * repeat,blockDataSize - 2 * repeat });
	g_rayCastingShaderProgram.setUniformValue("volumeDataSize", ysl::Vector3i{ originalDataWidth,originalDataDepth,originalDataDepth });
	g_rayCastingShaderProgram.setUniformValue("pageTableBlockEntrySize", ysl::Vector3i{ pageTableBlockEntry ,pageTableBlockEntry ,pageTableBlockEntry });
	g_rayCastingShaderProgram.setUniformValue("repeatSize", ysl::Vector3i{ repeat,repeat,repeat });
	g_rayCastingShaderProgram.unbind();

	g_texTransferFunction = OpenGLTexture::CreateTexture1D(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear, OpenGLTexture::Linear, OpenGLTexture::ClampToEdge, OpenGLTexture::RGBA, OpenGLTexture::Float32, 256, nullptr);

	g_texEntryPos = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		initialWidth,
		initialHeight,
		nullptr);
	g_texEntryPos->BindToDataImage(2, 0, false, 0, OpenGLTexture::ReadAndWrite, OpenGLTexture::RGBA32F);



	g_texExitPos = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		initialWidth,
		initialHeight,
		nullptr);

	g_texIntermediateResult = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		initialWidth,
		initialHeight,
		nullptr);


	g_texDepth = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::InternalDepthComponent,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ExternalDepthComponent,
		OpenGLTexture::Float32,
		initialWidth,
		initialHeight,
		nullptr);

	

	g_framebuffer = std::shared_ptr<OpenGLFramebufferObject>(new OpenGLFramebufferObject);
	g_framebuffer->Bind();
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::ColorAttachment0, g_texEntryPos);
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::ColorAttachment1, g_texExitPos);
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::ColorAttachment2, g_texIntermediateResult);
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::DepthAttachment, g_texDepth);
	g_framebuffer->CheckFramebufferStatus();
	g_framebuffer->Unbind();
	//init 
	initBlockExistsHash();
	initMissedBlockVector();
	{
		ysl::TransferFunction m_tfObject;
		m_tfObject.read("d:\\scidata\\std_tf1d.TF1D");
		m_tfObject.FetchData(m_tfData.data(), 256);
		g_texTransferFunction->SetData(OpenGLTexture::RGBA32F, OpenGLTexture::RGBA, OpenGLTexture::Float32, 256, 0, 0, m_tfData.data());
	}

}



void renderLoop()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	// clear intermediate result
	g_framebuffer->Bind();
	g_proxyVAO.bind();
	g_clearIntermediateProgram.bind();
	g_clearIntermediateProgram.setUniformValue("projMatrix", g_projMatrix.Matrix());
	g_clearIntermediateProgram.setUniformValue("worldMatrix", ysl::Transform{}.Matrix());
	g_clearIntermediateProgram.setUniformValue("viewMatrix", g_camera.view().Matrix());
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	GL_ERROR_REPORT;
	// Cull face
	g_positionShaderProgram.bind();
	g_positionShaderProgram.setUniformValue("projMatrix", g_projMatrix.Matrix());
	g_positionShaderProgram.setUniformValue("worldMatrix", ysl::Transform{}.Matrix());
	g_positionShaderProgram.setUniformValue("viewMatrix", g_camera.view().Matrix());
	GL_ERROR_REPORT;
	glDrawBuffer(GL_COLOR_ATTACHMENT0);					// Draw into attachment 0
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	GL_ERROR_REPORT;
	// Draw frontGL_ERROR_REPORT;
	glDepthFunc(GL_LESS);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	GL_ERROR_REPORT;

	// Draw Back
	glDrawBuffer(GL_COLOR_ATTACHMENT1);					// Draw into attachment 1
	glClear(GL_COLOR_BUFFER_BIT);						// Do not clear depth buffer here.
	glDepthFunc(GL_GREATER);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
	// loop begin
	glDrawBuffer(GL_COLOR_ATTACHMENT2);		// Draw intermediate result into attachment 2
	glDepthFunc(GL_LESS);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);

	// init gpu hash table and missed cache block id
	//GLsync s;
	int maxLoopCount = 0;
	do
	{
		initBlockExistsHash();
		initMissedBlockVector();
		g_rayCastingShaderProgram.bind();
		g_rayCastingShaderProgram.setUniformValue("totalPageTableSize", ysl::Vector3i{ pageTableX,pageTableY,pageTableZ });
		g_rayCastingShaderProgram.setUniformValue("blockDataSizeNoRepeat", ysl::Vector3i{ blockDataSize - 2 * repeat,blockDataSize - 2 * repeat,blockDataSize - 2 * repeat });
		g_rayCastingShaderProgram.setUniformValue("volumeDataSizeNoRepeat", ysl::Vector3i{ originalDataWidth,originalDataDepth,originalDataDepth });
		g_rayCastingShaderProgram.setUniformValue("pageTableBlockEntrySize", ysl::Vector3i{ pageTableBlockEntry ,pageTableBlockEntry ,pageTableBlockEntry });
		g_rayCastingShaderProgram.setUniformValue("repeatOffset", ysl::Vector3i{ repeat,repeat,repeat });
		g_rayCastingShaderProgram.setUniformSampler("cacheVolume", OpenGLTexture::TextureUnit5, *g_texCache);

		g_rayCastingShaderProgram.setUniformValue("viewMatrix", g_camera.view().Matrix());
		g_rayCastingShaderProgram.setUniformValue("orthoMatrix", g_orthoMatrix.Matrix());
		g_rayCastingShaderProgram.setUniformSampler("texStartPos", OpenGLTexture::TextureUnit0, *g_texEntryPos);
		g_rayCastingShaderProgram.setUniformSampler("texEndPos", OpenGLTexture::TextureUnit1, *g_texExitPos);
		g_rayCastingShaderProgram.setUniformSampler("texTransfunc", OpenGLTexture::TextureUnit2, *g_texTransferFunction);
		g_rayCastingShaderProgram.setUniformSampler("texIntermediateResult", OpenGLTexture::TextureUnit4, *g_texIntermediateResult);
		g_rayCastingShaderProgram.setUniformValue("step", step);
		g_rayCastingShaderProgram.setUniformValue("ka", ka);
		g_rayCastingShaderProgram.setUniformValue("ks", ks);
		g_rayCastingShaderProgram.setUniformValue("kd", kd);
		g_rayCastingShaderProgram.setUniformValue("shininess", shininess);
		g_rayCastingShaderProgram.setUniformValue("lightdir", g_lightDirection);
		auto halfWay = g_lightDirection - g_camera.front();
		if (halfWay.Length() > 1e-10) halfWay.Normalize();
		g_rayCastingShaderProgram.setUniformValue("halfway", halfWay);
		g_rayCastingVAO.bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/*	std::stringstream ss;
			ss << maxLoopCount;
			std::string str;
			ss >> str;
			g_texIntermediateResult->SaveAsImage("C:\\Users\\ysl\\Desktop\\debugimage\\result"+str+".jpg");
			g_texEntryPos->SaveAsImage("C:\\Users\\ysl\\Desktop\\debugimage\\entry" + str + ".jpg");*/
		maxLoopCount++;

	} while (CaptureAndHandleCacheMiss());

	//ysl::Log("Render pass per frame:%d.\n", maxLoopCount);

	g_framebuffer->Unbind();
	glDepthFunc(GL_LESS);
	g_quadsShaderProgram.bind();
	g_quadsShaderProgram.setUniformValue("viewMatrix", g_camera.view().Matrix());
	g_quadsShaderProgram.setUniformValue("orthoMatrix", g_orthoMatrix.Matrix());
	g_quadsShaderProgram.setUniformSampler("resTex", OpenGLTexture::TextureUnit2, *g_texIntermediateResult);
	g_rayCastingVAO.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);			// Draw into default framebuffer
	glDisable(GL_DEPTH_TEST);
	g_quadsShaderProgram.unbind();
}

static const char trivialVertexShader[] = "#version 330\n"
"uniform mat4 modelViewMat;\n"
"uniform mat4 projMat;\n"
"layout(location = 0) in vec3 vertex;\n"
"void main()\n"
"{\n"
"	gl_Position = projMat * modelViewMat*vec4(vertex, 1.0);\n"
"}\n";

static const char trivialFragShader[] = "#version 330\n"
"out vec4 fgColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"	fgColor = vec4(color,1.0);\n"
"}\n";


/**************************************************/

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char** argv)
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	std::shared_ptr<GLFWwindow> window{
		glfwCreateWindow(initialWidth, initialHeight, "Mixed Render Engine", NULL, NULL),
		[](GLFWwindow*window) {glfwDestroyWindow(window); } };

	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); // Enable vsync



	gl3wInit();

	if (!gl3wIsSupported(4, 4))
	{
		std::cout << "OpenGL 4.6 or later must be needed\n";
		system("pause");
		return 0;
	}

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	(void)io;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();


	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	//bool show_my_first_imgui_window = true;


	// 'Global' state

	///![1] console 
	TinyConsole app;
	// Config Commands


	app.ConfigCommand("load_data", [](const char * cmd)
		{
		});

	app.ConfigCommand("load_tf", [](const char * cmd)
		{
			cmdline::parser p;
			p.add<std::string>("tf", 'f', "load transfer function", false);
			p.parse(cmd);
			const auto fileName = p.get<std::string>("tf");

			std::cout << "fileName:" << fileName;

			ysl::TransferFunction m_tfObject;
			m_tfObject.read(fileName);
			if (!m_tfObject.valid())
			{
				std::cout << "Reading failed\n";
				return;
			}

			m_tfObject.FetchData(m_tfData.data(), 256);
			g_texTransferFunction->SetData(OpenGLTexture::RGBA32F, OpenGLTexture::RGBA, OpenGLTexture::Float32, 256, 0, 0, m_tfData.data());
		});
	auto showGLInformation = false;
	app.ConfigCommand("glinfo", [&showGLInformation](const char * cmd)
		{
			showGLInformation = true;
		});
	///![2] window size
	ysl::Vector2i windowSize;

	initializeResource();

	int blockBytes = g_largeVolumeData->BlockSize() *  g_largeVolumeData->BlockSize() *  g_largeVolumeData->BlockSize()*sizeof(char);

	GLbitfield flgs = GL_MAP_PERSISTENT_BIT |GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;
	
	glGenBuffers(1, &g_pingPBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pingPBO);
	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, blockBytes, nullptr,flgs);
	g_pboPtr[0] = (char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,0,blockBytes,flgs);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	GL_ERROR_REPORT;


	glGenBuffers(1, &g_pongPBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pongPBO);
	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, blockBytes, nullptr,flgs);
	g_pboPtr[1] = (char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, blockBytes, flgs);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	g_data = new char[blockBytes];

	GL_ERROR_REPORT;

	// Main loop
	while (!glfwWindowShouldClose(window.get()))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		app.Draw("Welcome Mixed Render Engine", nullptr);
		//ImGui::End();
		//ImGui::ShowDemoWindow();
		static bool lock = true;
		ImGui::Begin("Control Panel");
		ImGui::SliderFloat("step", &step, 0.001, 1.0);
		ImGui::SliderFloat("ka", &ka, 0.0f, 1.0f);
		ImGui::SliderFloat("kd", &kd, 0.0f, 1.0f);
		ImGui::SliderFloat("ks", &ks, 0.0f, 1.0f);
		ImGui::SliderFloat("shininess", &shininess, 0.0f, 50.f);
		if (ImGui::Button("lock"))
		{
			lock = !lock;
		}
		ImGui::End();


		if (showGLInformation)
			ShowGLInformation(&showGLInformation);

		// Event handle
		if (ImGui::IsMousePosValid())
		{
			MouseEvent pressEvent({ 0,0 }, 0);
			for (auto i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
			{
				// Only support two buttons now
				if (ImGui::IsMouseClicked(i))
				{
					ysl::Point2i pos{ int(io.MousePos.x),int(io.MousePos.y) };
					switch (i)
					{
					case 0:pressEvent.m_buttons |= MouseEvent::LeftButton; pressEvent.m_pos = pos; break;
					case 1:pressEvent.m_buttons |= MouseEvent::RightButton; pressEvent.m_pos = pos; break;
					default:break;
					}
				}
			}
			if (pressEvent.m_buttons != 0)
			{
				mousePressedEvent(&pressEvent);
			}
		}

		// Move Event
		MouseEvent moveEvent({ 0,0 }, 0);
		for (auto i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
		{
			if (io.MouseDownDuration[i] > 0.0f && !ImGui::IsMouseHoveringAnyWindow() && !lock)
			{
				if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0)
				{
					ysl::Point2i pos{ int(io.MousePos.x),int(io.MousePos.y) };
					switch (i)
					{
					case 0:moveEvent.m_buttons |= MouseEvent::LeftButton; moveEvent.m_pos = pos; break;
					case 1:moveEvent.m_buttons |= MouseEvent::RightButton; moveEvent.m_pos = pos; break;
					default:break;
					}
				}
			}
		}
		if (moveEvent.m_buttons != 0)
		{
			mouseMoveEvent(&moveEvent);
		}


		// Release Event

		MouseEvent releaseEvent({ 0,0 }, 0);
		for (auto i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
		{
			if (ImGui::IsMouseReleased(i))
			{
				ysl::Point2i pos{ int(io.MousePos.x),int(io.MousePos.y) };
				switch (i)
				{
				case 0:releaseEvent.m_buttons |= MouseEvent::LeftButton; releaseEvent.m_pos = pos; break;
				case 1:releaseEvent.m_buttons |= MouseEvent::RightButton; releaseEvent.m_pos = pos; break;
				default:break;
				}
			}
		}

		if (releaseEvent.m_buttons != 0)
			mouseReleaseEvent(&releaseEvent);
		// Rendering window resize Event
		if (io.DisplaySize.x != windowSize.x || io.DisplaySize.y != windowSize.y)
		{
			windowSize = { int(io.DisplaySize.x),int(io.DisplaySize.y) };
			ResizeEvent event(windowSize);
			renderingWindowResize(&event);
		}

		ImGui::EndFrame();
		// Rendering

		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window.get());
		glfwGetFramebufferSize(window.get(), &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		// User's render is here
		renderLoop();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window.get());
		glfwSwapBuffers(window.get());
	}



	// Test Code
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//glfwDestroyWindow(window);


	glfwTerminate();

	system("pause");

	return 0;
}
