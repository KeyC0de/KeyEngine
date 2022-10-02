#pragma once

#include <vector>
#include <memory>
#include "constant_buffer_ex.h"


class Graphics;
class IRenderTargetView;
class IDepthStencilView;
class Camera;

namespace ren
{

class IPass;
class RenderQueuePass;
class IProducer;
class IConsumer;

class Renderer
{
	std::vector<std::unique_ptr<IPass>> m_passes;
	std::vector<std::unique_ptr<IConsumer>> m_globalConsumers;
	std::vector<std::unique_ptr<IProducer>> m_globalProducers;
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
	void addGlobalProducer( std::unique_ptr<IProducer> pProducer );
	void addGlobalConsumer( std::unique_ptr<IConsumer> pConsumer );
	void addPass( std::unique_ptr<IPass> pPass );
	void setupGlobalConsumerTarget( const std::string &globalConsumerName, const std::string &passName, const std::string &producerName );
	void validateConsumersLinkage();
	//===================================================
	//	\function	linkPassConsumers
	//	\brief  links pass's consumers to their producers
	//			call this function last from derived Renderer's ctor
	//	\date	2021/10/27 18:00
	void linkPassConsumers( IPass &pass );
	void linkGlobalConsumers();
	IPass& getPass( const std::string &name );
};

class Renderer3d
	: public Renderer
{
	static constexpr inline int m_maxRadius = 13;
	static constexpr inline float m_maxSigma = 7.0f;
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
	Renderer3d( Graphics &gph, int radius, float sigma, KernelType kernelType = Gauss );

	void showImGuiWindows( Graphics &gph );
	void dumpShadowMap( Graphics &gph, const std::string &path );
	void setActiveCamera( Camera &cam );
	void setShadowCamera( Camera &cam );
private:
	void showGaussianBlurImguiWindow( Graphics &gph );
	void showShadowDumpImguiWindow( Graphics &gph );
	void setKernelGauss( int radius, float sigma ) cond_noex;
	void setKernelBox( int radius ) cond_noex;
};

class Renderer2d
	: public Renderer
{
public:
	Renderer2d( Graphics &gph );
};


}//ren