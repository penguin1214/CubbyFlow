#ifndef CUBBYFLOW_LAND_AND_WAVES_APP_H_
#define CUBBYFLOW_LAND_AND_WAVES_APP_H_

#include <Array/Array2.h>
#include <Collider/RigidBodyCollider3.h>
#include <Emitter/VolumeGridEmitter3.h>
#include <Geometry/Box3.h>
#include <Geometry/Cylinder3.h>
#include <Geometry/Plane3.h>
#include <Geometry/Sphere3.h>
#include <Geometry/TriangleMesh3.h>
#include <Grid/ScalarGrid3.h>
#include <Grid/VertexCenteredScalarGrid3.h>
#include <Math/MathUtils.h>
#include <MarchingCubes/MarchingCubes.h>
#include <Solver/LevelSet/LevelSetLiquidSolver3.h>
#include <Surface/Implicit/ImplicitSurfaceSet3.h>
#include <Surface/Implicit/CustomImplicitSurface3.h>
#include <Utils/Logger.h>
#include <pystring/pystring.h>

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "../../Common/Camera.h"
#include "FrameResource.h"

const int gNumFrameResources = 3;

enum class RenderLayer : int
{
	Opaque = 0,
	Sky,
	Count
};

using namespace CubbyFlow;
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

struct RenderItem
{
	RenderItem() = default;
	RenderItem(const RenderItem& rhs) = delete;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class LandAndWavesApp : public D3DApp
{
public:
	LandAndWavesApp(HINSTANCE hInstance);
	LandAndWavesApp(const LandAndWavesApp& rhs) = delete;
	LandAndWavesApp& operator=(const LandAndWavesApp& rhs) = delete;
	~LandAndWavesApp();

	virtual bool Initialize()override;
	virtual bool CubbyFlowInitialize();
	void TriangulateAndSave();
	void ConvertVertexData();

protected:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateMaterial(const GameTimer& gt);

	void BuildWaterGeometryBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;  

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	RenderItem* mWaterRitem = nullptr;
	TriangleMesh3 mMesh;

	UINT mSkyTexHeapIndex = 0;

	PassConstants mMainPassCB;

	Camera mCamera;

	LevelSetLiquidSolver3Ptr mSolver = nullptr;

	Frame mFrame;
	size_t mResX;

	double mFps;
	double mNumberOfFrames;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	float mSunTheta = 1.25f*XM_PI;
	float mSunPhi = XM_PIDIV4;

	POINT mLastMousePos;
};

#endif