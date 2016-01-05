#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/datastructures/LineSet.hpp>

#include <GL/glew.h>

#include "GL_LineAO.h"


using namespace fantom;

namespace {

	class LineAO : public VisAlgorithm {

		std::shared_ptr< const LineSet > m_streamlines;

		std::unique_ptr< Primitive > m_lineAO;

	public:

		struct Options : public VisAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				VisAlgorithm::Options( control )
			{
				add< LineSet >( "streamlines", "Streamlines to render" );
			}
		};

		struct VisOutputs : public VisAlgorithm::VisOutputs {
			VisOutputs( fantom::VisOutputs::Control& control ) :
				VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "LineAO" );
			}
		};

		LineAO( InitData& data ) :
			VisAlgorithm( data )
		{
			glewExperimental = GL_TRUE;
			glewInit();
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			m_streamlines = options.get< LineSet >( "streamlines" );
			if( !m_streamlines ) {
				infoLog() << "No input streamline dataset!" << std::endl;
				return;
			}

			m_lineAO = getGraphics( "LineAO" ).makePrimitive();
			//m_lineAO->setBlending( true );
			/*m_lineAO->setShaders( resourcePath() + "../../../praktikum/shader/LineAO-vertex.glsl",
								resourcePath() + "../../../praktikum/shader/LineAO-fragment.glsl" );*/
			m_lineAO->addCustom( std::bind( makeLineRenderer, m_streamlines ) );
		}

		static std::unique_ptr< CustomDrawer > makeLineRenderer( std::shared_ptr< const LineSet > sLines ) {
			return std::unique_ptr< CustomDrawer >( new GL_LineAO( sLines ) );
		}

	};

	AlgorithmRegister< LineAO > reg( "VisPraktikum/LineAO", "Line ambient occlusion" );

}