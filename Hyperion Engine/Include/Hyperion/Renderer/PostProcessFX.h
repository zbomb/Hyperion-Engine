/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/PostProcessFX.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"


namespace Hyperion
{

	class PostProcessFX
	{

	protected:

		std::shared_ptr< RPostProcessShader > m_Shader;
		PostProcessRenderTarget m_Target;

	public:

		PostProcessFX()
			: m_Target( PostProcessRenderTarget::BackBuffer )
		{

		}


		~PostProcessFX()
		{
			m_Shader.reset();
		}


		void AttachShader( const std::shared_ptr< RPostProcessShader >& inShader )
		{
			if( !inShader || !inShader->IsValid() )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to attach post process shader to FX, the shader was null/invalid" );
				return;
			}

			m_Shader = inShader;
		}


		inline std::shared_ptr< RPostProcessShader > GetShader() const { return m_Shader; }
		inline PostProcessRenderTarget GetRenderTarget() const { return m_Target; }

		bool IsValid() const
		{
			return m_Shader && m_Shader->IsValid();
		}

		void SetRenderTarget( PostProcessRenderTarget inTarget )
		{
			m_Target = inTarget;
		}

	};

}