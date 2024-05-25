#pragma once

#include <vector>
#include <memory>
#include <string>


class Graphics;
class IRenderTargetView;
class IDepthStencilView;
class Camera;

class IPixelShaderConstantBufferEx;
template<class>
class ConstantBufferEx;
using PixelShaderConstantBufferEx = ConstantBufferEx<IPixelShaderConstantBufferEx>;

namespace ren
{

class IPass;
class RenderQueuePass;
class ILinker;
class IBinder;

class Renderer
{
	bool m_bValidatedPasses = false;
	std::vector<std::unique_ptr<IPass>> m_passes;
	std::vector<std::unique_ptr<IBinder>> m_globalBinders;
	std::vector<std::unique_ptr<ILinker>> m_globalLinkers;
protected:
	bool m_bUsesOffscreen;
	std::unique_ptr<IPass> m_pFinalPostProcessPass;
	std::shared_ptr<IRenderTargetView> m_pRtv;
	std::shared_ptr<IDepthStencilView> m_pDsv;
public:
	Renderer( Graphics &gfx, const bool drawToOffscreen );
	virtual ~Renderer() noexcept;

	void run( Graphics &gfx ) cond_noex;
	virtual void recreate( Graphics &gfx );
	virtual void reset() noexcept;
	RenderQueuePass& getRenderQueuePass( const std::string &name );
	bool isUsingOffscreenRendering() const noexcept;
protected:
	void addGlobalLinker( std::unique_ptr<ILinker> pLinker );
	void addGlobalBinder( std::unique_ptr<IBinder> pBinder );
	void addPass( std::unique_ptr<IPass> pPass );
	void setupGlobalBinderTarget( const std::string &globalBinderName, const std::string &passName, const std::string &linkerName );
	void linkGlobalBinders();
	IPass& getPass( const std::string &name );
private:
	void validateBindersLinkage();
	//	\function	linkPassBinders	||	\date	2021/10/27 18:00
	//	\brief	links pass's binders to their linkers
	void linkPassBinders( IPass &pass );
	//	\function	offscreenToBackBufferSwap	||	\date	2022/11/05 15:42
	//	\brief	If there's a final post process pass (Pass that renders directly to the Back Buffer) then swap the render targets, ie.
	//			bind a. Back Buffer RTV as output and b. Offscreen RTV as input
	void offscreenToBackBufferSwap(  Graphics &gfx );
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
	Renderer3d( Graphics &gfx, const bool bDrawToOffscreen, const int radius, const float sigma, const KernelType kernelType = Gauss );

	void recreate( Graphics &gfx ) override;
	void displayImguiWidgets( Graphics &gfx ) noexcept;
	void dumpShadowMap( Graphics &gfx, const std::string &path );
	//	\function	setActiveCamera	||	\date	2022/05/24 17:10
	//	\brief	binds active camera to all Passes that need it
	void setActiveCamera( const Camera &cam );
	void setShadowCamera( const Camera &cam, const bool bEnable );
private:
	void showShadowDumpImguiWindow( Graphics &gfx ) noexcept;
	void showGaussianBlurImguiWindow( Graphics &gfx ) noexcept;
	void showDisplayMode( Graphics &gfx ) noexcept;
	void setKernelGauss( const int radius, const float sigma ) cond_noex;
	void setKernelBox( const int radius ) cond_noex;
};

class Renderer2d
	: public Renderer
{
public:
	Renderer2d( Graphics &gfx );
	
	void recreate( Graphics &gfx ) override;
};


}//ren