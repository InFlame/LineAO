#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>
#include <fantom/datastructures/LineSet.hpp>

using namespace fantom;

namespace {

	class Integrator : public DataAlgorithm {

	protected:
		std::shared_ptr< const TensorFieldInterpolated< 3, Vector3 > > m_field;
		std::shared_ptr< const Grid< 3 > > m_grid;
		std::shared_ptr< const LineSet > m_seedLine;
		size_t m_numPoints;
		std::vector< std::vector< Point3 > > m_vertices;
		float m_stepSize;

	public:
		struct Options : public VisAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				VisAlgorithm::Options( control )
			{
				add< TensorFieldInterpolated< 3, Vector3 > >( "Field", "3D vector field" );
				add< LineSet >( "Seed line", "Starting points" );
				add< float >( "Step size", "Integration step size", 0.1 );
			}
		};

		struct DataOutputs : public DataAlgorithm::DataOutputs {
			DataOutputs( fantom::DataOutputs::Control& control ) :
				DataAlgorithm::DataOutputs( control )
			{
				add< LineSet >( "Streamlines" );
			}
		};

		Integrator( InitData& data ) :
			DataAlgorithm( data )
		{

		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			m_field = options.get< TensorFieldInterpolated< 3, Vector3 > >( "Field" );
			m_grid = std::dynamic_pointer_cast< const Grid< 3 > >( m_field->domain() );

			m_seedLine = options.get< const LineSet >( "Seed line" );
			if( !m_seedLine ) {
				infoLog() << "No input seedline!" << std::endl;
				return;
			}
			if( !m_field ) {
				infoLog() << "No input field!" << std::endl;
				return;
			}

			for( int i=0; i<m_vertices.size(); i++ ) {
				m_vertices[i].clear();
			}
			m_vertices.clear();
			m_stepSize = options.get< float >( "Step size" );
		}

		void makeLineSet( const Algorithm::Options& options ) {
			std::shared_ptr< LineSet > streamlines( new LineSet );
			for( int i=0; i<m_vertices.size(); i++ ) {
				std::vector< size_t > indices;
				for( int j=0; j<m_vertices[i].size(); j++ ) {
					indices.push_back( streamlines->addPoint( m_vertices[i][j] ) );
				}
				streamlines->addLine( indices );
			}
			setResult( "Streamlines", streamlines );
		}


	};

	//AlgorithmRegister< Integrator > reg( "Integrator", "Line Integrator" );
}