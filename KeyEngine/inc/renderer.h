#pragma once

#include <vector>
#include <memory>
#include "constant_buffer_ex.h"


// #TODO: rename all m_staticVar to s_staticVar
// #TODO: rename all Texture2D HLSL variable names
// #TODO: make Renderer more flexible with the names of Passes & Binder-Linker names

class Graphics;
class IRenderTargetView;
class IDepthStencilView;
class Camera;

namespace ren
{

class IPass;
class RenderQueuePass;
class ILinker;
class IBinder;

class Renderer
{
	std::vector<std::unique_ptr<IPass>> m_passes;
	std::vector<std::unique_ptr<IBinder>> m_globalBinders;
	std::vector<std::unique_ptr<ILinker>> m_globalLinkers;
	bool m_bValidatedPasses = false;
protected:
	std::shared_ptr<IRenderTargetView> m_globalColorBuffer;
	std::shared_ptr<IDepthStencilView> m_globalDepthStencil;
public:
	Renderer( Graphics &gph );
	~Renderer() noexcept;

	void run( Graphics &gph ) cond_noex;
	void reset() noexcept;
	RenderQueuePass& getRenderQueuePass( const std::string &name );
protected:
	void addGlobalLinker( std::unique_ptr<ILinker> pLinker );
	void addGlobalBinder( std::unique_ptr<IBinder> pBinder );
	void addPass( std::unique_ptr<IPass> pPass );
	void setupGlobalBinderTarget( const std::string &globalBinderName, const std::string &passName, const std::string &linkerName );
	//	\function	linkPassBinders	||	\date	2021/10/27 18:00
	//	\brief	links pass's binders to their linkers
	//			call this function last from derived Renderer's ctor
	void linkPassBinders( IPass &pass );
	void linkGlobalBinders();
	IPass& getPass( const std::string &name );
	void validateBindersLinkage();
};

class Renderer3d
	: public Renderer
{
	static constexpr inline int s_maxRadius = 13;
	static constexpr inline float s_maxSigma = 7.0f;
	static constexpr inline unsigned s_fullscreenRezReductFactor = 4u;
	int m_radius;
	float m_sigma;
	std::shared_ptr<PixelShaderConstantBufferEx> m_blurKernel;
	std::shared_ptr<PixelShaderConstantBufferEx> m_blurDirection;
public:
	enum KernelType
	{
		Gauss,
		Box,
	};
	KernelType m_kernelType;
public:
	Renderer3d( Graphics &gph, const int radius, const float sigma, KernelType kernelType = Gauss );

	void showImGuiWindows( Graphics &gph );
	void dumpShadowMap( Graphics &gph, const std::string &path );
	void setActiveCamera( Camera &cam );
	void setShadowCamera( Camera &cam );
private:
	void showShadowDumpImguiWindow( Graphics &gph );
	void showGaussianBlurImguiWindow( Graphics &gph );
	void setKernelGauss( const int radius, const float sigma ) cond_noex;
	void setKernelBox( const int radius ) cond_noex;
};

class Renderer2d
	: public Renderer
{
public:
	Renderer2d( Graphics &gph );
};


}//ren